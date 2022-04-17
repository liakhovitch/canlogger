#include "can.h"
#include "MCP2515.h"
#include "CANSPI.h"
#include "spi.h"
#include "stdbool.h"
#include "globals.h"

// "Read RX Buffer" command from MCP2515 datasheet
#define READ_COMMAND 0b10010000
const unsigned char read_command[14] = {READ_COMMAND,0,0,0,0,0,0,0,0,0,0,0,0,0};

// Variables that point MCP2515 library to a particular SPI port
extern SPI_HandleTypeDef hspi2;
extern SPI_HandleTypeDef hspi3;
extern SPI_HandleTypeDef* SPI_CAN;
extern GPIO_TypeDef* SPI_PORT;
extern uint16_t SPI_PIN;

// CAN data buffers and other methods of communication with the main loop
extern struct circularBuffer buf1;
extern struct circularBuffer buf2;
extern _Atomic volatile unsigned int overflow_flag;

// DMA lock variables to ensure that "CAN data ready" interrupts are only enabled if *neither* DMA is currently in progress
_Atomic volatile int dmalock1 = 0;
_Atomic volatile int dmalock2 = 0;
unsigned int buf1_write_pos_new = 0;
unsigned int buf2_write_pos_new = 0;

void disable_can_irq(){
    NVIC_DisableIRQ(EXTI0_IRQn);
    NVIC_DisableIRQ(EXTI1_IRQn);
    NVIC_DisableIRQ(EXTI9_5_IRQn);
}

void enable_can_irq(){
    NVIC_EnableIRQ(EXTI0_IRQn);
    NVIC_EnableIRQ(EXTI1_IRQn);
    NVIC_EnableIRQ(EXTI9_5_IRQn);
}

int init_single_mcp2515(){
    // Run 'CANSPI' library functions to initialize MCP2515s
    // Reset device
    MCP2515_Reset();
    // Basic initialization
    if(CANSPI_Initialize() == false) return 1;
    // Put MCP2515 back into config mode for custom initialization steps
    if(MCP2515_SetConfigMode() == false) return 1;
    // Set RX0BF to act as buffer full interrupt
    // Write 0b11100000 to BPFCTRL
    MCP2515_WriteByte(0x0C, 0b10100000);
    // Set RXB0 to receive any message and not rollover to RXB1
    // Write 0b00000110 to RXB0CTRL
    MCP2515_WriteByte(MCP2515_RXB0CTRL, 0b10100000);
#ifndef PCB_V2
    // Enable "Receive buffer full" interrupt on INT
    // Set CANINTE to 0b10000000
    MCP2515_WriteByte(MCP2515_CANINTE, 0b10100000);
#endif
#ifdef PCB_V2
    // Enable error interrupt on INT
    // Set CANINTE to 0b00000100
    MCP2515_WriteByte(MCP2515_CANINTE, 0b00000100);
#endif
    if(MCP2515_SetNormalMode() == false) return 1;
    // Set general purpose interrupt to only report errors
    return 0;
}

// Clear errors and interrupt flags on a single MCP2515. Expects library to be set to use the right interface.
void clear_errors(){
    // Clear all pending errors
    MCP2515_WriteByte(MCP2515_EFLG, 0b00000000);
    // Clear error interrupt without clearing buffer full interrupt
    MCP2515_BitModify(MCP2515_CANINTF, 0b00000101, 0b00000000);
}

// Set the CAN controller that the MCP2515 library will interact with.
// if i==0, we use CAN1. If i==1, we use CAN2.
void set_mcp2515_iface(unsigned char i){
    if(!i){
        SPI_CAN = &hspi2;
        SPI_PORT = CAN1_CS_GPIO_Port;
        SPI_PIN = CAN1_CS_Pin;
    }else{
        SPI_CAN = &hspi3;
        SPI_PORT = CAN2_CS_GPIO_Port;
        SPI_PIN = CAN2_CS_Pin;
    }
}

// Clears errors and interrupt flags on both MCP2515s
void clear_errors_all(){
    set_mcp2515_iface(0);
    clear_errors();
    set_mcp2515_iface(1);
    clear_errors();
}

int init_can(){
    disable_can_irq();
    // In case this function is being called as a last-ditch recovery attempt for some awful situation
    HAL_SPI_Abort_IT(&hspi2);
    HAL_SPI_Abort_IT(&hspi3);
    dmalock1 = 0;
    dmalock2 = 0;
    // Note: this is an ugly way to do things, but it lets the MCP2515 library work with multiple chips with only
    //       minimal modification to the library.
    // Set MCP2515 library to use SPI2
    set_mcp2515_iface(0);
    // Init CAN1 controller
    if(init_single_mcp2515()) return 1;
    // Set MCP2515 library to use SPI3
    set_mcp2515_iface(1);
    // Init CAN2 controller
    if(init_single_mcp2515()) return 1;
    enable_can_irq();
    return 0;
}

void can_panic(){
    overflow_flag = 1;
    disable_can_irq();
}

#ifdef PCB_V2
#define CAN1_ERR()   !HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0)
#define CAN2_ERR()   !HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1)
#define CAN1_READY() !HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_6)
#define CAN2_READY() !HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_7)
#endif
#ifndef PCB_V2
#define CAN1_ERR()   0
#define CAN2_ERR()   0
#define CAN1_READY() !HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0)
#define CAN2_READY() !HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1)
#endif

void handle_can_spi(){
    if(CAN1_ERR()){
        set_mcp2515_iface(0);
        clear_errors();
        // For now, we clear errors and silently ignore them.
        // TODO: Place error message on stack
    }
    if(CAN2_ERR()){
        set_mcp2515_iface(1);
        clear_errors();
    }
    if(CAN1_READY()){
        // Circular buffer management:
        // Determine next write position, with wraparound
        // Note: we use a global variable because the DMA handler also needs access to this
        buf1_write_pos_new = buf1.write_pos + buf1.increment;
        if(buf1_write_pos_new >= buf1.len) buf1_write_pos_new = 0;
        // Check if buffer full
        if(buf1_write_pos_new == buf1.read_pos){
            can_panic();
            return;
        }
        // Disable EXTI interrupts so that we don't try to start another transfer while this one is ongoing
        disable_can_irq();
        // Set DMAlock1 so that CAN2 DMA handler knows not to re-enable interrupts until DMA for CAN1 is done
        dmalock1 = 1;
        // Set chip select to tell MCP2515 that transfer is happening
        HAL_GPIO_WritePin(CAN1_CS_GPIO_Port, CAN1_CS_Pin, 1);
        // Initiate DMA
        HAL_SPI_TransmitReceive_DMA(&hspi2, (uint8_t*)read_command, (uint8_t*)(buf1.start_ptr + buf1.write_pos), 14);
    }
    if(CAN2_READY()){
        // Circular buffer management:
        // Determine next write position, with wraparound
        // Note: we use a global variable because the DMA handler also needs access to this
        buf2_write_pos_new = buf2.write_pos + buf2.increment;
        if(buf2_write_pos_new >= buf2.len) buf2_write_pos_new = 0;
        // Check if buffer full
        if(buf2_write_pos_new == buf2.read_pos){
            can_panic();
            return;
        }
        // Disable EXTI interrupts so that we don't try to start another transfer while this one is ongoing
        disable_can_irq();
        // Set DMAlock1 so that CAN2 DMA handler knows not to re-enable interrupts until DMA for CAN1 is done
        dmalock2 = 1;
        // Set chip select to tell MCP2515 that transfer is happening
        HAL_GPIO_WritePin(CAN2_CS_GPIO_Port, CAN2_CS_Pin, 0);
        // Initiate DMA
        HAL_SPI_TransmitReceive_DMA(&hspi3, (uint8_t*)read_command, (uint8_t*)(buf2.start_ptr + buf2.write_pos), 14);
    }
}

void handle_dma_done1(){
    // Set CS pin high to tell the MCP2515 that the transfer is over
    HAL_GPIO_WritePin(CAN1_CS_GPIO_Port, CAN1_CS_Pin, 1);
    // Do circular buffer maintenance (set write position)
    buf1.write_pos = buf1_write_pos_new;
    // Clear dmalock1 to tell the other DMA handler that it is free to re-enable EXTI interrupts
    dmalock1 = 0;
    // If all DMAs are finished, re-enable EXTI interrupts so that we can handle more incoming data
    if(!dmalock2){
        enable_can_irq();
    }
}

void handle_dma_done2(){
    // Set CS pin high to tell the MCP2515 that the transfer is over
    HAL_GPIO_WritePin(CAN2_CS_GPIO_Port, CAN2_CS_Pin, 1);
    // Do circular buffer maintenance (set write position)
    buf2.write_pos = buf2_write_pos_new;
    // Clear dmalock2 to tell the other DMA handler that it is free to re-enable EXTI interrupts
    dmalock2 = 0;
    // If all DMAs are finished, re-enable EXTI interrupts so that we can handle more incoming data
    if(!dmalock1){
        enable_can_irq();
    }
}
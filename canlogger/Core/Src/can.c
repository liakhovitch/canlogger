#include "can.h"
#include "MCP2515.h"
#include "CANSPI.h"
#include "spi.h"
#include "stdbool.h"

// "Read RX Buffer" command from MCP2515 datasheet
#define READ_COMMAND 0b10010000
const unsigned char read_command[14] = {READ_COMMAND,0,0,0,0,0,0,0,0,0,0,0,0,0};

// Variables that point MCP2515 library to a particular SPI port
extern SPI_HandleTypeDef hspi2;
extern SPI_HandleTypeDef hspi3;
extern SPI_HandleTypeDef* SPI_CAN;
extern GPIO_TypeDef* SPI_PORT;
extern uint16_t SPI_PIN;

// DMA lock variables to ensure that "CAN data ready" interrupts are only enabled if *neither* DMA is currently in progress
volatile int dmalock1 = 0;
volatile int dmalock2 = 0;

int init_single_mcp2515(){
    // Run 'CANSPI' library functions to initialize MCP2515's
    // Pseudocode:
    // Basic initialization
    if(CANSPI_Initialize() == false) return 1;
    // Set controllers to only use RXB0 (nothing to do here - this is the default)
    // Enable _RXB0F interrupt output
    if(MCP2515_SetConfigMode() == false) return 1;
    // TODO -- enable _RXB0F interrupt output
    if(MCP2515_SetNormalMode() == false) return 1;
    // Set general purpose interrupt to only report errors
    return 0;
}

int init_can(){
    // Note: this is an ugly way to do things, but it lets the MCP2515 library work with multiple chips with only
    //       minimal modification to the library.

    // Set MCP2515 library to use SPI2
    SPI_CAN = &hspi2;
    SPI_PORT = CAN1_CS_GPIO_Port;
    SPI_PIN = CAN1_CS_Pin;
    // Init CAN1 controller
    if(init_single_mcp2515()) return 1;

    // Set MCP2515 library to use SPI3
    SPI_CAN = &hspi3;
    SPI_PORT = CAN2_CS_GPIO_Port;
    SPI_PIN = CAN2_CS_Pin;
    // Init CAN2 controller
    if(init_single_mcp2515()) return 1;
    return 0;
}

void handle_can_spi(){
    // Pseudocode:
    // Is CAN1 error flag set? If so, handle error
    // Is CAN2 error flag set? If so, handle error
    // Is CAN1 data ready? If so:
    //   Do circular buffer stuff
    //   Disable EXTI interrupts
    //   Set dmalock1
    //   Set up DMA
    // Is CAN2 data ready? If so:
    //   Do circular buffer stuff
    //   Disable EXTI interrupts
    //   Set dmalock2
    //   Set up DMA
    // For later: line that actually starts DMA transfer:
    // HAL_SPI_TransmitReceive_DMA(&hspi2, read_command, --pointer to next buffer slot--);
}

void handle_dma_done1(){
    // Pseudocode:
    // Clear dmalock1
    // Do circular buffer maintenance
    // If dmalock2 is cleared, re-enable EXTI interrupts
}

void handle_dma_done2(){
    // Pseudocode:
    // Clear dmalock2
    // Do circular buffer maintenance
    // If dmalock1 is cleared, re-enable EXTI interrupts
}
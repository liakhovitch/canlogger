#include "test_gen_can.h"
#include "globals.h"
#include "MCP2515.h"
#include "CANSPI.h"
#include "can.h"

// Variables that point MCP2515 library to a particular SPI port
extern SPI_HandleTypeDef *SPI_CAN;
extern GPIO_TypeDef *SPI_PORT;
extern uint16_t SPI_PIN;

// CAN data buffers and other methods of communication with the main loop
extern struct circularBuffer buf1;
extern struct circularBuffer buf2;

// i = 0: CAN1
// i = 1: CAN2
void test_gen_can_single(unsigned char i){
    set_mcp2515_iface(i);
    uCAN_MSG rxMessage;
    struct bufCell cell;
    if(CANSPI_Receive(&rxMessage))
    {
        uint16_t sid = 0;
        uint16_t eid = 0;
        if(rxMessage.frame.idType == dSTANDARD_CAN_MSG_ID_2_0B){
            sid = (uint16_t)(rxMessage.frame.id);
        }else if (rxMessage.frame.idType == dEXTENDED_CAN_MSG_ID_2_0B){
            sid = (uint16_t)(rxMessage.frame.id);
            eid = (uint16_t)(rxMessage.frame.id >> 16);
        }
        uint8_t dat_len = rxMessage.frame.dlc;
        uint8_t dat[8] = {
                rxMessage.frame.data0,
                rxMessage.frame.data1,
                rxMessage.frame.data2,
                rxMessage.frame.data3,
                rxMessage.frame.data4,
                rxMessage.frame.data5,
                rxMessage.frame.data6,
                rxMessage.frame.data7,
        };
        construct_packet(&cell, sid, eid, &dat, dat_len);
        if(i == 0){
            buf_put( &buf1, &cell);
        }else{
            buf_put( &buf2, &cell);
        }
    }
}

// Test data generator that uses MCP2515 CAN library in a polling fashion.
// Must be called continuously from the main loop.
void test_gen_can(){
    test_gen_can_single(0);
    test_gen_can_single(1);
}

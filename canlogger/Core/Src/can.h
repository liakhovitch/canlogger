#ifndef CANLOGGER_CAN_H
#define CANLOGGER_CAN_H

// Interface with the rest of the program:
// * The init_can() function is to be called once at the beginning of the program
// * The "handle_" functions are to be called from their respective ISRs
// * In case of a catastrophic failure, "can_panic_flag" from "globals.c" will be set to 1.
//   The main loop must monitor for this condition, and:
//   1. Free up space in the CAN buffer (at least two slots, preferably a lot more)
//   2. Call init_can() to restart the CAN handlers

// Initialize CAN modules. This also acts as a reset function in case of a catastrophic failure.
int init_can();

void can_panic();

void handle_can_panic();

void handle_can_spi();

void handle_dma_done1();

void handle_dma_done2();

void test_offload_data();

void set_mcp2515_iface(unsigned char i);

#endif //CANLOGGER_CAN_H

#ifndef CANLOGGER_CAN_H
#define CANLOGGER_CAN_H


int init_can();

void handle_can_spi();

void handle_dma_done1();

void handle_dma_done2();

#endif //CANLOGGER_CAN_H

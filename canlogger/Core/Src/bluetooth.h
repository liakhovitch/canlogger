#ifndef CANLOGGER_BLUETOOTH_H
#define CANLOGGER_BLUETOOTH_H

#include <stdint-gcc.h>

int init_bt();

int handle_bt();

void handle_bt_irq(uint16_t len);

#endif //CANLOGGER_BLUETOOTH_H

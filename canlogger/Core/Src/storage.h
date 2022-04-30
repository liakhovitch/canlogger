#include "fatfs.h"

#ifndef CANLOGGER_STORAGE_H
#define CANLOGGER_STORAGE_H

int init_storage(FATFS *FatFs);

int flush_storage(FATFS *FatFs);

int flush_buf(int can_ch_slct, FIL *fil);

int demount_storage();

void test_generate_data();

#endif //CANLOGGER_STORAGE_H

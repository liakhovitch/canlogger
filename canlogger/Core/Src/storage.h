#include "fatfs.h"

#ifndef CANLOGGER_STORAGE_H
#define CANLOGGER_STORAGE_H

int init_storage(FATFS *FatFs);

//This clears the buffers and closes the file buffers
//Then runs until the circularBuffers are empty
//Only Run when you are finished recording CAN data
int flush_storage();


//Pulls one frame off of each buffer
//Assume file pointers are at eof
int pop_buf();

int close_fil();

int open_fil();

int find_eof();

int read_storage(uint8_t can_ch_slct, BYTE * buffer, UINT btr, UINT * br);

int get_file_size(uint8_t can_ch_slct);

int delete_file(uint8_t can_ch_slct);

int demount_storage();

void handle_unmount();

void test_generate_data();

#endif //CANLOGGER_STORAGE_H

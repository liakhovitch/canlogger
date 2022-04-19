#include <stdint-gcc.h>
#include "storage.h"
#include "globals.h"

extern struct circularBuffer buf1;
extern struct circularBuffer buf2;

int init_storage(){
    return 0;
}

int flush_storage(){
    return 0;
}

// Generate some data and push it to buf1 and buf2. To be used for testing only.
void test_generate_data(){
    struct bufCell cell;
    uint8_t test_dat[8] = {2, 3, 5, 7, 11, 13, 17, 19};
    construct_packet(&cell, 69, 420, test_dat, 8);
    buf_put(&buf1, &cell);
    buf_put(&buf2, &cell);
}
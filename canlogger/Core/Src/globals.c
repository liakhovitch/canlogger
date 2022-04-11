#include "globals.h"

const unsigned char buf1_array[BUFF_SIZE * sizeof(struct bufCell)];
struct circularBuffer buf1 = {
        .start_ptr = buf1_array,
        .len = BUFF_SIZE + sizeof(struct bufCell),
        .increment = sizeof(struct bufCell),
        .read_pos = 0,
        .write_pos = 0
};

const unsigned char buf2_array[BUFF_SIZE * sizeof(struct bufCell)];
struct circularBuffer buf2 = {
        .start_ptr = buf2_array,
        .len = BUFF_SIZE + sizeof(struct bufCell),
        .increment = sizeof(struct bufCell),
        .read_pos = 0,
        .write_pos = 0
};

// Reference circular buffer implementation
// IMPORTANT - if reimplementing elsewhere, follow the exact same algorithm! Do not try to further optimize!

int buf_get(struct circularBuffer* buf, struct bufCell* ret){
    // Check for empty buffer
    if (buf->read_pos == buf->write_pos) return 1;
    // Use the popped value
    // Note: make sure that the value is no longer needed after this. In this case, we achieve this by copying.
    struct bufCell* cell = (struct bufCell*)(buf->start_ptr + buf->read_pos);
    *ret = *cell;
    // Determine new read position, with wraparound
    unsigned int read_pos_new = buf->read_pos + buf->increment;
    if(read_pos_new >= buf->len) read_pos_new = 0;
    // *Atomically* set new read position
    buf->read_pos = read_pos_new;
    return 0;
}

int buf_put(struct circularBuffer* buf, struct bufCell* dat){
    // Determine next write position, with wraparound
    unsigned int write_pos_new = buf->write_pos + buf->increment;
    if(write_pos_new >= buf->len) write_pos_new = 0;
    // Check if buffer full
    if(write_pos_new == buf->read_pos) return 1;
    // Write the data to the current write position
    struct bufCell* cell = (struct bufCell*)(buf->start_ptr + buf->write_pos);
    *cell = *dat;
    // *Atomically* set new write position
    buf->write_pos = write_pos_new;
}

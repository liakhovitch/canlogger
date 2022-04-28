#include <stdint-gcc.h>
#include "globals.h"

// Flag to communicate fail condition from CAN code to main loop
_Atomic volatile unsigned int overflow_flag = 0;

uint8_t buf1_array[BUFF_SIZE * sizeof(struct bufCell)];
struct circularBuffer buf1 = {
        .start_ptr = buf1_array,
        .len = BUFF_SIZE + sizeof(struct bufCell),
        .increment = sizeof(struct bufCell),
        .read_pos = 0,
        .write_pos = 0
};

uint8_t buf2_array[BUFF_SIZE * sizeof(struct bufCell)];
struct circularBuffer buf2 = {
        .start_ptr = buf2_array,
        .len = BUFF_SIZE + sizeof(struct bufCell),
        .increment = sizeof(struct bufCell),
        .read_pos = 0,
        .write_pos = 0
};

// Reference circular buffer implementation
// IMPORTANT - if reimplementing elsewhere, follow the exact same algorithm! Do not try to further optimize!

int buf_get(struct circularBuffer *buf, struct bufCell *ret) {
    // Check for empty buffer
    if (buf->read_pos == buf->write_pos) return 1;
    // Use the popped value
    // Note: make sure that the value is no longer needed after this. In this case, we achieve this by copying.
    struct bufCell *cell = (struct bufCell *) (buf->start_ptr + buf->read_pos);
    *ret = *cell;
    // Determine new read position, with wraparound
    unsigned int read_pos_new = buf->read_pos + buf->increment;
    if (read_pos_new >= buf->len) read_pos_new = 0;
    // *Atomically* set new read position
    buf->read_pos = read_pos_new;
    return 0;
}

int buf_put(struct circularBuffer *buf, struct bufCell *dat) {
    // Determine next write position, with wraparound
    unsigned int write_pos_new = buf->write_pos + buf->increment;
    if (write_pos_new >= buf->len) write_pos_new = 0;
    // Check if buffer full
    if (write_pos_new == buf->read_pos) return 1;
    // Write the data to the current write position
    struct bufCell *cell = (struct bufCell *) (buf->start_ptr + buf->write_pos);
    *cell = *dat;
    // *Atomically* set new write position
    buf->write_pos = write_pos_new;
    return 0;
}

void buf_clear(struct circularBuffer *buf){
    buf->read_pos = 0;
    buf->write_pos = 0;
}

void construct_packet(struct bufCell* cell, uint16_t sid, uint16_t eid, const uint8_t* dat, uint8_t dat_len) {
    cell->STATUS   = 0;
    cell->RXB0SIDH = (uint8_t)(sid>>8);
    cell->RXB0SIDL = (uint8_t)(sid & 0x00FF);
    cell->RXB0EID8 = (uint8_t)(eid>>8);
    cell->RXB0EID0 = (uint8_t)(eid & 0x00FF);
    cell->RXB0DLC  = dat_len & 0x0F;
    cell->RXB0D0   = dat_len > 0 ? dat[0] : 0;
    cell->RXB0D1   = dat_len > 1 ? dat[1] : 0;
    cell->RXB0D2   = dat_len > 2 ? dat[2] : 0;
    cell->RXB0D3   = dat_len > 3 ? dat[3] : 0;
    cell->RXB0D4   = dat_len > 4 ? dat[4] : 0;
    cell->RXB0D5   = dat_len > 5 ? dat[5] : 0;
    cell->RXB0D6   = dat_len > 6 ? dat[6] : 0;
    cell->RXB0D7   = dat_len > 7 ? dat[7] : 0;
}

void parse_packet(struct bufCell* cell, uint16_t* sid, uint16_t* eid, uint8_t dat[8], uint8_t* dat_len) {
    *sid = ((uint16_t)cell->RXB0SIDH << 8) | (uint16_t)(cell->RXB0SIDL);
    *eid = ((uint16_t)cell->RXB0EID8 << 8) | (uint16_t)(cell->RXB0EID0);
    *dat_len = cell->RXB0DLC & 0x0F;
    dat[0] = *dat_len > 0 ? cell->RXB0D0 : 0;
    dat[1] = *dat_len > 1 ? cell->RXB0D1 : 0;
    dat[2] = *dat_len > 2 ? cell->RXB0D2 : 0;
    dat[3] = *dat_len > 3 ? cell->RXB0D3 : 0;
    dat[4] = *dat_len > 4 ? cell->RXB0D4 : 0;
    dat[5] = *dat_len > 5 ? cell->RXB0D5 : 0;
    dat[6] = *dat_len > 6 ? cell->RXB0D6 : 0;
    dat[7] = *dat_len > 7 ? cell->RXB0D7 : 0;
}

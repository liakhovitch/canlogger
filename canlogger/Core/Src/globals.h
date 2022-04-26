#ifndef CANLOGGER_GLOBALS_H
#define CANLOGGER_GLOBALS_H

// Test defines
// Pick one of these
#define PRODUCTION_GEN
//#define TEST_GEN_FIXED
//#define TEST_GEN_CAN
// Pick one of these
#define PRODUCTION_OFFLOAD
//#define TEST_OFFLOAD_UART

// Uncomment if using PCB rev 2
#define PCB_V2

// Size of data buffers, in data packets (14 bytes each)
#define BUFF_SIZE 200

struct circularBuffer {
    uint8_t * start_ptr;
    const unsigned int len;
    const unsigned int increment;
    _Atomic volatile unsigned int read_pos;
    _Atomic volatile unsigned int write_pos;
};

/*
 * Data pulled from the MCP2515 can be directly interpreted as an instance of this struct, which is guaranteed to have
 * memory layout consistent with the order in which the bytes are sent over SPI.
*/
struct bufCell {
    unsigned char STATUS;
    unsigned char RXB0SIDH;
    unsigned char RXB0SIDL;
    unsigned char RXB0EID8;
    unsigned char RXB0EID0;
    unsigned char RXB0DLC;
    unsigned char RXB0D0;
    unsigned char RXB0D1;
    unsigned char RXB0D2;
    unsigned char RXB0D3;
    unsigned char RXB0D4;
    unsigned char RXB0D5;
    unsigned char RXB0D6;
    unsigned char RXB0D7;
    uint16_t : 16;          // Padding for best alignment
}__attribute__((packed, aligned(1)));

// Example circular buffer implementations
int buf_get(struct circularBuffer *buf, struct bufCell *ret);

int buf_put(struct circularBuffer *buf, struct bufCell *dat);

void buf_clear(struct circularBuffer *buf);

void construct_packet(struct bufCell* cell, uint16_t sid, uint16_t eid, const uint8_t* dat, uint8_t dat_len);

void parse_packet(struct bufCell* cell, uint16_t* sid, uint16_t* eid, uint8_t dat[8], uint8_t* dat_len);

#endif //CANLOGGER_GLOBALS_H

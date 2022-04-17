#ifndef CANLOGGER_GLOBALS_H
#define CANLOGGER_GLOBALS_H

// Test defines
// Comment these for production code
// #define TEST_DATA_GEN

// Uncomment if using PCB rev 2
 #define PCB_V2

#define BUFF_SIZE 200

struct circularBuffer{
    const unsigned char* start_ptr;
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
    unsigned char GARBAGE;
    unsigned char RXB0SIDh;
    unsigned char RXB0SIDL;
    unsigned char RXB0EID8;
    unsigned char RXB0EID0;
    unsigned char RXB0DLC;
    unsigned char RXB0D0;
    unsigned char rxB0D1;
    unsigned char rxB0D2;
    unsigned char rxB0D3;
    unsigned char rxB0D4;
    unsigned char rxB0D5;
    unsigned char rxB0D6;
    unsigned char rxB0D7;
}__attribute__((packed, aligned(1)));

// Example circular buffer implementations
int buf_get(struct circularBuffer* buf, struct bufCell* ret);

int buf_put(struct circularBuffer* buf, struct bufCell* dat);

#endif //CANLOGGER_GLOBALS_H

#ifndef _7SEGMENT_H_
#define _7SEGMENT_H_
#include <Arduino.h>

#define SEGMENT_COUNT 8
#define MAX_DIGITS 4
// define a struct to hold the port configuration for each segment
typedef struct {
    uint8_t portA,
            portB,
            portC;
} segmentBitmask;

// Segment configuration for each digit. Add a single bit to the port to turn on the segment
const segmentBitmask segments[SEGMENT_COUNT] = {
    {0b00000001, 0b00000010, 0b00000100}, // 0
    {0b00000010, 0b00000001, 0b00000100}, // 1
    {0b00000001, 0b00000100, 0b00000010}, // 2
    {0b00000010, 0b00000100, 0b00000001}, // 3
    {0b00000001, 0b00000100, 0b00000010}, // 4
    {0b00000010, 0b00000001, 0b00000100}, // 5
    {0b00000001, 0b00000010, 0b00000100}, // 6
    {0b00000010, 0b00000100, 0b00000001}  // 7
};

#define SYMBOL_COUNT 16
const char symbolLookup[SYMBOL_COUNT] = {
    '0', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
};

struct symbol{
    int count;
    char segments[SEGMENT_COUNT];
};

// Define the segments that need to be turned on for each symbol
const symbol symbols[SYMBOL_COUNT] = {
    {3, {0, 1, 2}}, // 0
    {2, {1, 2}},    // 1
    {3, {0, 1, 3}}, // 2
    {3, {1, 2, 3}}, // 3
    {2, {1, 2}},    // 4
    {3, {0, 2, 3}}, // 5
    {3, {0, 2, 3}}, // 6
    {2, {1, 2}},    // 7
    {3, {0, 1, 2, 3}}, // 8
    {3, {0, 1, 3}}, // 9
    {3, {0, 1, 2}}, // a
    {3, {0, 1, 2, 3}}, // b
    {2, {0, 3}},    // c
    {2, {0, 3}},    // d
    {3, {0, 2, 3}}, // e
    {3, {0, 2, 3}}  // f
};

enum polarity_t{
    COMMON_ANODE,
    COMMON_CATHODE
};

#define SET_BY_MASK(port, mask) port |= mask
#define CLEAR_BY_MASK(port, mask) port &= ~mask

class SegmentDisplay{
    public:
        SegmentDisplay(int digits, polarity_t polarity, segmentBitmask digitPins[]);
        void begin();
        void end();
        void display(const char *value);
        void display(const int value);
        void clear();
        void next();
    private:
        char outputBuffer[MAX_DIGITS];
        polarity_t polarity = COMMON_CATHODE;
        int digits = 0;
        segmentBitmask *digitPins[MAX_DIGITS];
        PORT_t *portA = portToPortStruct(0);
        PORT_t *portB = portToPortStruct(1);
        PORT_t *portC = portToPortStruct(2);
};


#endif
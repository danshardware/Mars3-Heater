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
//   __A_ 
//  F    B
//  |__G_|
//  E    C
//  |__D_| . (DP)

/*
    In our hardware the pin mapping is:
    A - PC3
    B - PC2
    C - PA4
    D - PB5
    E - PA6
    F - PC0
    G - PC1
    DP - PA5
*/
const segmentBitmask segments[SEGMENT_COUNT] = {
    // PORT A,   PORT B,     PORT C
    {0b00000000, 0b00000000, 0b00001000}, // A
    {0b00000000, 0b00000000, 0b00000100}, // B
    {0b00010000, 0b00000000, 0b00000000}, // C
    {0b00000000, 0b00100000, 0b00000000}, // D
    {0b01000000, 0b00000000, 0b00000000}, // E
    {0b00000000, 0b00000000, 0b00000001}, // F
    {0b00000000, 0b00000000, 0b00000010}, // G
    {0b00100000, 0b00000000, 0b00000000}  // DP
};

/*
    Digit 0 - PB0
    Digit 1 - PB1
*/
const segmentBitmask digitPins[MAX_DIGITS] = {
    {0b00000000, 0b00000001, 0b00000000}, // Digit 0
    {0b00000000, 0b00000010, 0b00000000}, // Digit 1
    {0b00000000, 0b00000000, 0b00000000}, // Digit 2
    {0b00000000, 0b00000000, 0b00000000}  // Digit 3
};

#define SEG_A 0
#define SEG_B 1
#define SEG_C 2
#define SEG_D 3
#define SEG_E 4
#define SEG_F 5
#define SEG_G 6
#define SEG_DP 7

#define SYMBOL_COUNT 18
const char symbolLookup[SYMBOL_COUNT] = {
    '0', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', 'a', 'b', 'c', 'd', 'e', 'f', '-', ' '
};

struct symbol{
    int count;
    char segments[SEGMENT_COUNT];
};

// Define the segments that need to be turned on for each symbol
const symbol symbols[SYMBOL_COUNT] = {
    //
    {6, {SEG_A, SEG_B, SEG_C, SEG_D, SEG_E, SEG_F}}, // 0
    {2, {SEG_B, SEG_C}},    // 1
    {5, {SEG_A, SEG_B, SEG_D, SEG_E, SEG_G}}, // 2
    {5, {SEG_A, SEG_B, SEG_C, SEG_D, SEG_G}}, // 3
    {4, {SEG_B, SEG_C, SEG_F, SEG_G}},    // 4
    {5, {SEG_A, SEG_C, SEG_D, SEG_F, SEG_G}}, // 5
    {6, {SEG_A, SEG_C, SEG_D, SEG_E, SEG_F, SEG_G}}, // 6
    {3, {SEG_A, SEG_B, SEG_C}},    // 7
    {7, {SEG_A, SEG_B, SEG_C, SEG_D, SEG_E, SEG_F, SEG_G}}, // 8
    {5, {SEG_A, SEG_B, SEG_C, SEG_F, SEG_G}}, // 9
    {4, {SEG_C, SEG_D, SEG_E,  SEG_G}}, // a
    {5, {SEG_C, SEG_D, SEG_E, SEG_F, SEG_G}}, // b
    {3, {SEG_D, SEG_E, SEG_G}},    // c
    {5, {SEG_B, SEG_C, SEG_D, SEG_E, SEG_G}},    // d
    {5, {SEG_A, SEG_D, SEG_E, SEG_F, SEG_G}}, // e
    {4, {SEG_A, SEG_E, SEG_F, SEG_G}},  // f
    {1, {SEG_G}},  // -
    {0, {}} // blank
};

enum polarity_t{
    COMMON_ANODE,  // Common anodes light up on an output 0, with the digit being 1
    COMMON_CATHODE, // Common cathodes light up on an output 1, with the digit being 0
    COMMON_ANODE_INV_DIGIT, // Common anodes light up on an output 0, with the digit being 0
    COMMON_CATHODE_INV_DIGIT, // Common anodes light up on an output 1, with the digit being 1
};

#define SET_BY_MASK(port, mask) port |= mask
#define CLEAR_BY_MASK(port, mask) port &= ~mask

class SegmentDisplay{
    public:
        SegmentDisplay(int digits, polarity_t polarity);
        void begin();
        void display(const char *value, int len = MAX_DIGITS);
        void display(const int value);
        void clear(){for (int i = 0; i < MAX_DIGITS; i++) outputBuffer[i] = ' ';};
        void next();
        void test();
        void setDp(bool state){dp = state;};
        void blankDisplay(); // turns off the display
    private:
        segmentBitmask allSegments;
        segmentBitmask allDigits;
        char outputBuffer[MAX_DIGITS];
        polarity_t polarity = COMMON_CATHODE;
        int digits = 0;
        int current_digit = 0;
        int counter = 0;
        bool dp = false;
};


#endif
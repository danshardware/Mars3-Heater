#include "7segment.h"
#include <Arduino.h>

SegmentDisplay::SegmentDisplay(int digits, polarity_t polarity)
{
    this->digits = digits;
    this->polarity = polarity;

    allDigits = {0, 0, 0};
    allSegments = {0, 0, 0};

    // fill the output buffer with sapces
    clear();
    
    // set the masks
    for (int i = 0; i < MAX_DIGITS; i++)
    {
        allDigits.portA |= digitPins[i].portA;
        allDigits.portB |= digitPins[i].portB;
        allDigits.portC |= digitPins[i].portC;
    }

    for (int i = 0; i < SEGMENT_COUNT; i++)
    {
        allSegments.portA |= segments[i].portA;
        allSegments.portB |= segments[i].portB;
        allSegments.portC |= segments[i].portC; 
    }
}

void SegmentDisplay::blankDisplay(){
    // reset the digits to the inactive state
    if(polarity == COMMON_ANODE || polarity == COMMON_CATHODE_INV_DIGIT){
        // port active on 1, so we output 0
        PORTA.OUT &= ~allDigits.portA;
        PORTB.OUT &= ~allDigits.portB;
        PORTC.OUT &= ~allDigits.portC;

    } else {
        // port active on 0, so we output 1
        PORTA.OUT |=  allDigits.portA;
        PORTB.OUT |=  allDigits.portB;
        PORTC.OUT |=  allDigits.portC;

    }

    //segments off
    if (polarity == COMMON_ANODE_INV_DIGIT || polarity == COMMON_ANODE){
        PORTA.OUT |= allSegments.portA;
        PORTB.OUT |= allSegments.portB;
        PORTC.OUT |= allSegments.portC;
    } else {
        PORTA.OUT &= ~allSegments.portA;
        PORTB.OUT &= ~allSegments.portB;
        PORTC.OUT &= ~allSegments.portC;
    }

}

void SegmentDisplay::begin()
{
    // set the port pins as outputs
    PORTA.DIRSET = (allDigits.portA | allSegments.portA);
    PORTB.DIRSET = (allDigits.portB | allSegments.portB);
    PORTC.DIRSET = (allDigits.portC | allSegments.portC);
    blankDisplay();
}

void SegmentDisplay::display(const char *value, int length = MAX_DIGITS)
{
    // copy the value to the output buffer
    if (length > MAX_DIGITS)
    {
        length = MAX_DIGITS;
    }
    strncpy(outputBuffer, value, length);
}

void SegmentDisplay::display(const int value)
{
    // convert value to a string, up to MAX_DIGITS and copy to the output buffer
    itoa(value, outputBuffer, 10);
}

void SegmentDisplay::next()
{
    blankDisplay();
    
    // get the index of the next digit in the symbol lookup table
    int index = 0;
    while (symbolLookup[index] != outputBuffer[current_digit] && index < SYMBOL_COUNT)
    {
        index++;
    }
    
    if(index >= SYMBOL_COUNT){
        index = SYMBOL_COUNT;
    }

    // output the current symbol to the display
    for (int i = 0; i < symbols[index].count; i++)
    {
        if(polarity == COMMON_CATHODE_INV_DIGIT || polarity == COMMON_CATHODE){
            SET_BY_MASK(PORTA.OUT, segments[symbols[index].segments[i]].portA);
            SET_BY_MASK(PORTB.OUT, segments[symbols[index].segments[i]].portB);
            SET_BY_MASK(PORTC.OUT, segments[symbols[index].segments[i]].portC);
        } else {
            CLEAR_BY_MASK(PORTA.OUT, segments[symbols[index].segments[i]].portA);
            CLEAR_BY_MASK(PORTB.OUT, segments[symbols[index].segments[i]].portB);
            CLEAR_BY_MASK(PORTC.OUT, segments[symbols[index].segments[i]].portC);
        }
    }

    // if this is the final digit and the dp bit is set, turn on the decimal point
    if (current_digit == digits - 1 && dp)
    {
        if(polarity == COMMON_CATHODE_INV_DIGIT || polarity == COMMON_CATHODE){
            SET_BY_MASK(PORTA.OUT, segments[SEG_DP].portA);
            SET_BY_MASK(PORTB.OUT, segments[SEG_DP].portB);
            SET_BY_MASK(PORTC.OUT, segments[SEG_DP].portC);
        } else {
            CLEAR_BY_MASK(PORTA.OUT, segments[SEG_DP].portA);
            CLEAR_BY_MASK(PORTB.OUT, segments[SEG_DP].portB);
            CLEAR_BY_MASK(PORTC.OUT, segments[SEG_DP].portC);
        }
    }

    // set the digit output to the active state
    if(polarity == COMMON_ANODE || polarity == COMMON_CATHODE_INV_DIGIT){
        // port active on 1, so we output 1
        PORTA.OUT |= digitPins[current_digit].portA;
        PORTB.OUT |= digitPins[current_digit].portB;
        PORTC.OUT |= digitPins[current_digit].portC;
    } else {
        // port active on 0, so we output 0
        PORTA.OUT &= ~digitPins[current_digit].portA;
        PORTB.OUT &= ~digitPins[current_digit].portB;
        PORTC.OUT &= ~digitPins[current_digit].portC;
    }

    current_digit++;
    if(current_digit >= digits){
        current_digit = 0;
    }
}

void SegmentDisplay::test(){
    // output the current symbol to the display
    for (int i = 0; i < symbols[counter].count; i++)
    {
        if(polarity == COMMON_CATHODE_INV_DIGIT || polarity == COMMON_CATHODE){
            SET_BY_MASK(PORTA.OUT, segments[symbols[counter].segments[i]].portA);
            SET_BY_MASK(PORTB.OUT, segments[symbols[counter].segments[i]].portB);
            SET_BY_MASK(PORTC.OUT, segments[symbols[counter].segments[i]].portC);
        } else {
            CLEAR_BY_MASK(PORTA.OUT, segments[symbols[counter].segments[i]].portA);
            CLEAR_BY_MASK(PORTB.OUT, segments[symbols[counter].segments[i]].portB);
            CLEAR_BY_MASK(PORTC.OUT, segments[symbols[counter].segments[i]].portC);
        }
    }

    // set the digit output to the active state
    if(polarity == COMMON_ANODE || polarity == COMMON_CATHODE_INV_DIGIT){
        // port active on 1, so we output 1
        PORTA.OUT |= digitPins[current_digit].portA;
        PORTB.OUT |= digitPins[current_digit].portB;
        PORTC.OUT |= digitPins[current_digit].portC;
    } else {
        // port active on 0, so we output 0
        PORTA.OUT &= ~digitPins[current_digit].portA;
        PORTB.OUT &= ~digitPins[current_digit].portB;
        PORTC.OUT &= ~digitPins[current_digit].portC;
    }

    counter++;
    if(counter >= SYMBOL_COUNT){
        counter = 0;
    }
    current_digit++;
    if(current_digit >= digits){
        current_digit = 0;
    }
}

// void printDisplayMask(){
//   Serial.print(F("Display Mask\n    PORTA: "));
//   printMask(display.allSegments.portA);
//   Serial.print(F("    PORTB: "));
//   printMask(display.allSegments.portB);
//   Serial.print(F("    PORTC: "));
//   Serial.println();
//   printMask(display.allSegments.portC);
//   Serial.print(F("Digit Mask\n    PORTA: "));
//   printMask(display.allDigits.portA);
//   Serial.print(F("    PORTB: "));
//   printMask(display.allDigits.portB);
//   Serial.print(F("    PORTC: "));
//   printMask(display.allDigits.portC);
//   Serial.println();

// }
#include "7segment.h"
#include <Arduino.h>

SegmentDisplay::SegmentDisplay(int digits, polarity_t polarity, segmentBitmask digitPins[])
{
    this->digits = digits;
    this->polarity = polarity;
    memcpy(this->digitPins, digitPins, digits * sizeof(segmentBitmask));
}

void SegmentDisplay::begin()
{
    // set the port pins as outputs
    uint8_t portMask[3] = {0, 0, 0};
    uint8_t portValue[3] = {0, 0, 0};
    for (int i = 0; i < SEGMENT_COUNT; i++)
    {
        portMask[0] |= segments[i].portA;
        portMask[1] |= segments[i].portB;
        portMask[2] |= segments[i].portC;

    }

    if(polarity == COMMON_ANODE){
        portValue[0] = portMask[0];
        portValue[1] = portMask[1];
        portValue[2] = portMask[2];
    } else {
        portValue[0] = ~portMask[0];
        portValue[1] = ~portMask[1];
        portValue[2] = ~portMask[2];
    }

    for (int i = 0; i < this->digits; i++)
    {
        segmentBitmask d = *(this->digitPins[i]);
        portMask[0] |= d.portA;
        portMask[1] |= d.portB;
        portMask[2] |= d.portC;
        if(polarity == COMMON_ANODE){
            portValue[0] |= d.portA;
            portValue[1] |= d.portB;
            portValue[2] |= d.portC;
        }
    }

    // write port directions and values
    portA->DIR|= portMask[0];
    portB->DIR|= portMask[1];
    portC->DIR|= portMask[2];
    portA->OUT = portA->OUT & portMask[0] | portValue[0];
    portB->OUT = portB->OUT & portMask[1] | portValue[1];
    portC->OUT = portC->OUT & portMask[2] | portValue[2];
    
}

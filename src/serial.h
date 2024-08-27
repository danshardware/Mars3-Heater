#ifndef SERIAL_H
#define SERIAL_H

#include "config.h"
#include <Arduino.h>
#include "calibration.h"
#include "temperature.h"
/*
    * Serial programming functions
*/

#define SERIAL_BUFFER_SIZE 64
const char INVALID_SENSOR_ID[] PROGMEM = "Invalid sensor ID. Should be 0 (ambient) or 1 (heater)";
const char INVALID_TEMP[] PROGMEM = "Invalid temperature - must be between -100 and 100C";
const char INVALID_ADC[] PROGMEM = "Invalid ADC reading - must be between 0 and 1023";
const char INVALID_OFFSET[] PROGMEM = "Invalid offset - must be between -100 and 100";

void setupSerial(){
    Serial.begin(9600);
}

void printHelp(){
    Serial.println("Serial command reference:");
    Serial.println("    t <temp> - set the target temperature");
    Serial.println("    r <sensor (0|1)> - read the temperature from the sensor");
    Serial.println("    c <sensor (0|1)> <actual temp> <ADC Reading> - calibrate the sensor.");
    Serial.println("    o <sensor (0|1)> <offset> - set the offset for the sensor");
    Serial.println("    s - save the calibration data to EEPROM");
    Serial.println("    v - toggle verbose mode");
    Serial.println("    start - begin the heating process");
    Serial.println("    stop - stop regulating temperature");
}

bool checkValidSensor(char *buffer, int bufferLength, int index){
    if(bufferLength <= index){
        Serial.println("Invalid sensor ID");
        return false;
    }
    int sensorId = atoi(&buffer[index]);
    if(sensorId != SENSOR_AMBIENT && sensorId != SENSOR_HEATER){
        Serial.println("Invalid sensor ID");
        return false;
    }
    return true;
}

bool checkValidInt(char *buffer, int bufferLength, int index, long min, long max){
    if(bufferLength <= index){
        return false;
    }

    long val = atol(&buffer[index]);
    if(val < min || val > max){
        return false;
    }
    return true;
}

/*
    Serial command reference:
        - t <temp> - set the target temperature
        - r <sensor (0|1)> - read the temperature from the sensor
        - c <sensor (0|1)> <actual temp> <ADC Reading> - calibrate the sensor. 
        - o <sensor (0|1)> <offset> - set the offset for the sensor
        - s - save the calibration data to EEPROM
        - v - toggle verbose mode
        - h - print the help message
        - 1 - begin the heating process
        - 0 - stop regulating temperature
*/

// function to parse the serial input
void parseSerial(char *buffer, int bufferLength){
    // given a buffer, parse the command
    char command = buffer[0];
    int sensorId, offset;
    long actualTemp, adc;
    
    // first check if the command is a single character
    if(bufferLength == 1 || buffer[1] == ' '){
        switch(command){
            case 't':
                // set the target temperature
                targetTemp = atol(&buffer[2]);
                if (!checkValidInt(buffer, bufferLength, 4, 0, 50)){
                    Serial.println("Invalid Temperature - must be between 0 and 50C");
                    return;
                }
                Serial.print(F("Target temperature set to "));
                Serial.print(targetTemp);
                Serial.println("C");
                break;
            case 'r':
                // read the temperature from the sensor
                bool oldVerbose = verbose;
                verbose = true;
                int sensorId = atoi(&buffer[2]);
                if (!checkValidSensor(buffer, bufferLength, 2)){
                    Serial.println();
                    return;
                }
                long temp = readTemp(sensorId);
                verbose = oldVerbose;
                break;
            case 'c':
                // calibrate the sensor
                sensorId = atoi(&buffer[2]);
                if (!checkValidSensor(buffer, bufferLength, 2)){
                    Serial.println(INVALID_SENSOR_ID);
                    return;
                }
                actualTemp = atol(&buffer[4]);
                if (!checkValidInt(buffer, bufferLength, 4, -100, 100)){
                    Serial.println(INVALID_TEMP);
                    return;
                }
                adc = atol(&buffer[7]);
                if (!checkValidInt(buffer, bufferLength, 7, 0, 1023)){
                    Serial.println(INVALID_ADC);
                    return;
                }
                if (actualTemp < 25){
                    calibration[sensorId].adcLow = adc;
                    calibration[sensorId].tempLow = actualTemp * tempMultiplyFactor;
                } else {
                    calibration[sensorId].adcHigh = adc;
                    calibration[sensorId].tempHigh = actualTemp * tempMultiplyFactor;
                }
                calibration[sensorId].offset = 0;
                break;
            case 'o':
                // set the offset for the sensor
                sensorId = atoi(&buffer[2]);
                if (!checkValidSensor(buffer, bufferLength, 2)){
                    Serial.println(INVALID_SENSOR_ID);
                    return;
                }
                offset = atoi(&buffer[4]);
                if (!checkValidInt(buffer, bufferLength, 4, -100, 100)){
                    Serial.println("Invalid offset");
                    return;
                }
                calibration[sensorId].offset = offset;
                break;
            case 's':
                writeCal();
                break;
            case 'v':
                verbose = !verbose;
                Serial.print("Verbose mode: ");
                Serial.println(verbose ? "ON" : "OFF");
                break;
            case 'h':
                // print the help message
                printHelp();
                break;
            case '1':
                running = true;
                Serial.println("Starting temperature regulation");
                break;
            case '0':
                running = false;
                Serial.println("Stopping temperature regulation");
                break;
            case ' ':
                // do nothing
                break;
            default:
                Serial.print("Unknown command: ");
                Serial.println(command);
                break;
        }
    }
    else {
        Serial.print("Unknown command: ");
        Serial.println(command);
    }
}

// function to read lines from serial without blocking
bool handleSerial(){
    static int bufferIndex = 0;
    static char buffer[SERIAL_BUFFER_SIZE];
    static bool overrun = false;
    static bool init = false;
    const int bufferLength = SERIAL_BUFFER_SIZE;

    // zero out the buffer
    if(!init){
        memset(buffer, 0, bufferLength);
        init = true;
    }

    while(Serial.available() > 0){
        char c = Serial.read();
        // Serial.print("I received: ");
        // Serial.println(c, HEX);
        if(c == '\n' || c == '\r'){
            // parse the buffer
            Serial.println(F("> "));
            if (bufferIndex == 0) return true;
            if (!overrun) parseSerial(buffer, bufferIndex);
            overrun = false;
            memset(buffer, 0, bufferLength);
            bufferIndex = 0;
            return true;
        }
        if (!overrun) {
            // echo back printable characters and store
            if( c== 0x7F || c == 0x08){
                // backspace
                if(bufferIndex > 0){
                    buffer[bufferIndex--] = 0;
                    Serial.write(0x08);
                    Serial.write(' ');
                    Serial.write(0x08);
                }
                return true;
            }

            if(c >= 32 && c <= 126){
                Serial.write(c);
                buffer[bufferIndex++] = c;
            } else {
                Serial.write(0x7); //ASCII bell character
            }
        }
        if(bufferIndex >= bufferLength){
            overrun = true;
            return false;
        }
    }
    return false;
}

#endif
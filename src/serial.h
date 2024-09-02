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
    Serial.begin(115200);
}

#define MAX_TEMP 50
#define MIN_TEMP 0

void printHelp(){
    Serial.println(F("Serial command reference:"));
    Serial.println(F("    t <temp> - set the target temperature"));
    Serial.println(F("    r <sensor (0|1)> - read the temperature from the sensor"));
    Serial.println(F("    c <sensor (0|1)> <actual temp> <ADC Reading> - calibrate the sensor. Low point < 25C, High point > 25C"));
    Serial.println(F("    o <sensor (0|1)> <offset> - set the offset for the sensor"));
    Serial.println(F("    p - Print Calibration Data"));
    Serial.println(F("    s - save the calibration data to EEPROM"));
    Serial.println(F("    v - toggle verbose mode"));
    Serial.println(F("    1 - begin the heating process"));
    Serial.println(F("    0 - stop regulating temperature"));
}

bool checkValidSensor(char *buffer, int bufferLength, int index){
    if(bufferLength <= index){
        Serial.println(F("Invalid sensor ID"));
        return false;
    }
    int sensorId = atoi(&buffer[index]);
    if(sensorId != SENSOR_AMBIENT && sensorId != SENSOR_HEATER){
        Serial.println(F("Invalid sensor ID"));
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
        - p - Print Calibration Data
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
    Serial.print(F("[Parsing] Command: "));
    Serial.print(command);
    Serial.print(F(" bufferLength: "));
    Serial.println(bufferLength);

    if(bufferLength == 1 || buffer[1] == ' '){
        switch(command){
            case ' ':
                // do nothing
                break;
            case '0':
                running = false;
                Serial.println("Stopping temperature regulation");
                break;
            case '1':
                running = true;
                Serial.println("Starting temperature regulation");
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
                    Serial.println(F("Low calibration point set"));
                } else {
                    calibration[sensorId].adcHigh = adc;
                    calibration[sensorId].tempHigh = actualTemp * tempMultiplyFactor;
                    Serial.println(F("High calibration point set"));
                }
                calibration[sensorId].offset = 0;
                break;

            case 'h':
                // print the help message
                printHelp();
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
                    Serial.println(F("Invalid offset"));
                    return;
                }
                calibration[sensorId].offset = offset;
                break;
            case 'p':
                // print the calibration data
                printCalibration();
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
            case 's':
                writeCal();
                break;
            case 't':
                // set the target temperature
                targetTemp = atol(&buffer[2]);

                if (targetTemp < MIN_TEMP || targetTemp > MAX_TEMP){
                    Serial.println(F("Invalid Temperature - must be between 0 and 50C"));
                    return;
                }
                Serial.print(F("Target temperature set to "));
                Serial.print(targetTemp);
                Serial.println(F("C"));
                break;
            case 'v':
                verbose = !verbose;
                Serial.print(F("Verbose mode: "));
                Serial.println(verbose ? "ON" : "OFF");
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
        // Serial.print(F("received: 0x"));
        // Serial.print(c, HEX);
        // Serial.print(F(" bufferIndex: "));
        // Serial.println(bufferIndex);
        if(c == '\r'){
            continue;
        }
        if(c == '\n'){
            // parse the buffer
            if (bufferIndex == 0) return true;
            if (!overrun) {
                buffer[bufferIndex] = 0;
                parseSerial(buffer, bufferIndex);
            }
            Serial.print(F("> "));
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
                continue;
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
            continue;
        }
    }
    return false;
}

#endif
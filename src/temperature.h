#ifndef _TEMPERATURE_H_
#define _TEMPERATURE_H_

#include "config.h"
#include <Arduino.h>
#include "calibration.h"


extern bool verbose; // defined in main.cpp
// Make sure that the suffixes array is the same length as the tempMultiplyFactor
const char *suffixes[] = {".000", ".125", ".250", ".375", ".500", ".625", ".750", ".875"};

const long tempHysteresis = 1;
const long maxHeaterTemp = 80;

void setupAdc(){
  // set the voltage reference to 1.1V
  analogReference(INTERNAL1V1);
}

void printTempVerbose(int sensorId, long temperature, uint32_t adc){
    Serial.print("Sensor: ");
    Serial.print(sensorId == SENSOR_AMBIENT ? "Ambient" : "Heater");
    Serial.print("ADC: ");
    Serial.print(adc);
    Serial.print(", Temp: ");
    Serial.print(temperature / tempMultiplyFactor);
    Serial.print(suffixes[temperature % tempMultiplyFactor]);
    Serial.println("C");
}

/***
 * Read the temperature from the sensor
 * Input: pin - the pin the sensor is connected to
 * Output: the temperature in Celsius, multiplied by tempMultiplyFactor
*/
long readTemp(int sensorId, int oversample = 8){
  uint32_t sum = 0;
  int pin = sensorId == SENSOR_AMBIENT ? tempPinAmbient : tempPinHeater;
  calibrationData *cal = &calibration[sensorId];
  // read the value from the sensor
  for (int counter = 0; counter < oversample; counter++){
    sum += analogRead(pin);
  }
  long val = sum / oversample;

  // We are using a 10K NTC with a beta of 3950
  // NTC is in a voltage divider with a 10K resistor, and in parallel with another 10K resistor
  // ADC is set to 1.1V reference, 10 bits

  long adcOffset = val - (long)cal->adcLow;
  long tempRange = ((long)cal->tempLow - (long)cal->tempHigh);
  long adcRange = (long)cal->adcLow - (long)cal->adcHigh;
  // Serial.print("ADC Offset: ");
  // Serial.print(adcOffset);
  // Serial.print(", Temp Range: ");
  // Serial.print(tempRange);
  // Serial.print(", ADC Range: ");
  // Serial.print(adcRange);

  long temperature = (adcOffset * tempRange) / adcRange + ((long)cal->tempLow + (long)cal->offset);
  
  // Serial.print("Raw Temperature: ");
  // Serial.println(temperature);
  if(verbose){
    printTempVerbose(sensorId, temperature, val);
  }

  return temperature;
}

void printTemps(long tempAmbient, long tempHeater){
  Serial.print("Ambient: ");
  Serial.print(tempAmbient / tempMultiplyFactor);
  Serial.print(suffixes[tempAmbient % tempMultiplyFactor]);
  Serial.print("C, Heater: ");
  Serial.print(tempHeater / tempMultiplyFactor);
  Serial.print(suffixes[tempHeater % tempMultiplyFactor]);
  Serial.println("C");
}

#endif
#include <Arduino.h>
#include "calibration.h"

const int tempPinAmbient = 15;
const int tempPinHeater = 14;
const int heaterOutput = 16;

// Make sure that the suffixes array is the same length as the tempMultiplyFactor
const char *suffixes[] = {".000", ".125", ".250", ".375", ".500", ".625", ".750", ".875"};

const long tempHysteresis = 1;
const long maxHeaterTemp = 80;

// Globals
long targetTemp = 30;
bool running = true;
bool verbose = false;

void setupAdc(){
  // set the voltage reference to 1.1V
  analogReference(INTERNAL1V1);
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
  uint32_t val = sum / oversample;

  // We are using a 10K NTC with a beta of 3950
  // NTC is in a voltage divider with a 10K resistor, and in parallel with another 10K resistor
  // ADC is set to 1.1V reference, 10 bits
  long temperature = map(val, 
    cal->adcHigh, 
    cal->adcLow, 
    (cal->tempHigh * tempMultiplyFactor),
    (cal->tempLow * tempMultiplyFactor)) + 
    cal->offset;
  
  if(verbose){
    Serial.print("Sensor: ");
    Serial.print(sensorId == SENSOR_AMBIENT ? "Ambient" : "Heater");
    Serial.print("ADC: ");
    Serial.print(val);
    Serial.print(", Temp: ");
    Serial.print(temperature / tempMultiplyFactor);
    Serial.print(suffixes[temperature % tempMultiplyFactor]);
    Serial.println("C");
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

void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(heaterOutput, OUTPUT);
  Serial.begin(115200);
  setupAdc();
  getCalibration();
}

// the loop function runs over and over again forever
void loop() {
  digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)
  // Read the temperature from the sensors
  long tempAmbient = readTemp(tempPinAmbient);
  long tempHeater = readTemp(tempPinHeater);

  printTemps(tempAmbient, tempHeater);

  // turn on the heater if needed
  if (tempHeater > maxHeaterTemp * tempMultiplyFactor){
    digitalWrite(heaterOutput, LOW);
  } else if (tempAmbient < targetTemp * tempMultiplyFactor - tempHysteresis * tempMultiplyFactor){
    digitalWrite(heaterOutput, HIGH);
  } else if (tempAmbient > targetTemp * tempMultiplyFactor + tempHysteresis * tempMultiplyFactor){
    digitalWrite(heaterOutput, LOW);
  }

  digitalWrite(LED_BUILTIN, LOW);   // turn the LED off by making the voltage LOW
  delay(1000);                      // wait for a second
}

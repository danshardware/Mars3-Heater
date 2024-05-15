#include <Arduino.h>
#include "config.h"
#include "calibration.h"
#include "serial.h"
#include "temperature.h"
#include <avr/sleep.h>

// Globals
// long targetTemp = 30;
// bool running = true;
// bool verbose = false;
void zzz(){
  sleep_enable();
  set_sleep_mode (SLEEP_MODE_IDLE); 
  sleep_mode();
  sleep_disable();
}

void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(heaterOutput, OUTPUT);
  digitalWrite(heaterOutput, LOW);
  sleep_enable();
  set_sleep_mode (SLEEP_MODE_IDLE); 
  setupSerial();
  Serial.println(F("Starting up"));
  setupAdc();
  getCalibration();
  Serial.println(F("Setup complete"));
}

// the loop function runs over and over again forever
void loop() {
  static uint32_t nextWakeUp = millis() + 1000;

  // Check the serial
  handleSerial();
  if(!running || millis() < nextWakeUp){
    zzz();
    return;
  }

  nextWakeUp = millis() + 1000;
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

  digitalWrite(LED_BUILTIN, LOW);
}

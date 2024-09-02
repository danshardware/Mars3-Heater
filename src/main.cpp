#include <Arduino.h>
#include "config.h"
#include "calibration.h"
#include "serial.h"
#include "temperature.h"
#include "7segment.h"
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


void printPortStautus(){
  Serial.print(F("PORT Outputs\n    PORTA: "));
  Serial.print(PORTA.OUT, BIN);
  Serial.print(F(" PORTB: "));
  Serial.print(PORTB.OUT, BIN);
  Serial.print(F(" PORTC: "));
  Serial.println(PORTC.OUT, BIN);
}

void printPortDriection(){
  Serial.print(F("PORT Direction\n    PORTA: "));
  Serial.print(PORTA.DIR, BIN);
  Serial.print(F(" PORTB: "));
  Serial.print(PORTB.DIR, BIN);
  Serial.print(F(" PORTC: "));
  Serial.println(PORTC.DIR, BIN);
}

SegmentDisplay display(2, COMMON_ANODE_INV_DIGIT);

void printMask(uint8_t mask){
  for(int i = 0; i < 8; i++){
    Serial.print(mask & 128);
    mask <<= 1;
  }
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
  display.begin();
  Serial.println(F("Setup complete"));

  display.display("--aa", 4);
  printPortDriection();
}

// the loop function runs over and over again forever
void loop() {
  static uint32_t nextWakeUp = millis() + 5000;
  static uint16_t counter = 0;
  static int oldAmbient = 0;
  static int oldHeater = 0;

  // Check the serial
  handleSerial();
  display.next();
  if (!running){
    display.display("--", 2);
    digitalWrite(heaterOutput, LOW);
  }
  if(!running || millis() < nextWakeUp){
    zzz();
    return;
  }

  nextWakeUp = millis() + 1000;
  // counter++;
  // digitalWrite(LED_BUILTIN, counter & 1);
  // display.display(counter);
  // if (counter >= 100){
  //   counter = 0;
  // }

  // turn the LED on (HIGH is the voltage level)
  digitalWrite(LED_BUILTIN, HIGH);  

  // Read the temperature from the sensors
  long tempAmbient = readTemp(SENSOR_AMBIENT);
  long tempHeater = readTemp(SENSOR_HEATER);

  if (verbose || tempAmbient != oldAmbient || tempHeater != oldHeater){
    printTemps(tempAmbient, tempHeater);
    oldAmbient = tempAmbient;
    oldHeater = tempHeater;
    if (tempAmbient > 99 * tempMultiplyFactor){
      display.display("hi");
    } else {
      display.display(int(tempAmbient / tempMultiplyFactor));
    }
  }

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

#ifndef _CALIBRATION_H_
#define _CALIBRATION_H_
#include <Arduino.h>
#include <EEPROM.h>

#define SENSOR_AMBIENT 0
#define SENSOR_HEATER  1
int tCalibration = 0; // used only for one-point calibration
const int tempMultiplyFactor = 8;


// structure to hold calibration data
struct calibrationData{
    int tempLow;
    int tempHigh;
    uint16_t adcLow;
    uint16_t adcHigh;
    int offset;
} ;

// structure to hold the calibration data for both the ambient and heater sensors
struct calibrationData calibration[2] = {
    {-6 * tempMultiplyFactor, 110 * tempMultiplyFactor, 1023, 164, 0},
    {-6 * tempMultiplyFactor, 110 * tempMultiplyFactor, 1023, 164, 0}
};

// from https://docs.arduino.cc/learn/programming/eeprom-guide/
unsigned long eeprom_crc(void) {
  const unsigned long crc_table[16] = {
    0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac,
    0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
    0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
    0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c
  };

  unsigned long crc = ~0L;
  int16_t eepromSize = EEPROM.length() - sizeof(unsigned long);
  for (int index = 0 ; index < eepromSize  ; ++index) {
    crc = crc_table[(crc ^ EEPROM[index]) & 0x0f] ^ (crc >> 4);
    crc = crc_table[(crc ^ (EEPROM[index] >> 4)) & 0x0f] ^ (crc >> 4);
    crc = ~crc;
  }
  return crc;
}

bool checkEepromCrc(){
    unsigned long crc = eeprom_crc();
    unsigned long storedCrc;
    EEPROM.get(EEPROM.length() - sizeof(unsigned long), storedCrc);
    return crc == storedCrc;
}

void wipeEeprom(){
    for(int i = 0; i < EEPROM.length(); i++){
        EEPROM.write(i, 0);
    }
}

void writeCal(){
    for(int i = 0; i < 2; i++){
        EEPROM.put(i * sizeof(calibrationData), calibration[i]);
    }
    unsigned long crc = eeprom_crc();
    EEPROM.put(EEPROM.length() - sizeof(unsigned long), crc);
    Serial.println(F("Calibration data written to EEPROM"));
}

void printCalibration(){
    for(int i = 0; i < 2; i++){
        Serial.print(F("Sensor "));
        Serial.print(i == SENSOR_AMBIENT ? F("Ambient") : F("Heater"));
        Serial.print(F(" Low: "));
        Serial.print(calibration[i].tempLow / tempMultiplyFactor);
        Serial.print(F(" High: "));
        Serial.print(calibration[i].tempHigh / tempMultiplyFactor);
        Serial.print(F(" Low ADC: "));
        Serial.print(calibration[i].adcLow);
        Serial.print(F(" High ADC: "));
        Serial.print(calibration[i].adcHigh);
        Serial.print(F(" Offset: "));
        Serial.println(calibration[i].offset);
    }

}
void getCalibration(){
    if (!checkEepromCrc()){
        Serial.println(F("EEPROM CRC check failed, wiping and using defaults"));
        wipeEeprom();
        writeCal();
        return;
    }

    for(int i = 0; i < 2; i++){
        calibrationData data;
        EEPROM.get(i * sizeof(calibrationData), data);
        calibration[i] = data;
    }
    Serial.println(F("Calibration data loaded from EEPROM"));
    printCalibration();
}
#endif
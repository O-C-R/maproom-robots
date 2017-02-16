#include <Wire.h>
#include <SoftwareSerial.h>
#include "AHRSProtocol.h"

byte data[100];

int read16(uint8_t low, uint8_t high){
  return low + (high << 8);
}

bool pretty = true;

void setup() {
  Serial.begin(57600);
  Wire.begin();
}

void loop() {
  int i = 0;
  Wire.beginTransmission(0x32); // NavX is at I2C bus 0x32
  Wire.write(0);
  Wire.write(32);
  Wire.endTransmission();

  Wire.beginTransmission(0x32);
  Wire.requestFrom(0x32,32);
  while(Wire.available()) {
     data[i++] = Wire.read();
  }
  Wire.endTransmission();
  // values are originally -180 to 180, we want all positive numbers to work with
  if (pretty) {
    Serial.println((float((36000 + read16(data[NAVX_REG_YAW_L], data[NAVX_REG_YAW_H]))%36000))/100.0);
  } else {
    Serial.println(36000 + read16(data[NAVX_REG_YAW_L], data[NAVX_REG_YAW_H]));
  }

}

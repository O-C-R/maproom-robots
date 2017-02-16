#include <Wire.h>
#include <SoftwareSerial.h>
#include "./Navx/AHRSProtocol.h"

byte data[512];

int read16(uint8_t low, uint8_t high){
  return low + (high << 8);
}

Class Navx()
{
  int offset;
  int actual;

  Navx()
  {
    Wire.begin();
    offset = 0;
  }

  int get_yaw() {
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
    actual = 36000 + read16(data[NAVX_REG_YAW_L], data[NAVX_REG_YAW_H]);
    return (actual - offset);
  }

  void set_zero() {
    offset = (36000 - actual);
  }
}

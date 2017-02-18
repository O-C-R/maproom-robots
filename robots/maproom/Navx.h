#include <Wire.h>
#include <SoftwareSerial.h>
#include "./includes/AHRSProtocol.h"

byte data[100];

int read16(uint8_t low, uint8_t high) {
  return low + (high << 8);
}

class Navx
{
  bool logging;
  float offset;
  float actual;
  public:

  Navx(bool logging)
  {
    Wire.begin();
    offset = 0.0;
  }

  float getYaw() {
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

    actual = float((36000 + read16(data[NAVX_REG_YAW_L], data[NAVX_REG_YAW_H]))%36000)/100.0;
    float extra = (actual - offset) - floor(actual - offset);
    return (float((int(actual - offset)%360) + extra));
  }

  void calibrate(float newRotation) {
    offset = (actual - newRotation);
    if (logging) {
      Serial.print("Yaw: ");
      Serial.println(actual);
      Serial.print("Offset: ");
      Serial.println(offset);
    }
  }
};

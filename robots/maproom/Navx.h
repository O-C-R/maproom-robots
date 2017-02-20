#include <Wire.h>
#include <SoftwareSerial.h>
#include <math.h>

#include "./includes/AHRSProtocol.h"

byte data[100];

inline int read16(uint8_t low, uint8_t high) {
  return low + (high << 8);
}

class Navx {

public:
  float worldYaw;

  Navx() {
    Wire.begin();
    worldRobotOffset = 0.0;
  }

  float update() {
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

    leftHandMeasured = float((36000 + read16(data[NAVX_REG_YAW_L], data[NAVX_REG_YAW_H]))%36000)/100.0;

    // Swap to use right hand if needed
    rightHandMeasured = 360.0 - leftHandMeasured;

    // Compute world yaw in 0, 360
    worldYaw = fmod(leftHandMeasured - worldRobotOffset + 360.0, 360.0);

    return worldYaw;
  }

  // World is left handed
  void calibrateToWorld(const float worldRotation) {
    worldRobotOffset = leftHandMeasured - worldRotation;
  }

private:
  float rightHandMeasured, leftHandMeasured;
  float worldRobotOffset;
};

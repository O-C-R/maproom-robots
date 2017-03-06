#include <Wire.h>
#include <math.h>

#include "./includes/AHRSProtocol.h"
#include "./includes/I2C_Clear.h"

#define NAVX_TIMEOUT_MILLS 5000

inline int read16(const uint8_t low, const uint8_t high) {
  return low + (high << 8);
}

class Navx {

public:
  byte data[100];

  float worldYaw;

  float measured, measuredLH, measuredRH;
  float worldRobotOffset;

  unsigned long lastAngleChangeTime;

  Navx(): worldRobotOffset(0), worldYaw(0), lastAngleChangeTime(0) {}

  void setup() {
    Serial.println("Clearing I2C bus...");

    int rtn = I2C_ClearBus();
    if (rtn != 0) {
      Serial.println(F("I2C bus error. Could not clear"));
      if (rtn == 1) {
        Serial.println(F("SCL clock line held low"));
      } else if (rtn == 2) {
        Serial.println(F("SCL clock line held low by slave clock stretch"));
      } else if (rtn == 3) {
        Serial.println(F("SDA data line held low"));
      }
    } else {
      Serial.println(F("I2C bus is clear!"));
    }

    Wire.begin(SDA_PIN, SCL_PIN);
  }

  float update() {
    unsigned long now = millis();

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

    measuredLH = float((36000 + read16(data[NAVX_REG_YAW_L], data[NAVX_REG_YAW_H]))%36000)/100.0;
    measuredRH = 360.0 - measuredLH;

    // All other systems are right handed
    measured = measuredRH;

    // Compute world yaw in 0, 360
    float newWorldYaw = fmod(measured - worldRobotOffset + 360.0, 360.0);
    if (abs(worldYaw - newWorldYaw) > 0.0001) {
      lastAngleChangeTime = now;
    }
    if (now - lastAngleChangeTime > NAVX_TIMEOUT_MILLS) {
      Serial.println("ERR:NAVXDOWN");
      lastAngleChangeTime = now;
    }
    worldYaw = newWorldYaw;

    return newWorldYaw;
  }

  // World is left handed
  void calibrateToWorld(const float worldRotation) {
    worldRobotOffset = measured - worldRotation;
  }
};

#include <I2C.h>
#include <math.h>

#include "./includes/AHRSProtocol.h"

#define NAVX 0x32

inline int read16(uint8_t low, uint8_t high) {
  return low + (high << 8);
}

class Navx {

public:
  uint8_t data[100];

  float worldYaw;

  float measured, measuredLH, measuredRH;
  float worldRobotOffset;

  Navx(): worldRobotOffset(0.0) {}

  void setup() {
    I2c.begin();
    I2c.pullup(true);
    I2c.timeOut(200);
  }

  float update() {
    I2c.write(NAVX, 0, 32);
    I2c.read(NAVX, 32, data);

    measuredLH = float((36000 + read16(data[NAVX_REG_YAW_L], data[NAVX_REG_YAW_H]))%36000)/100.0;
    measuredRH = 360.0 - measuredLH;

    // All other systems are right handed
    measured = measuredRH;

    // Compute world yaw in 0, 360
    worldYaw = fmod(measured - worldRobotOffset + 360.0, 360.0);

    return worldYaw;
  }

  // World is left handed
  void calibrateToWorld(const float worldRotation) {
    worldRobotOffset = measured - worldRotation;
  }
};

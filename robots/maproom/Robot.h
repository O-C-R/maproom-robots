#include <math.h>
#include "Motor.h"
#include "Navx.h"
#include "Pen.h"

inline float angleDiff(const float a1, const float a2) {
  float difference = a1 - a2;
  if (difference < -180) difference += 360;
  if (difference > 180) difference -= 360;
  return difference;
}

class Robot
{
  int id;
  int state;

  float targetHeading;
  float remoteHeading;

  float lastNavxHeading;
  float driveHeading;
  float driveMag;

  Motor motorA;
  Motor motorB;
  Motor motorC;
  Navx navx;
  Pen pen;

public:
  Robot() {}
  Robot(int id, int dirA, int pwmA, int dirB, int pwmB, int dirC, int pwmC):
    id(id),
    state(STATE_WAITING),
    motorA(dirA, pwmA, 90.0),
    motorB(dirB, pwmB, 210.0),
    motorC(dirC, pwmC, 330.0)
  {}

  void setState(const int newState) {
    Serial.print("State transition from ");
    Serial.print(state);
    Serial.print(" to ");
    Serial.println(newState);

    state = newState;
  }

  void setPen(const int penState) {
    if (penState == PEN_UP) {
      pen.setState(PEN_UP);
    } else if (penState == PEN_DOWN) {
      pen.setState(PEN_DOWN);
    } else {
      Serial.println("unknown pen state");
    }
  }

  void stateRotate() {
    const float worldYaw = navx.worldYaw;

    const float headingDiff = angleDiff(targetHeading, worldYaw);
    const float absHeadingDiff = abs(headingDiff);

    Serial.print("Heading diff: ");
    Serial.println(headingDiff);

    if (absHeadingDiff < ROTATION_ERROR) {
      Serial.print("Done rotating, got to: ");
      Serial.println(worldYaw);

      stop();
      setState(STATE_WAITING);

      return;
    }

    float rotationSpeed = map(absHeadingDiff, 0.0, 180.0, ROTATION_SPEED_MIN, ROTATION_SPEED_MAX);
    rotate(rotationSpeed * (headingDiff > 0 ? -1.0 : 1.0));
  }

  inline void stop() {
    motorA.stop();
    motorB.stop();
    motorC.stop();
  }

  inline void commandMotors() {
    motorA.commandMotor();
    motorB.commandMotor();
    motorC.commandMotor();
  }

  void update() {
    navx.update();
    const unsigned long now = millis();

    if (state == STATE_WAITING) {
      if (pen.state != PEN_UP) {
        pen.setState(PEN_UP);
      }

      stop();
    } else if (state == STATE_ROTATING) {
      stateRotate();
    } else if (state == STATE_POSITIONING) {
      if (pen.state != PEN_UP || (pen.state == PEN_UP && !pen.doneChanging(now))) {
        pen.setState(PEN_UP);
        stop();
      } else {
        driveDirection();
      }
    } else if (state == STATE_DRAWING) {
      if (pen.state != PEN_DOWN || (pen.state == PEN_DOWN && !pen.doneChanging(now))) {
        pen.setState(PEN_DOWN);
        stop();
      } else {
        driveDirection();
      }
    }

    commandMotors();
  }

  void commandCalibrate(const float worldAngle) {
    navx.calibrateToWorld(worldAngle);

#if LOGGING
    const float worldYaw = navx.update();

    Serial.print("Calibrating yaw, world angle is: ");
    Serial.println(worldAngle);
    Serial.print("Navx reports angle is: ");
    Serial.println(worldYaw);
#endif
  }

  void commandRotate(const float target, const float measured) {
    setState(STATE_ROTATING);

    targetHeading = target;
    remoteHeading = measured;
    navx.calibrateToWorld(measured);

#if LOGGING
    Serial.print("ROTATING to headingDegree: ");
    Serial.print(targetHeading);
    Serial.print(" got measured ");
    Serial.println(remoteHeading);
#endif
  }

  void commandPosition(const float dir, const int mag, const int measured) {
    setState(STATE_POSITIONING);

    driveHeading = dir;
    driveMag = mag;

    lastNavxHeading = navx.worldYaw;
    navx.calibrateToWorld(measured);

#if LOGGING
    Serial.print("POSITIONING – heading: ");
    Serial.println(driveHeading);
    Serial.print("mag: ");
    Serial.println(driveMag);
#endif
  }

  void commandDraw(const float dir, const int mag, const int measured) {
    setState(STATE_DRAWING);

    driveHeading = dir;
    driveMag = mag;

    lastNavxHeading = navx.worldYaw;
    navx.calibrateToWorld(measured);

#if LOGGING
    Serial.print("DRAWING – heading: ");
    Serial.println(driveHeading);
    Serial.print("mage: ");
    Serial.println(driveMag);
#endif
  }

  void commandStop() {
    setState(STATE_WAITING);
  }

  void driveDirection() {
    const float worldHeading = navx.worldYaw;
    const float direction = fmod(driveHeading - worldHeading + 360.0, 360.0);

    motorA.driveVector(direction, driveMag, 0);
    motorB.driveVector(direction, driveMag, 0);
    motorC.driveVector(direction, driveMag, 0);
  }

  void driveSpecific(int w0, int w1, int w2) {
    motorA.driveConstant(w0);
    motorB.driveConstant(w1);
    motorC.driveConstant(w2);
  }

  void rotate(float speed) {
    motorA.driveConstant(speed);
    motorB.driveConstant(speed);
    motorC.driveConstant(speed);
  }

};

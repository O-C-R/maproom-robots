#include "Motor.h"
#include "Navx.h"
#include "Marker.h"
#include <math.h>

// States
#define STATE_WAITING 0
#define STATE_ROTATING 1
#define STATE_POSITIONING 2
#define STATE_DRAWING 3
#define STATE_MOVING_MARKER 4

// Pen states
#define MARKER_DOWN 1
#define MARKER_UP 0

// Speeds
#define ROTATION_ERROR 1.0
#define ROTATION_SPEED 126

class Robot
{
  bool logging;
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
  Marker marker;

public:
  Robot() {}
  Robot(int id, int dirA, int pwmA, int dirB, int pwmB, int dirC, int pwmC, bool setLogging):
    id(id),
    logging(setLogging),
    state(STATE_WAITING),
    motorA(dirA, pwmA, 0.0, logging),
    motorB(dirB, pwmB, 120.0, logging),
    motorC(dirC, pwmC, 240.0, logging),
    marker(0)
  {}

  void stateRotate() {
    // if (abs(navxRotation - headingDegree) <  ROTATION_ERROR) {
    //   Serial.println("ALIGNED");
    //   stop();
    // } else {
    //   Serial.print("upper: ");
    //   Serial.print(headingDegree + ROTATION_ERROR);
    //   Serial.print(" lower: ");
    //   Serial.print(headingDegree - ROTATION_ERROR);

    //   Serial.print(" HEADING: ");
    //   Serial.print(headingDegree);
    //   Serial.print(" CURRENT: ");
    //   Serial.print(navxRotation);

    //   int switchDirection = (abs(headingDegree-navxRotation) > 180 ? -1 : 1);
    //   if (navxRotation > headingDegree) {
    //     Serial.println(switchDirection == -1 ? " ROTATING CW" : " ROTATING CCW" );
    //     rotate(switchDirection * ROTATION_SPEED);
    //   } else {
    //     Serial.println(switchDirection == -1 ? " ROTATING CCW" : " ROTATING CW" );
    //     rotate(switchDirection * -ROTATION_SPEED);
    //   }
    // }
  }

  void update() {
    const float worldYaw = navx.update();

    if (state == STATE_WAITING) {
      if (marker.getPosition() != MARKER_UP) {
        marker.setPosition(MARKER_UP);
      }

      stop();
    } else if (state == STATE_ROTATING) {
      //stateRotate();

      stop();
    } else if (state == STATE_POSITIONING) {
      if (marker.getPosition() != MARKER_UP) {
        marker.setPosition(MARKER_UP);
      }

      driveDirection();
    } else if (state == STATE_DRAWING) {
      if (marker.getPosition() != MARKER_DOWN) {
        marker.setPosition(MARKER_DOWN);
      }

      driveDirection();
    }

    commandMotors();
  }

  void stop() {
    motorA.stop();
    motorB.stop();
    motorC.stop();
  }

  void commandMotors() {
    motorA.commandMotor();
    motorB.commandMotor();
    motorC.commandMotor();
  }

  void commandCalibrate(const float worldAngle) {
    navx.calibrateToWorld(worldAngle);
    const float worldYaw = navx.update();

#if LOGGING
    Serial.print("Calibrating yaw, world angle is: ");
    Serial.println(worldAngle);
    Serial.print("Navx reports angle is: ");
    Serial.println(worldYaw);
#endif
  }

  void commandRotate(const float target, const float measured) {
    state = STATE_ROTATING;

    targetHeading = target;
    remoteHeading = measured;

#if LOGGING
    Serial.print("ROTATING to headingDegree: ");
    Serial.print(headingDegree);
    Serial.print(" got measured ");
    Serial.println(measured);
#endif
  }

  void commandPosition(const float dir, const long mag) {
    state = STATE_POSITIONING;

    driveHeading = dir;
    driveMag = mag;
    lastNavxHeading = navx.worldYaw;

#if LOGGING
    Serial.print("POSITIONING – heading: ");
    Serial.println(driveHeading);
    Serial.print("mag: ");
    Serial.println(driveMag);
#endif
  }

  void commandDraw(const float dir, const long mag) {
    state = STATE_DRAWING;

    driveHeading = dir;
    driveMag = mag;

#if LOGGING
    Serial.print("DRAWING – heading: ");
    Serial.println(driveHeading);
    Serial.print("mage: ");
    Serial.println(driveMag);
#endif
  }

  void commandStop() {
    state = STATE_WAITING;
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

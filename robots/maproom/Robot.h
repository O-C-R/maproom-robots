#include <math.h>
#include "Motor.h"
#include "Navx.h"
#include "Pen.h"

//Specify the links and initial tuning parameters
#define YAW_PID_KP 12.0
#define YAW_PID_KI 0.3
#define YAW_PID_KD 10.0

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

  MiniPID yawPID;
  float yawPIDOutput;
  float worldYaw;
  float targetHeading;

  float driveHeading;
  float driveMag;

  Motor motorA;
  Motor motorB;
  Motor motorC;
  Navx navx;
  Pen pen;

public:
  Robot(int id, int dirA, int pwmA, int dirB, int pwmB, int dirC, int pwmC):
    id(id),
    state(STATE_WAITING),
    targetHeading(0),
    yawPIDOutput(0),
    motorA(dirA, pwmA, 0.0),
    motorB(dirB, pwmB, 120.0),
    motorC(dirC, pwmC, 240.0),
    yawPID(YAW_PID_KP, YAW_PID_KI, YAW_PID_KD)
  {
    yawPID.setOutputLimits(-200.0, 200.0);
    yawPID.setMaxIOutput(50.0);
  }

  Robot(): Robot(-1, -1, -1, -1, -1, -1, -1) {}

  void setup() {
    pen.setup();
    setPen(PEN_UP);
    stop();
    navx.setup();
  }

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
    rotate(yawPIDOutput);
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

  void updatePID() {
    const float headingDiff = angleDiff(targetHeading, worldYaw);
    yawPIDOutput = yawPID.getOutput(headingDiff, 0);
  }

  void update() {
    const unsigned long now = millis();

    worldYaw = navx.update();

    if (state == STATE_WAITING) {
      stop();
    } else if (state == STATE_ROTATING) {
      updatePID();
      stateRotate();
    } else if (state == STATE_POSITIONING) {
      if (pen.state != PEN_UP) {
        pen.setState(PEN_UP);
      }

      if (pen.state == PEN_UP && !pen.doneChanging(now)) {
        stop();
      } else {
        updatePID();
        driveDirection();
      }
    } else if (state == STATE_DRAWING) {
      if (pen.state != PEN_DOWN) {
        pen.setState(PEN_DOWN);
      }

      if (pen.state == PEN_DOWN && !pen.doneChanging(now)) {
        stop();
      } else {
        updatePID();
        driveDirection();
      }
    }

    commandMotors();
  }

  void commandCalibrate(const float worldAngle) {
    navx.calibrateToWorld(worldAngle);
    yawPID.reset();

#if LOGGING
    worldYaw = navx.update();

    Serial.print("Calibrating yaw, world angle is: ");
    Serial.println(worldAngle);
    Serial.print("Navx reports angle is: ");
    Serial.println(worldYaw);
#endif
  }

  void commandRotate(const float target, const float measured) {
    if (state != STATE_ROTATING) {
      setState(STATE_ROTATING);

      navx.calibrateToWorld(measured);
      yawPID.reset();
    }

    targetHeading = target;

#if LOGGING
    Serial.print("ROTATING to headingDegree: ");
    Serial.print(targetHeading);
    Serial.print(" got measured ");
    Serial.println(measured);
#endif
  }

  void commandPosition(const float dir, const int mag, const int measured) {
    if (state != STATE_POSITIONING) {
      setState(STATE_POSITIONING);

      navx.calibrateToWorld(measured);
      worldYaw = navx.update();
      targetHeading = worldYaw;
      yawPID.reset();
    }

    driveHeading = dir;
    driveMag = mag;

#if LOGGING
    Serial.print("POSITIONING – heading: ");
    Serial.println(driveHeading);
    Serial.print("mag: ");
    Serial.println(driveMag);
#endif
  }

  void commandDraw(const float dir, const int mag, const int measured) {
    if (state != STATE_DRAWING) {
      setState(STATE_DRAWING);

      navx.calibrateToWorld(measured);
      worldYaw = navx.update();
      targetHeading = worldYaw;
      yawPID.reset();
    }

    driveHeading = dir;
    driveMag = mag;

#if LOGGING
    Serial.print("DRAWING – heading: ");
    Serial.println(driveHeading);
    Serial.print("mag: ");
    Serial.println(driveMag);
#endif
  }

  void commandStop() {
    setState(STATE_WAITING);
  }

  void driveDirection() {
    const float direction = fmod(driveHeading - worldYaw + 360.0, 360.0);

    motorA.driveVector(direction, driveMag, yawPIDOutput);
    motorB.driveVector(direction, driveMag, yawPIDOutput);
    motorC.driveVector(direction, driveMag, yawPIDOutput);
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

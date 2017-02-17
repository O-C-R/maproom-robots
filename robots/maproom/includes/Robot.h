#include "Motor.h"
#include "Navx.h"

#define STATE_WAITING 0
#define STATE_ROTATING 1
#define STATE_MOVING 2

#define ROTATION_ERROR 1

class Robot
{
  int id;
  int state;
  int rotation;

  bool logging; // flag for logging

  int headingTheta;
  int headingMag;
  float headingDegree;

  Motor * motorA;
  Motor * motorB;
  Motor * motorC;
  Navx * navx;

  public:
  Robot(int id, int dirA, int pwmA, int dirB, int pwmB, int dirC, int pwmC, bool logging)
  {
    id = id;
    logging = logging;

    motorA = new Motor(dirA, pwmA, 0.0, logging);
    motorB = new Motor(dirB, pwmB, 120.0, logging);
    motorC = new Motor(dirC, pwmC, 240.0, logging);

    navx = new Navx(logging);
    navx->setZero();
    rotation = navx->getYaw();
  }

  void checkStatus() {
    if (state = STATE_WAITING) return;
    if (rotation < headingDegree + ROTATION_ERROR && rotation > headingDegree + ROTATION_ERROR) {
      // robot is aligned with heading
      state = STATE_MOVING;
    }
  }

  void commandMotors() {
    if (logging) {
      Serial.println("output:");
      Serial.println("-----");
    }
    motorA->commandMotor();
    motorB->commandMotor();
    motorC->commandMotor();
  }

  void calibrateYaw() {
    if(logging) {
      Serial.println("Calibrating Yaw");
    }
    navx->setZero();
  }

  void stop() {
    state = STATE_WAITING;
    motorA->stop();
    motorB->stop();
    motorC->stop();

    commandMotors();
  }

  void setHeading(long x, long y) {
    headingTheta = atan2(y, x);
    headingDegree = headingTheta * 180 / 3.14159;
    headingMag = sqrt((x*x)+(y*y));
  }

  void rotate(int dir, int mag) {
    if(logging) {
      Serial.println("rotating:");
      Serial.println(dir > 0 ? "CW" : "CCW");
      Serial.println("mag");
    }
    motorA->driveConstant(dir, mag);
    motorB->driveConstant(dir, mag);
    motorC->driveConstant(dir, mag);
  }

  void drive() {
    if(logging) {
      Serial.println("dir:");
      Serial.println(headingTheta);
      Serial.println(headingMag);
      Serial.println("wheels:");
    }

    motorA->driveVector(headingTheta, headingMag);
    motorB->driveVector(headingTheta, headingMag);
    motorC->driveVector(headingTheta, headingMag);

    commandMotors();
  }

  void goSpecific(int w0, int w1, int w2) {
    // motorA->
  }
};

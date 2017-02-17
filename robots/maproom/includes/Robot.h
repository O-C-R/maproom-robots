#include "Motor.h"
#include "Navx.h"

class Robot
{
  int id;

  int headingTheta;
  int headingMag;

  bool status; // flag for logging

  Motor * motorA;
  Motor * motorB;
  Motor * motorC;

  public:
  Robot(int id, int dirA, int pwmA, int dirB, int pwmB, int dirC, int pwmC, bool status)
  {
    id = id;
    status = status;

    motorA = new Motor(dirA, pwmA, 0.0);
    motorB = new Motor(dirB, pwmB, 120.0);
    motorC = new Motor(dirC, pwmC, 240.0);
  }

  void commandMotors() {
    if (status) {
      Serial.println("output:");
      Serial.println("-----");
    }

    motorA->commandMotor(status);
    motorB->commandMotor(status);
    motorC->commandMotor(status);
  }

  void stop() {
    motorA->stop();
    motorB->stop();
    motorC->stop();
  }

  void setHeading(long x, long y) {
    headingTheta = atan2(y, x);
    headingMag = sqrt((x*x)+(y*y));

    if(status) {
      Serial.println("dir:");
      Serial.println(x);
      Serial.println(y);
      Serial.println(headingTheta);
      Serial.println(headingMag);
      Serial.println("wheels:")
    }
    motorA->go(x, y, headingTheta, headingMag, status);
    motorB->go(x, y, headingTheta, headingMag, status);
    motorC->go(x, y, headingTheta, headingMag, status);
  }

  void goSpecific(int w0, int w1, int w2) {
    motorA->
  }
};

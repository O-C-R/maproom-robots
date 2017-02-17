#include "Motor.h"
#include "Navx.h"

#define STATE_WAITING 0
#define STATE_ROTATING 1
#define STATE_MOVING 2

#define ROTATION_ERROR 1
#define ROTATION_SPEED 126

class Robot
{
  bool logging;
  int id;
  int state;
  float rotation;

  int headingTheta;
  int headingMag;
  float headingDegree;

  Motor *motorA;
  Motor *motorB;
  Motor *motorC;
  Navx *navx;

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

  bool turningCW = false;

  void update() {
    if (state == STATE_WAITING) return;
    if (state == STATE_ROTATING) {
      if (rotation < headingDegree + ROTATION_ERROR && rotation > headingDegree + ROTATION_ERROR) {
        Serial.println("ALIGNED");
        stop();
      } else {
        Serial.print("HEADING ");
        Serial.println(headingDegree);
        Serial.print("CURRENT ANGLE ");
        Serial.println(rotation);

        if (rotation < headingDegree) {
          // wheel needs to turn CCW
          if (turningCW) {
            turningCW = false;
            Serial.println("ROTATING CCW");
            rotate(-ROTATION_SPEED);
          }
        } else {
          // wheels need to move CW
          if (!turningCW) {
            turningCW = true;
            Serial.println("ROTATING CW");
            rotate(ROTATION_SPEED);
          }
        }
      }
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

  void getYaw() {
    rotation = navx->getYaw();
    if(logging) {
      Serial.println(rotation);
    }
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

    drive();
  }

  void rotate(int speed) {
    if(logging) {
      Serial.println("rotating:");
      Serial.println(speed > 0 ? "CW" : "CCW");
      Serial.println("speed");
    }
    motorA->driveConstant(speed);
    motorB->driveConstant(speed);
    motorC->driveConstant(speed);

    commandMotors();
  }

  void rotateSpecific(int angle) {
    state = STATE_ROTATING;
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

  void driveSpecific(int w0, int w1, int w2) {
    motorA->driveConstant(w0);
    motorB->driveConstant(w1);
    motorC->driveConstant(w2);

    commandMotors();
  }
};

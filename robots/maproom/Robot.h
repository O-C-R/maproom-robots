#include "Motor.h"
#include "Navx.h"

#define STATE_WAITING 0
#define STATE_ROTATING 1
#define STATE_DRIVING 2

#define ROTATION_ERROR 1.0
#define ROTATION_SPEED 126

class Robot
{
  bool logging;
  int id;
  int state;
  float rotation;

  float headingTheta;
  float headingMag;
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

  void update() {
    if (state == STATE_WAITING) return;
    // update rotation from navX
    getYaw();
    if (state == STATE_ROTATING) {
      if (rotation < headingDegree + ROTATION_ERROR && rotation > headingDegree - ROTATION_ERROR) {
        Serial.println("ALIGNED");
        stop();
      } else {
        Serial.print("upper: ");
        Serial.print(headingDegree + ROTATION_ERROR);
        Serial.print(" lower: ");
        Serial.print(headingDegree - ROTATION_ERROR);

        Serial.print(" HEADING: ");
        Serial.print(headingDegree);
        Serial.print(" CURRENT: ");
        Serial.print(rotation);

        if (rotation > headingDegree) {
          // wheel needs to turn CCW
            Serial.println(" ROTATING CCW");
            rotate(-ROTATION_SPEED);
        } else {
          // wheels need to move CW
            Serial.println(" ROTATING CW");
            rotate(ROTATION_SPEED);
        }
      }
    } else if (state == STATE_DRIVING) {
      drive();
    }
  }

  void commandMotors() {
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
  }

  void stop() {
    state = STATE_WAITING;
    motorA->stop();
    motorB->stop();
    motorC->stop();

    commandMotors();
  }

  void rotateStart(float angle) {
    state = STATE_ROTATING;
    headingDegree = angle;
    Serial.print("ROTATING – headingDegree: ");
    Serial.println(headingDegree);
  }

  void rotate(float speed) {
    motorA->driveConstant(speed);
    motorB->driveConstant(speed);
    motorC->driveConstant(speed);

    commandMotors();
  }

  void driveStart(long x, long y) {
    state = STATE_DRIVING;
    headingTheta = atan2(y, x);
    headingDegree = headingTheta * 180 / 3.14159;
    headingMag = sqrt((x*x)+(y*y));
    Serial.print("DRIVING – headingTheta: ");
    Serial.println(headingTheta);
    Serial.print("headingDegree: ");
    Serial.println(headingDegree);
    Serial.print("headingMag: ");
    Serial.println(headingMag);
  }

  void drive() {
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

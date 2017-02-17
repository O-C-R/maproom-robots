#include "Motor.h"
#include "Navx.h"

class Robot
{
  int id;
  int orientation;
  bool status;

  // todo: figure out notation for motor orientation
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

  void commandMotors(bool status) {
    if (status) {
      Serial.println("output:");
      Serial.println("-----");
    }

    motorA->commandMotor(status);
    motorB->commandMotor(status);
    motorC->commandMotor(status);
  }

  void allStop() {
    motorA->stop();
    motorB->stop();
    motorC->stop();
  }

  // void easeMotors() {
  //   motorA->ease();
  //   motorB->ease();
  //   motorC->ease();
  // }

  void motorsGo(long x, long y, bool status) {
    motorA->go(x, y, status);
    motorB->go(x, y, status);
    motorC->go(x, y, status);
  }
};

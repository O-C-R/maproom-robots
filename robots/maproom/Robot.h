#include "Motor.h"
#include "Navx.h"
#include "Marker.h"

#define STATE_WAITING 0
#define STATE_ROTATING 1
#define STATE_POSITIONING 2
#define STATE_DRAWING 3
#define STATE_MOVING_MARKER 4

#define MARKER_DOWN 1
#define MARKER_UP 0

#define ROTATION_ERROR 1.0
#define ROTATION_SPEED 126

class Robot
{
  bool logging;
  int id;
  int state;

  float headingMag;
  float headingDegree;

  Motor motorA;
  Motor motorB;
  Motor motorC;
  Navx navx;
  Marker marker;

  public:
  Robot() {}
  Robot(int id, int dirA, int pwmA, int dirB, int pwmB, int dirC, int pwmC, bool logging)
  {
    id = id;
    logging = logging;

    state = STATE_WAITING;

    motorA = Motor(dirA, pwmA, 0.0, logging);
    motorB = Motor(dirB, pwmB, 120.0, logging);
    motorC = Motor(dirC, pwmC, 240.0, logging);

    navx = Navx(logging);
    // navx.calibrate(0);

    marker = Marker(0);
  }

  void cycle() {
    if (state == STATE_WAITING) {
      if (marker.getPosition() != MARKER_UP ) {
         marker.setPosition(MARKER_UP);
         return;
      }
      return;

    }

    if (state == STATE_ROTATING) {
      float rotation = navx.getYaw();

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
            rotate(ROTATION_SPEED);
        } else {
          // wheels need to move CW
            Serial.println(" ROTATING CW");
            rotate(-ROTATION_SPEED);
        }
      }
    } else if (state == STATE_POSITIONING) {
       if (marker.getPosition() != MARKER_UP ) {
         marker.setPosition(MARKER_UP);
         return;
       }
      drive();
    } else if (state == STATE_DRAWING) {
       if (marker.getPosition() != MARKER_DOWN ) {
         marker.setPosition(MARKER_DOWN);
         return;
       }
      drive();
    }
  }

  void commandMotors() {
    motorA.commandMotor();
    motorB.commandMotor();
    motorC.commandMotor();
  }

  void calibrate(float newAngle) {
    if(logging) {
      Serial.println("Calibrating Yaw");
    }
    navx.calibrate(newAngle);
  }

  float getRotation() {
    return navx.getYaw();
  }

  void stop() {
    state = STATE_WAITING;
    motorA.stop();
    motorB.stop();
    motorC.stop();

    commandMotors();
  }

  void rotateManager(float angle, float recorded) {
    state = STATE_ROTATING;
    headingDegree = angle;
    Serial.print("ROTATING – headingDegree: ");
    Serial.println(headingDegree);
  }

  void rotate(float speed) {
    motorA.driveConstant(speed);
    motorB.driveConstant(speed);
    motorC.driveConstant(speed);

    commandMotors();
  }

  void driveManager(float dir, long mag) {
    state = STATE_POSITIONING;

    headingDegree = dir;
    headingMag = mag;
    Serial.print("POSITIONING – headingDegree: ");
    Serial.println(headingDegree);
    Serial.print("headingMag: ");
    Serial.println(headingMag);
  }

  void drive() {
    motorA.driveVector(headingDegree, headingMag);
    motorB.driveVector(headingDegree, headingMag);
    motorC.driveVector(headingDegree, headingMag);

    commandMotors();
  }

  void driveSpecific(int w0, int w1, int w2) {
    motorA.driveConstant(w0);
    motorB.driveConstant(w1);
    motorC.driveConstant(w2);

    commandMotors();
  }

  void drawManager(float dir, long mag) {
    state = STATE_DRAWING;

    headingDegree = dir;
    headingMag = mag;
    Serial.print("DRAWING – headingDegree: ");
    Serial.println(headingDegree);
    Serial.print("headingMag: ");
    Serial.println(headingMag);
  }

};

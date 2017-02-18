#include "Robot.h"

#define LOGGING 1

#define pwmA 6
#define dirA 7

#define pwmB 5
#define dirB 4

#define pwmC 11
#define dirC 12

typedef struct {
  int pulse;
  bool direction;

  float current;
  float target;
} MotorValues;

MotorValues motorA;
MotorValues motorB;
MotorValues motorC;

Robot * warby;

char *buf;
int bufLen;
bool bufDone;

#define BUF_SIZE 1024

void setup() {
  warby = new Robot(0, dirA, pwmA, dirB, pwmB, dirC, pwmC, true);
  warby->stop();

  buf = (char *)malloc(BUF_SIZE * sizeof(char));
  bufLen = 0;
  bufDone = false;

  Serial.begin(57600);
  Serial.println("Looping...");
}

void handleMessage(char *buf) {
  if (buf[0] != 'M' && buf[1] != 'R') return;

  if (buf[2] == 'M' && buf[3] == 'O' && buf[4] == 'V') {
    // MOVE COMMAND

    byte val = buf[11];
    buf[11] = 0;
    int dir = atoi(buf + 5);
    buf[11] = val;
    int mag = atoi(buf + 11);

    warby->driveStart(dir, mag);
  } else if (buf[2] == 'R' && buf[3] == 'O' && buf[4] == 'T') {
    // ROTATE COMMAND

    byte val = buf[11];
    buf[11] = 0;
    int desiredAngle = atoi(buf + 5);
    buf[11] = val;
    int measuredAngle = atoi(buf + 11);
    Serial.println("ROTATE MESSAGE");
    Serial.print("desiredAngle: ");
    Serial.print(desiredAngle);
    Serial.print("measuredAngle: ");
    Serial.print(measuredAngle);

    warby->rotateStart(desiredAngle, measuredAngle);

  } else if (buf[2] == 'X' && buf[3] == 'D' && buf[4] == 'R') {
    // DEBUG

    byte val = buf[11];
    buf[11] = 0;
    int w0_mag = atoi(buf + 5);
    buf[11] = val;
    val = buf[17];
    buf[17] = 0;
    int w1_mag = atoi(buf + 11);
    buf[17] = val;
    int w2_mag = atoi(buf + 17);

    warby->driveSpecific(w0_mag, w1_mag, w2_mag);
  } else if (buf[2] == 'C' && buf[3] == 'A' && buf[4] == 'L') {
    // CALIBRATE

    int newRotation = atoi(buf + 5);
    warby->calibrate(newRotation);

  } else {
    Serial.print("Unknown message: ");
    Serial.println(buf);

    warby->stop();
  }
}

int wait = 0;

void loop() {
  while (Serial.available()) {
    char inChar = (char)Serial.read();

    if (bufLen >= BUF_SIZE - 1) {
      Serial.println("BUF OVERRUN");
      continue;
    }

    if (!bufDone && inChar == '\n') {
      buf[bufLen++] = 0;
      bufDone = true;
    } else if (!bufDone) {
      buf[bufLen++] = inChar;
    }
  }

  if (bufDone) {
    Serial.print("buf: ");
    Serial.write(buf, bufLen);
    Serial.println();
    handleMessage(buf);

    bufDone = false;
    bufLen = 0;
  }

  wait++;
  if (wait > 5000) {
//    warby->getYaw();
//    warby->setHeading(0, 300);
//    warby->rotate(-100.0);
//    warby->rotateSpecific(45.00);
    warby->update();
    wait = 0;
  }
}


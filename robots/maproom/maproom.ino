#include "Robot.h"

#define LOGGING true

#define ROBOT_ID_0 '0'
#define ROBOT_ID_1 '1'

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
  warby = new Robot(0, dirA, pwmA, dirB, pwmB, dirC, pwmC, LOGGING);
  warby->stop();

  buf = (char *)malloc(BUF_SIZE * sizeof(char));
  bufLen = 0;
  bufDone = false;

  Serial.begin(57600);
  Serial.println("Looping...");
}

// MR01CMDPAR1PAR2
// 012345678901234

// MR01MOV05120512
// MR01MOV06120612
// MR01MOV04120412

void handleMessage(char *buf) {
  if (buf[0] != 'M' && buf[1] != 'R') return;
  if (buf[2] != ROBOT_ID_0 && buf[3] != ROBOT_ID_1) return;

  if (buf[4] == 'M' && buf[5] == 'O' && buf[6] == 'V') {
    byte val = buf[11];
    buf[11] = 0;
    int param1 = atoi(buf + 7);
    buf[11] = val;
    int param2 = atoi(buf + 11);

    warby->setHeading(param1 - 500, param2 - 500);

  } else if (buf[4] == 'R' && buf[5] == 'O' && buf[6] == 'T') {
    byte val = buf[11];
    buf[11] = 0;
    int param1 = atoi(buf + 7);
    buf[11] = val;

    warby->rotateSpecific(param1 - 500);

  } else if (buf[4] == 'S' && buf[5] == 'E' && buf[6] == 'T') {
    byte val = buf[11];
    buf[11] = 0;
    int param1 = atoi(buf + 7);
    buf[11] = val;

    val = buf[15];
    buf[15] = 0;
    int param2 = atoi(buf + 11);
    buf[15] = val;
    int param3 = atoi(buf + 15);

    warby->driveSpecific(param1 - 500, param2 - 500, param3 - 500);

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
  if (wait > 100) {
//    warby->update();
    warby->getYaw();
    wait = 0;
  }
}


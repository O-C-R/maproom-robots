#include <Servo.h>
#include <MiniPID.h>

#include "Constants.h"
#include "Robot.h"

// Change both of these if the ID of the robot changes
#define ROBOT_ID 2
#define HEARTBEAT_MSG "SENRB02HB"

// Serial options
#define HEARTBEAT 1
#define HEARTBEAT_TIMEOUT_MILLIS 500
#define MSG_TIMEOUT_DRAWING_MILLIS 500
#define MSG_TIMEOUT_MILLIS 1000

// Msg buffer
#define BUF_SIZE 125
#define BUF_VAL_WIDTH 6

char buf[BUF_SIZE], buf2[BUF_SIZE];
int bufLen, bufLen2;
bool bufDone, bufDone2;

Robot robot;
unsigned long lastMsgRecvTime;
unsigned long lastHeartbeatTime;

void setup() {
  Serial.begin(19200);
  Serial1.begin(19200);

  robot = Robot(ROBOT_ID, PIN_DIR_A, PIN_PWM_A, PIN_DIR_B, PIN_PWM_B, PIN_DIR_C, PIN_PWM_C);
  robot.setup();
  Serial.println("ROBOT SET UP");

  bufLen = 0;
  bufDone = false;

  lastHeartbeatTime = 0;
  lastMsgRecvTime = 0;

  Serial.println("Looping...");
  Serial1.println("MRSTART");
}

inline bool match(const char *haystack, const char *needle, const int len) {
  for (int i = 0; i < len; ++i) {
    if (haystack[i] != needle[i]) {
      return false;
    }
  }
  return true;
}

// buf has format +00000+00000
inline int extractInt(char *buf, const int idx) {
  const int startIdx = idx * BUF_VAL_WIDTH;
  const int endIdx = startIdx + BUF_VAL_WIDTH;

  byte val = buf[endIdx];
  buf[endIdx] = 0;
  const int ret = atoi(buf + startIdx);
  buf[endIdx] = val;

  return ret;
}

// Message format: MRCMD+00000+00000
void handleMessage(char *buf, const int len) {
  if (!match(buf, "MR", 2)) {
    // Not a maproom message
    return;
  }

  lastMsgRecvTime = millis();

  // Advance 2 char past MR for msg
  char *msg = buf + 2;

  // Vals are 3 after that, as the command names are 3char
  char *vals = msg + 3;

  if (match(msg, "MOV", 3)) {
    // MOVE COMMAND

    if (len != 23) {
      Serial.println("Bad length");
      return;
    }

    int dir = extractInt(vals, 0);
    int mag = extractInt(vals, 1);
    int measuredAngle = extractInt(vals, 2);

    robot.commandPosition(dir, mag, measuredAngle);
  } else if (match(msg, "DRW", 3)) {
    // DRAW COMMAND

    if (len != 23) {
      Serial.println("Bad length");
      return;
    }

    int dir = extractInt(vals, 0);
    int mag = extractInt(vals, 1);
    int measuredAngle = extractInt(vals, 2);

    robot.commandDraw(dir, mag, measuredAngle);
  } else if (match(msg, "ROT", 3)) {
    // ROTATE COMMAND

    if (len != 17) {
      Serial.println("Bad length");
      return;
    }

    int desiredAngle = extractInt(vals, 0);
    int measuredAngle = extractInt(vals, 1);

    robot.commandRotate(desiredAngle, measuredAngle);
  } else if (match(msg, "CAL", 3)) {
    // CALIBRATE

    if (len != 11) {
      Serial.println("Bad length");
      return;
    }

    int newRotation = extractInt(vals, 0);
    robot.commandCalibrate(newRotation);
  } else if (match(msg, "STP", 3)) {
    if (vals[0] == '1') {
      robot.setPen(PEN_DOWN);
    } else {
      robot.setPen(PEN_UP);
    }

    robot.commandStop();
  } else {
    Serial.print("Unknown message: ");
    Serial.println(buf);

    robot.stop();
  }
}

int wait = 0;

void loop() {
  char inChar;
  const unsigned long now = millis();

  while (Serial.available()) {
    inChar = (char)Serial.read();

    if (bufLen >= BUF_SIZE - 1) {
      Serial.println("BUF OVERRUN");
      bufLen = 0;
      continue;
    }

    if (!bufDone && inChar == '\n') {
      buf[bufLen+1] = 0;
      bufDone = true;
    } else if (!bufDone) {
      buf[bufLen++] = inChar;
    }

    if (bufDone) {
#if LOGGING
      Serial.print("buf: ");
      Serial.write(buf, bufLen);
      Serial.println();
#endif
      handleMessage(buf, bufLen);

      bufDone = false;
      bufLen = 0;
    }
  }

  while (Serial1.available()) {
    inChar = (char)Serial1.read();

    if (bufLen2 >= BUF_SIZE - 1) {
      Serial.println("BUF OVERRUN");
      bufLen2 = 0;
      continue;
    }

    if (!bufDone2 && inChar == '\n') {
      buf2[bufLen2 + 1] = 0;
      bufDone2 = true;
    } else if (!bufDone2) {
      buf2[bufLen2++] = inChar;
    }

    if (bufDone2) {
#if LOGGING
      Serial.print("buf2: ");
      Serial.write(buf2, bufLen2);
      Serial.println();
#endif
      handleMessage(buf2, bufLen2);

      bufDone2 = false;
      bufLen2 = 0;
    }
  }

#if HEARTBEAT
  if (now - lastHeartbeatTime >= HEARTBEAT_TIMEOUT_MILLIS) {
    lastHeartbeatTime = now;
    Serial1.println(HEARTBEAT_MSG);
  }
#endif

  if (robot.state == STATE_DRAWING && now - lastMsgRecvTime >= MSG_TIMEOUT_DRAWING_MILLIS) {
    robot.commandStop();
  } else if (now - lastMsgRecvTime >= MSG_TIMEOUT_MILLIS) {
    robot.commandStop();
  }

  robot.update();
}


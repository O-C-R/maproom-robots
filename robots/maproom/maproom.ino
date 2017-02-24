#include <Servo.h>

#include "Constants.h"
#include "Robot.h"

// Change both of these if the ID of the robot changes
#define ROBOT_ID 2
#define HEARTBEAT_MSG "SENRB02HB"

// Serial options
#define HEARTBEAT 1
#define HEARTBEAT_TIMEOUT_MILLIS 500

// Msg buffer
#define BUF_SIZE 125
#define BUF_VAL_WIDTH 6

char buf[BUF_SIZE];
int bufLen;
bool bufDone;

Robot robot;
unsigned long lastHeartbeatTime;

void setup() {
  robot = Robot(ROBOT_ID, PIN_DIR_A, PIN_PWM_A, PIN_DIR_B, PIN_PWM_B, PIN_DIR_C, PIN_PWM_C);
  robot.setup();

  bufLen = 0;
  bufDone = false;

  lastHeartbeatTime = 0;

  Serial.begin(19200);

  Serial.println("Looping...");
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

  // Advance 2 char past MR for msg
  char *msg = buf + 2;

  // Vals are 3 after that, as the command names are 3char
  char *vals = msg + 3;

  if (match(msg, "MOV", 3)) {
    // MOVE COMMAND

    int dir = extractInt(vals, 0);
    int mag = extractInt(vals, 1);
    int measuredAngle = extractInt(vals, 2);

    robot.commandPosition(dir, mag, measuredAngle);
  } else if (match(msg, "DRW", 3)) {
    // DRAW COMMAND

    int dir = extractInt(vals, 0);
    int mag = extractInt(vals, 1);
    int measuredAngle = extractInt(vals, 2);

    robot.commandDraw(dir, mag, measuredAngle);
  } else if (match(msg, "ROT", 3)) {
    // ROTATE COMMAND

    int desiredAngle = extractInt(vals, 0);
    int measuredAngle = extractInt(vals, 1);

#if LOGGING
    Serial.println("ROTATE MESSAGE");
    Serial.print("desiredAngle: ");
    Serial.print(desiredAngle);
    Serial.print("measuredAngle: ");
    Serial.print(measuredAngle);
#endif

    robot.commandRotate(desiredAngle, measuredAngle);
  } else if (match(msg, "XDR", 3)) {
    // DEBUG

    // int w0_mag = extractInt(vals, 0);
    // int w1_mag = extractInt(vals, 1);
    // int w2_mag = extractInt(vals, 2);

    // robot.commandDriveSpecific(w0_mag, w1_mag, w2_mag);
  } else if (match(msg, "CAL", 3)) {
    // CALIBRATE

    int newRotation = extractInt(vals, 0);
    robot.commandCalibrate(newRotation);
  } else if (match(msg, "STP", 3)) {
    if (vals[0] == '1') {
      robot.setPen(PEN_DOWN);
    } else {
      robot.setPen(PEN_UP);
    }

    robot.stop();
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
      buf[bufLen++] = 0;
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

#if HEARTBEAT
  if (now - lastHeartbeatTime >= HEARTBEAT_TIMEOUT_MILLIS) {
    lastHeartbeatTime = now;
    Serial.println(HEARTBEAT_MSG);
  }
#endif

  robot.update();
}


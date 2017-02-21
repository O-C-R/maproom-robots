#include <AltSoftSerial.h>

#define LOGGING 1

#include "Robot.h"

// Change both of these if the ID of the robot changes
#define ROBOT_ID 1
#define HEARTBEAT_MSG "SENRB01HB"

// Serial options
#define HEARTBEAT 1
#define HEARTBEAT_TIMEOUT_MILLIS 500

// Pins
#define pwmA 6
#define dirA 7

#define pwmB 5
#define dirB 4

#define pwmC 11
#define dirC 12

// Msg buffer
#define BUF_SIZE 125
#define BUF_VAL_WIDTH 6

AltSoftSerial wifiSerial;

char buf[BUF_SIZE], wifiBuf[BUF_SIZE];
int bufLen, wifiBufLen;
bool bufDone, wifiBufDone;

Robot robot;
unsigned long lastHeartbeatTime;

void setup() {
  robot = Robot(ROBOT_ID, dirA, pwmA, dirB, pwmB, dirC, pwmC);
  robot.stop();

  bufLen = 0;
  bufDone = false;
  wifiBufLen = 0;
  wifiBufDone = false;

  lastHeartbeatTime = 0;

  Serial.begin(57600);
  wifiSerial.begin(57600);

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
void handleMessage(char *buf) {
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

    robot.commandDraw(dir, mag);
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
    // STOP

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
      handleMessage(buf);

      bufDone = false;
      bufLen = 0;
    }
  }

  while (wifiSerial.available()) {
    inChar = wifiSerial.read();

    if (wifiBufLen >= BUF_SIZE - 1) {
      Serial.println("WIFI BUF OVERRUN");
      wifiBufLen = 0;
      continue;
    }

    if (!wifiBufDone && inChar == '\n') {
      wifiBuf[wifiBufLen++] = 0;
      wifiBufDone = true;
    } else if (!bufDone) {
      wifiBuf[wifiBufLen++] = inChar;
    }

    if (wifiBufDone) {
#if LOGGING
      Serial.print("wifiBuf: ");
      Serial.write(wifiBuf, wifiBufLen);
      Serial.println();
#endif
      handleMessage(wifiBuf);

      wifiBufDone = false;
      wifiBufLen = 0;
    }
  }

  const unsigned long now = millis();

#if HEARTBEAT
  if (now - lastHeartbeatTime >= HEARTBEAT_TIMEOUT_MILLIS) {
    lastHeartbeatTime = now;
    wifiSerial.println(HEARTBEAT_MSG);
  }
#endif

  robot.update();
}


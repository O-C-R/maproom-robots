#include <Servo.h>
#include <MiniPID.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#include "Constants.h"
#include "Robot.h"

// Change both of these if the ID of the robot changes
#define ROBOT_ID 1

// Serial options
#define HEARTBEAT_TIMEOUT_MILLIS 500
#define MSG_TIMEOUT_DRAWING_MILLIS 500
#define MSG_TIMEOUT_MILLIS 2000
#define SERIAL_BUF_SIZE 125
#define BUF_VAL_WIDTH 6

// Wifi options
#define MAX_PACKET_LENGTH 255
#define WIFI_WAIT_TIME    1000

const char *ssid     = "StLouisMaproom";
const char *password = "CreativeResearch";
const unsigned int localUdpPort = 5111;

IPAddress remIP(192, 168, 7, 20);
const int remPort = 5101;

WiFiUDP udp;
char incomingPacket[MAX_PACKET_LENGTH];
char outgoingPacket[MAX_PACKET_LENGTH];

char inChar;
char serialBuf[SERIAL_BUF_SIZE];
int serialBufLen;
bool serialBufDone;

unsigned long lastMsgRecvTime;
unsigned long lastHeartbeatTime;

Robot robot(ROBOT_ID, PIN_DIR_A, PIN_PWM_A, PIN_DIR_B, PIN_PWM_B, PIN_DIR_C, PIN_PWM_C);

void setup() {
  Serial.begin(19200);
  Serial.println("MRBOOT");

  robot.setup();

  serialBufLen = 0;
  serialBufDone = false;
  lastHeartbeatTime = 0;
  lastMsgRecvTime = 0;

  WiFi.begin(ssid, password);
  Serial.println("MRSTART");
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

  if(match(msg, "AT", 2)) {
    if (WiFi.isConnected()) {
      Serial.println("ESCONN");
    } else {
      Serial.println("ESNC");
    }
    Serial.print("ESIP");
    Serial.println(WiFi.localIP());
  } else if (match(msg, "MOV", 3)) {
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

void ensureWifi() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("ESDISCONN");
    robot.commandStop();

    WiFi.reconnect();
    while (WiFi.status() != WL_CONNECTED)
    {
      delay(WIFI_WAIT_TIME);
      Serial.println("ESWAITCONN");
    }

    Serial.println("ESCONNECT");

    udp.begin(localUdpPort);
    Serial.printf("ESUDP:%s:%d\n", WiFi.localIP().toString().c_str(), localUdpPort);
  }
}

void handleSerial() {
  char inChar;

  while (Serial.available()) {
    inChar = (char)Serial.read();

    if (serialBufLen >= SERIAL_BUF_SIZE - 1) {
      Serial.println("Serial0 BUF OVERRUN");
      serialBufLen = 0;
      continue;
    }

    if (!serialBufDone && inChar == '\n') {
      serialBuf[serialBufLen+1] = '\0';
      serialBufDone = true;
    } else if (!serialBufDone) {
      serialBuf[serialBufLen++] = inChar;
    }

    if (serialBufDone) {
      handleMessage(serialBuf, serialBufLen);

      serialBufDone = false;
      serialBufLen = 0;
    }
  }
}

void handleUdp() {
  int packetSize = udp.parsePacket();
  if (packetSize) {
    int len = udp.read(incomingPacket, MAX_PACKET_LENGTH);
    if (len > 0) {
      incomingPacket[len] = '\0';
    }
    handleMessage(incomingPacket, len-1);

    // Update remote IP to whoever just pinged us
    remIP = udp.remoteIP();
  }
}

void sendHeartbeat() {
  udp.beginPacket(remIP, remPort);
  sprintf(outgoingPacket, "RB%02dHB\n", ROBOT_ID);
  udp.write(outgoingPacket);
  udp.endPacket();
}

void loop() {
  ensureWifi();
  handleSerial();
  handleUdp();

  const unsigned long now = millis();

  if (now - lastHeartbeatTime >= HEARTBEAT_TIMEOUT_MILLIS) {
    lastHeartbeatTime = now;
    sendHeartbeat();
  }

  if (robot.state == STATE_DRAWING && now - lastMsgRecvTime >= MSG_TIMEOUT_DRAWING_MILLIS) {
    robot.commandStop();
  } else if (now - lastMsgRecvTime >= MSG_TIMEOUT_MILLIS) {
    robot.commandStop();
  }

  robot.update();
}


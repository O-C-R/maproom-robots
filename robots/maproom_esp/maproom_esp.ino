/*
 *  This sketch shows the WiFi event usage
 *
 */

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#define MAX_PACKET_LENGTH 1024
#define HEARTBEAT_TIMEOUT 3000
#define WIFI_WAIT_TIME    1000

const char *ssid     = "StLouisMaproom";
const char *password = "CreativeResearch";
const unsigned int localUdpPort = 5111;

IPAddress remIP(192, 168, 7, 20);
const int remPort = 5101;

char buf[255];
int bufLen = 0;
bool bufDone = false;

WiFiUDP udp;
char incomingPacket[MAX_PACKET_LENGTH];

long lastHeartbeatTime = 0;

void setup() {
    Serial.begin(19200);
    Serial.println("ESBOOT");

    WiFi.begin(ssid, password);
    Serial.println("ESSTART");
}

void loop() {
  long now = millis();

  if (now - lastHeartbeatTime > HEARTBEAT_TIMEOUT) {
    Serial.println("ESHB");
    lastHeartbeatTime = now;
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("ESDISCONN");

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

  while (Serial.available()) {
    char inChar = (char)Serial.read();
    if (!bufDone && (inChar == '\n' || inChar == '\r')) {
      buf[bufLen+1] = 0;
      bufDone = true;
    } else {
      buf[bufLen++] = inChar;
    }
  }

  if (bufDone) {
    if (buf[0] == 'A' && buf[1] == 'T') {
      if (WiFi.isConnected()) {
        Serial.println("ESOK");
      } else {
        Serial.println("ESNC");
      }
    } else if (buf[0] == 'I' && buf[1] == 'P') {
      Serial.print("ESIP");
      Serial.println(WiFi.localIP());
    } else if (buf[0] == 'R' && buf[1] == 'E' && buf[2] == 'S') {
      Serial.println("ESRESTART");
      ESP.restart();
    } else if (buf[0] == 'S' && buf[1] == 'E' && buf[2] == 'N') {
      udp.beginPacket(remIP, remPort);
      udp.write(buf + 3);
      udp.write('\n');
      udp.endPacket();

      Serial.print("ESSENT");
      Serial.print(remIP);
      Serial.print(":");
      Serial.println(remPort);
    }

    bufLen = 0;
    bufDone = false;
  }

  int packetSize = udp.parsePacket();
  if (packetSize) {
    int len = udp.read(incomingPacket, MAX_PACKET_LENGTH);
    if (len > 0) {
      incomingPacket[len] = 0;
    }
    Serial.write(incomingPacket);

    // Update remote IP to whoever just pinged us
    remIP = udp.remoteIP();
  }
}


/*
 *  This sketch shows the WiFi event usage
 *
 */

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

const char* ssid     = "StLouisMaproom";
const char* password = "CreativeResearch";
const unsigned int localUdpPort = 5111;

char remIP[] = "192.168.0.20";
int remPort = 5100;

char *buf;
int bufLen = 0;
bool bufDone = false;

WiFiUDP udp;
char incomingPacket[255];

long lastTime;

void WiFiEvent(WiFiEvent_t event) {
    bool success;
    switch(event) {
        case WIFI_EVENT_STAMODE_GOT_IP:
            Serial.println("ESCONNECT");
            success = udp.begin(localUdpPort);
            if (!success) {
              Serial.println("ESUDP:FAIL");
            } else {
              Serial.printf("ESUDP:%s:%d\n", WiFi.localIP().toString().c_str(), localUdpPort);
            }
            break;
        case WIFI_EVENT_STAMODE_DISCONNECTED:
            Serial.println("ESDISCONN");
            break;
    }
}

void setup() {
    Serial.begin(57600);
    delay(10);

    buf = (char *)malloc(1024 * sizeof(char));

    delay(10);
    Serial.println("ESSTART");

    WiFi.setAutoReconnect(true);
    WiFi.setAutoConnect(true);
    WiFi.onEvent(WiFiEvent);
    WiFi.begin(ssid, password);

    lastTime = millis();
}


void loop() {
  long now = millis();
  
  if (now - lastTime > 3000) {
    Serial.println("ESHB");
    lastTime = now;
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
    }

    bufLen = 0;
    bufDone = false; 
  }
  
  int packetSize = udp.parsePacket();
  if (packetSize)
  {
    int len = udp.read(incomingPacket, 255);
    if (len > 0)
    {
      incomingPacket[len] = 0;
    }
    Serial.write(incomingPacket);
  }
}


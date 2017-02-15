#include <SoftwareSerial.h>
SoftwareSerial wifiSerial(8, 9);

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

char *buf, *wifiBuf;
int bufLen, wifiLen;
bool bufDone, wifiDone;

#define BUF_SIZE 1024

String wifiStr = "";

void setup() {
  pinMode(dirA, OUTPUT);
  pinMode(dirB, OUTPUT);
  pinMode(dirC, OUTPUT); 

  allStop();
//  motorA.direction = true;
//  motorB.direction = true;
//  motorC.direction = false;
//  motorA.pulse = 100;
//  motorB.pulse = 100;
//  motorC.pulse = 4000;
//  commandMotors();
  
  buf = (char *)malloc(BUF_SIZE * sizeof(char));
  wifiBuf = (char *)malloc(BUF_SIZE * sizeof(char));

  bufLen = 0;
  wifiLen = 0;
  bufDone = false;
  wifiDone = false;

  Serial.begin(57600);
//  wifiSerial.begin(57600);
  
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
    
    motorGo(param1 - 500, param2 - 500);
  } else if (buf[4] == 'R' && buf[5] == 'O' && buf[6] == 'T') {
    byte val = buf[11];
    buf[11] = 0;
    int param1 = atoi(buf + 7);
    buf[11] = val;
    
    motorRotate(param1 - 500);
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
    
    motorGoSpecific(param1 - 500, param2 - 500, param3 - 500);
  } else {
    Serial.print("Unknown message: ");
    Serial.println(buf);
    allStop();
  }
}

void loop() {
  while (Serial.available()) {
    char inChar = (char)Serial.read();
//    Serial.print("serial: ");
//    Serial.print(inChar, HEX);
//    Serial.println();

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
    if (buf[0] == 'A' && buf[1] == 'T') {
      wifiSerial.println("AT");
      Serial.println("Sent AT!");
    } else if (buf[0] == 'I' && buf[1] == 'P') {
      wifiSerial.println("IP");
      Serial.println("Sent IP!");
    }

    Serial.print("buf: ");
    Serial.write(buf, bufLen);
    Serial.println();
    handleMessage(buf);

    bufDone = false;
    bufLen = 0;
  }

  while (wifiSerial.available()) {
    char inChar = (char)wifiSerial.read();

    if (wifiLen >= BUF_SIZE - 1) {
//      Serial.println("WIFI BUF OVERRUN");
      continue;
    } else {
//      Serial.print(inChar, HEX);
    }

    wifiStr += inChar;

    if (!wifiDone && inChar == '\n') {
      wifiBuf[wifiLen++] = 0;
      wifiDone = true;
    } else {
      wifiBuf[wifiLen++] = inChar;
    }

    if (wifiDone) {
      Serial.println("wifidone");
      for (int i = 0; i < wifiLen; ++i) {
        Serial.print(wifiBuf[i], HEX);
      }
      Serial.println();
      Serial.write(wifiBuf, wifiLen);
      Serial.println();
      Serial.println(wifiStr);
      Serial.println("^wifidone");

      wifiStr = "";
      wifiLen = 0;
      wifiDone = false;
    }
  }


//  Serial.write("wifil: ");
//  Serial.write(wifiBuf, wifiLen);
//  Serial.write(" ");
//  Serial.println(wifiLen);
//  Serial.println("-----");
//  Serial.write("regl: ");
//  Serial.write(buf, bufLen);
//  Serial.write(" ");
//  Serial.println(bufLen);
//  Serial.println("-----");
//  easeMotors();
//  commandMotors();
  //delay(5);
}

//void serialEvent() {
//  while (Serial.available()) {
//    char inChar = (char)Serial.read();
//    if (!bufDone && (inChar == '\n' || inChar == '\r')) {
//      buf[bufLen++] = 0;
//      bufDone = true;
//    } else {
//      buf[bufLen++] = inChar;
//    }
//  }
//}



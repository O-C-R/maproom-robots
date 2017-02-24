#include "Constants.h"

class Pen {

public:
  int state;
  unsigned long penChangeTime;

  Pen(): state(PEN_DOWN), penChangeTime(0) {}

  void setState(const int newState) {
    if (newState == PEN_UP) {
      analogWrite(PIN_PEN, PEN_UP_VAL);
    } else if (newState == PEN_DOWN) {
      analogWrite(PIN_PEN, PEN_DOWN_VAL);
    } else {
      Serial.println("INVALID PEN STATE");
    }

    state = newState;
    penChangeTime = millis();
  }

  bool doneChanging(const unsigned long now) {
    return now - penChangeTime > PEN_MOVING_TIME;
  }

};

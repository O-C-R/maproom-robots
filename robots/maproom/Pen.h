class Pen {

public:
  Servo servo;

  int state;
  unsigned long penChangeTime;

  Pen(): state(PEN_DOWN), penChangeTime(0) {}

  void setup() {
    servo.attach(PIN_PEN);
  }

  void setState(const int newState) {
    if (newState == PEN_UP) {
      servo.write(PEN_UP_VAL);
    } else if (newState == PEN_DOWN) {
      servo.write(PEN_DOWN_VAL);
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

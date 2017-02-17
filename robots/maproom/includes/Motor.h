class Motor
{
  int pulse; // pwm signal mapped 0 - 255
  bool direction; // CW = positive, CCW = negative

  bool logging;

  int pwm; // pwm pin
  int dir; // dir pin

  float angle; // orientation of the wheel

  public:
  Motor(int dir_pin, int pwm_pin, float wheel_angle, bool logging)
  {
    logging = logging;
    pulse = 0;
    pwm = pwm_pin;
    dir = dir_pin;
    angle = wheel_angle;
    pinMode(dir, OUTPUT);
  }

  void commandMotor() {
    if (logging) {
      Serial.print(direction);
      Serial.print(" ");
      Serial.println(pulse);
    }
    digitalWrite(dir, direction);
    analogWrite(pwm, pulse);
  }

  void stop() {
    pulse = 0;
  }

  // vector sum of each wheel
  void driveVector(int theta, int mag) {
    static const float w_angle = angle / 180 * 3.14159;
    float w = mag * cos(w_angle - theta);
    if (logging) {
      Serial.println(w);
    }

    // ccw = negative values, cw = positive
    direction = w < 0 ? true : false;
    pulse = map(abs(w), 0, 600, 0, 255);
  }

  void driveConstant(int dir, int mag) {
    direction = dir < 0 ? true : false;
    pulse = map(abs(mag), 0, 600, 0, 255);
  }

};

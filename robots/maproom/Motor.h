class Motor
{
  bool logging;
  int pulse; // pwm signal mapped 0 - 255
  bool direction; // CW = positive, CCW = negative

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
  void driveVector(float theta, float mag) {
    float w_angle = angle / 180 * 3.14159;
    float w = mag * cos(w_angle - theta);

    // ccw = negative values, cw = positive
    direction = w < 0 ? true : false;
    pulse = map(abs(w), 0, 600, 0, 255);
  }

  // runs each motor at constant rate
  void driveConstant(float speed) {
    direction = speed > 0 ? true : false;
    pulse = map(abs(speed), 0, 600, 0, 255);
  }
};

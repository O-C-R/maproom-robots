class Motor
{
  int pulse; // pwm signal mapped 0 - 255
  bool direction; // CW = positive, CCW = negative

  int pwm; // pwm pin
  int dir; // dir pin

  float angle; // orientation of the wheel

  public:
  Motor(int dir_pin, int pwm_pin, float wheel_angle)
  {
    pulse = 0;
    pwm = pwm_pin;
    dir = dir_pin;
    angle = wheel_angle;
    pinMode(dir, OUTPUT);
  }

  void commandMotor(bool status) {
    if (status) {
      Serial.print(direction);
      Serial.print(" ");
      Serial.println(pulse);
    }
    digitalWrite(dir, direction);
    analogWrite(pwm, pulse);
  }

  void stop() {
    target = 0;
    current = 0;
    pulse = 0;
    analogWrite(pwm, 0);
  }

  void go(int y, int x, theta, mag, bool status) {

    static const float w_angle = angle / 180 * 3.14159;
    float w = mag * cos(w_angle - theta);
    if (status) {
      Serial.println(w);
    }

    // ccw = negative values, cw = positive
    direction = w < 0 ? true : false;
    pulse = map(abs(w), 0, 600, 0, 255);

    commandMotor(status);
  }
};

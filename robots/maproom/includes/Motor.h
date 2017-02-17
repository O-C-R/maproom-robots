class Motor
{
  int pulse;
  bool direction;

  float current;
  float target;

  int pwm;
  int dir;

  float angle;
  public:
  Motor(int dir_pin, int pwm_pin, float wheel_angle)
  {
    target = 0;
    current = 0;
    pulse = 0;

    pwm = pwm_pin;
    dir = dir_pin;
    angle = wheel_angle;
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

  void go(int y, int x, bool status) {
    float theta = atan2(y, x);
    float magnitude = sqrt((x*x)+(y*y));

    static const float w_angle = angle;

    float w = magnitude * cos(w_angle - theta);

    // todo: add logging

    boolean w_ccw = w < 0 ? true : false;

    int w_speed = map(abs(w), 0, 600, 0, 255);

    direction = w_ccw;

    pulse = w_speed;

    commandMotor(status);
  }
};

static const float kHeadingCorrectionAmt = 20.0;

class Motor
{
  bool logging;
  int pulse; // pwm signal mapped 0 - 255
  bool direction; // CW = positive, CCW = negative

  int pwm; // pwm pin
  int dir; // dir pin

  float angle; // orientation of the wheel, degrees
  float angleRad; // in radians

  float xContrib, yContrib;

  public:

  Motor() {}
  Motor(int dir_pin, int pwm_pin, float wheel_angle, bool setLogging):
    logging(setLogging),
    pulse(0),
    pwm(pwm_pin),
    dir(dir_pin),
    angle(wheel_angle)
  {
    pinMode(dir, OUTPUT);

    angleRad = angle / 180.0 * 3.14159;

    xContrib = cos(angleRad);
    yContrib = sin(angleRad);
  }

  void commandMotor() {
    digitalWrite(dir, direction);
    analogWrite(pwm, pulse);
  }

  void stop() {
    pulse = 0;
  }

  // vector sum of each wheel
  void driveVector(const float worldAngle, const float mag, const float headingCorrection) {
    const float worldAngleRH = 360.0 - worldAngle;
    const float worldAngleRHRad = worldAngleRH / 180.0 * 3.14159;

    // Rotate 90 deg
    const float xAmt = sin(worldAngleRHRad);
    const float yAmt = cos(worldAngleRHRad);

    // Go!
    float move = mag * (xAmt * xContrib + yAmt * yContrib);

    // Correct our heading
    const float rotate = kHeadingCorrectionAmt * headingCorrection;

    // Total
    const float w = move + rotate;

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

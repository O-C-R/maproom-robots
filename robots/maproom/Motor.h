class Motor
{

public:

  int pulse; // pwm signal mapped 0 - 255
  bool direction; // CW = positive, CCW = negative

  int pwm; // pwm pin
  int dir; // dir pin

  // Angle of the wheel from the center
  float wheelAngle, wheelAngleRad;

  // Angle that the wheel applies force at
  float wheelForce, wheelForceRad;

  // x and y components of the above force angle
  float xContrib, yContrib;

  Motor() {}
  Motor(int dirPin, int pwmPin, float inWheelAngle):
    pulse(0),
    dir(dirPin),
    pwm(pwmPin),
    wheelAngle(inWheelAngle),
    wheelForce(inWheelAngle + 90)
  {
    pinMode(dir, OUTPUT);

    wheelAngleRad = wheelAngle / 180.0 * 3.14159;
    wheelForceRad = wheelForce / 180.0 * 3.14159;

    xContrib = cos(wheelForceRad);
    yContrib = sin(wheelForceRad);
  }

  void commandMotor() {
    digitalWrite(dir, direction);
    analogWrite(pwm, pulse);
  }

  void stop() {
    pulse = 0;
  }

  void driveVector(const float worldAngle, const float mag, const float headingCorrection) {
    // It's not clear to me why this is still wrong. But it seems to work.
    const float worldAngleRH = 360.0 - worldAngle;
    const float worldAngleRHRad = worldAngleRH / 180.0 * 3.14159;

    const float xAmt = cos(worldAngleRHRad);
    const float yAmt = sin(worldAngleRHRad);

    // Go!
    const float move = mag * (xAmt * xContrib + yAmt * yContrib);

    // Total
    const float speed = move + headingCorrection;

    direction = speed > 0 ? MOTORS_DIR : !MOTORS_DIR;
    pulse = map(abs(speed), 0, 600, 22, 255);
  }

  // runs each motor at constant rate
  void driveConstant(float speed) {
    direction = speed > 0 ? MOTORS_DIR : !MOTORS_DIR;
    pulse = map(abs(speed), 0, 600, 22, 255);
  }
};

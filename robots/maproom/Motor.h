static const float kHeadingCorrectionAmt = 20.0;

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
    wheelForce(inWheelAngle - 90)
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

  // vector sum of each wheel
  void driveVector(const float worldAngle, const float mag, const float headingCorrection) {
    const float worldAngleRH = 360.0 - worldAngle;
    const float worldAngleRHRad = worldAngleRH / 180.0 * 3.14159;

    const float xAmt = cos(worldAngleRHRad);
    const float yAmt = sin(worldAngleRHRad);

    // Go!
    float move = mag * (xAmt * xContrib + yAmt * yContrib);

    // Correct our heading
    const float rotate = kHeadingCorrectionAmt * headingCorrection;

    // Total
    const float w = move + rotate;

    direction = w > 0 ? true : false;
    pulse = map(abs(w), 0, 600, 0, 255);
  }

  // runs each motor at constant rate
  void driveConstant(float speed) {
    direction = speed > 0 ? true : false;
    pulse = map(abs(speed), 0, 600, 0, 255);
  }
};

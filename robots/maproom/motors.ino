void allStop() {
  motorA.target = 0;
  motorA.current = 0;
  motorA.pulse = 0; 
  
  motorB.target = 0;
  motorB.current = 0;
  motorB.pulse = 0;
  
  motorC.target = 0;
  motorC.current = 0;
  motorC.pulse = 0;
  
  analogWrite(pwmA, 0);
  analogWrite(pwmB, 0);
  analogWrite(pwmC, 0);
}

static const float kEase = 0.001;
void easeMotors() {
  motorA.current += (motorA.target - motorA.current) * kEase;
  motorB.current += (motorB.target - motorB.current) * kEase;
  motorC.current += (motorC.target - motorC.current) * kEase;

  boolean mA_ccw = motorA.current < 0 ? true : false;
  boolean mB_ccw = motorB.current < 0 ? true : false;
  boolean mC_ccw = motorC.current < 0 ? true : false;
  int mA_speed = abs(motorA.current);
  int mB_speed = abs(motorB.current);
  int mC_speed = abs(motorC.current);

  motorA.direction = mA_ccw;
  motorB.direction = mB_ccw;
  motorC.direction = mC_ccw;
  motorA.pulse = mA_speed;
  motorB.pulse = mB_speed;
  motorC.pulse = mC_speed;
}

void commandMotors() {
  Serial.println("output:");
  Serial.print(motorA.direction);
  Serial.print(" ");
  Serial.println(motorA.pulse);
  Serial.print(motorB.direction);
  Serial.print(" ");
  Serial.println(motorB.pulse);
  Serial.print(motorC.direction);
  Serial.print(" ");
  Serial.println(motorC.pulse);
  Serial.println("-----");
  
  digitalWrite(dirA, motorA.direction);
  digitalWrite(dirB, motorB.direction);
  digitalWrite(dirC, motorC.direction);
  analogWrite(pwmA, motorA.pulse);
  analogWrite(pwmB, motorB.pulse);  
  analogWrite(pwmC, motorC.pulse);
}

const float sqrt3o2 = 1.0*sqrt(3)/2;

void motorRotate(int angle) {
  if (angle > 0) {
    motorA.direction = motorB.direction = motorC.direction = true;
    motorA.pulse = motorB.pulse = motorC.pulse = angle;
  } else if (angle < 0) {
    motorA.direction = motorB.direction = motorC.direction = false;
    motorA.pulse = motorB.pulse = motorC.pulse = -angle;
  } else {
    allStop();
  }

  commandMotors();
}

void motorGoSpecific(int w0, int w1, int w2) {
  boolean w0_ccw = w0 < 0 ? true : false;
  boolean w1_ccw = w1 < 0 ? true : false;
  boolean w2_ccw = w2 < 0 ? true : false;

  int w0_speed = abs(w0);
  int w1_speed = abs(w1);
  int w2_speed = abs(w2);
  
  motorA.direction = w0_ccw;
  motorB.direction = w1_ccw;
  motorC.direction = w2_ccw;
  
  motorA.pulse = w0_speed;
  motorB.pulse = w1_speed;
  motorC.pulse = w2_speed;

  commandMotors();
}

void motorGo(long x, long y) {
  float theta = atan2(y, x);
  float magnitude = sqrt((x*x)+(y*y));

  static const float w0_angle = 0;
  static const float w1_angle = 120.0 / 180.0 * 3.14159;
  static const float w2_angle = 240 / 180.0 * 3.14159;

  float w0 = magnitude * cos(w0_angle - theta);
  float w1 = magnitude * cos(w1_angle - theta);
  float w2 = magnitude * cos(w2_angle - theta);

  Serial.println("dir:");
  Serial.println(x);
  Serial.println(y);
  Serial.println(theta);
  Serial.println(magnitude);
  Serial.println("wheels:");
  Serial.println(w0);
  Serial.println(w1);
  Serial.println(w2);
  Serial.println("-----");

  boolean w0_ccw = w0 < 0 ? true : false;
  boolean w1_ccw = w1 < 0 ? true : false;
  boolean w2_ccw = w2 < 0 ? true : false;
  int w0_speed = map(abs(w0), 0, 600, 0, 255);
  int w1_speed = map(abs(w1), 0, 600, 0, 255);
  int w2_speed = map(abs(w2), 0, 600, 0, 255);
  
  motorA.direction = w0_ccw;
  motorB.direction = w1_ccw;
  motorC.direction = w2_ccw;
  
  motorA.pulse = w0_speed;
  motorB.pulse = w1_speed;
  motorC.pulse = w2_speed;

  commandMotors();
}

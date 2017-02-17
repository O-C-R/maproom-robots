#include <Servo.h>

Servo myservo;  // create servo object to control a servo

int switch_pin = 10; 
int servo_pin = 9;
 
int up = 200; 
int down = 100; 

int marker_down;

void setup() {
  Serial.begin(57600);
  Serial.println("Starting...");
  myservo.attach(servo_pin); 
  pinMode(switch_pin, INPUT); 
}


void loop() {  
  marker_down = digitalRead(switch_pin);
  myservo.write(marker_down ? down : up);
  delay(50);                           
}

#include <Servo.h> 

const int digi = 3;

Servo myservo; 

void setup() {
  myservo.attach(digi); 
  Serial.begin(9600); 
}

void loop() {
  Serial.println("shit"); 
//  myservo.write(90); 
  delay(2000); 
  myservo.write(70); 
  delay(2000); 
}

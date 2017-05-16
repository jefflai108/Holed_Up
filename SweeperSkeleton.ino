/**
   Final Project Sweeper
   Nick, Hao, Andrew, Jeff
   4/22/2017
*/

//Libraries
#include <Servo.h>
#include <SPI.h>
#include <RF24.h>
#include <Encoder.h>

//Objects
Servo flipper;
Encoder encoder(2, 3);

//Definitions
#define STOP              0
#define CLOCKWISE         1
#define COUNTER_CLOCKWISE 2
#define FLIP              3

//Pins
int flipPin = 7;
int rotatorPin = 4;

//Variables
unsigned long received;
int val;
int normalRotSpeed = 200;
int limitSwitch = 8;
long start;
long current;
long swipeDistance = 1000;

/****************** Radio Config ***************************/
/***      0 for sender    1 for receiver    ***/
//bool radioNumber = 1;
//RF24 radio(7,8); /*change to pins 7 & 8 */
//byte addresses[][6] = {"1Node","2Node"};
/**********************************************************/


void setup() {
  Serial.begin(9600);
  Serial.print("Setting up");

  pinMode(limitSwitch, INPUT);
  pinMode(rotatorPin, OUTPUT);
  pinMode(flipPin, OUTPUT);

  //  analogWrite(rotatorPin,123);

  //  digitalWrite(rotatorPin, HIGH);
  digitalWrite(rotatorPin, LOW);

  flipper.attach(flipPin);
  go(STOP);
  flipper.write(170); //initially flat

  delay(1000);
}

void loop() {

  Serial.println("Begin");

  //    while (1) {
  //        Serial.println(encoder.read());
  //    }

  //  counter = 0;
  //  current = encoder.read();
  //  while (current >= -1000) {
  //    Serial.println(current);
  //    Serial.println("moving");
  //    go(COUNTER_CLOCKWISE);
  //    previous = current;
  //    current = encoder.read();
  //    if (previous == current) {
  //      counter++;
  //    } else {
  //      counter = 0;
  //    }
  //    if (counter == 10) {
  //      counter = 0;
  //
  //    }
  //    for (int i = 0; i < 3; i++) {
  //      val = digitalRead(limitSwitch);
  //      Serial.println(val);
  //      if (val == 0) {
  //        continue;
  //      } else if (i == 2 && val == 1) {
  //          flipper.write(180);
  //      }
  //    }
  //  }

  while (encoder.read() >= -1000) {
    Serial.println(encoder.read());
    Serial.println("moving");
    go(COUNTER_CLOCKWISE);
    for (int i = 0; i < 3; i++) {
      val = digitalRead(limitSwitch);
      Serial.println(val);
      if (val == 0) {
        continue;
      } else if (i == 2 && val == 1) {
        flipper.write(180);
      }
    }
  }
  go(STOP);
  delay(2000);
  go(FLIP);
  Serial.println("shit");
  delay(100000000);
}

void endLoop () {

  while (true) {
    Serial.println("ended");
    delay(10000);
  }
}

void waitForAmbulance() { //loops here until ambulance RF received



}

void waitForEncoder() {
  //  start = encoder.read();
  //  while (true) {
  //    Serial.println(encoder.read());
  //    current = encoder.read();
  //    if(current - start >= swipeDistance) {
  //      go(STOP);
  //      break;
  //    }
  //
  //  }

}

void dropVictim() {
  go(FLIP);
  delay(200);
}

void initRadio() { //initializes the radio


}

void go(int dir) { //movement states
  switch (dir) {
    case 0:  //stop
      digitalWrite(rotatorPin, LOW);
      break;
    case 1: //rotate clockwise
      digitalWrite(rotatorPin, HIGH);
      break;
    case 2: //rotate counterclockwise
      digitalWrite(rotatorPin, HIGH);
      break;
    case 3: //turn flipper
      flipper.write(70);
      break;
  }

}


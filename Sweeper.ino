/**
 * Final Project Sweeper
 * Nick, Hao, Andrew, Jeff
 * 4/22/2017
 */

//Libraries
#include <Servo.h>
#include <SPI.h>
#include <RF24.h>
#include <Encoder.h>

//Objects
Servo flipper;
Encoder encoder(5, 6);

//Definitions
#define STOP              0
#define CLOCKWISE         1
#define COUNTER_CLOCKWISE 2
#define FLIP              3

//Pins
int rotatorPinEnable = 7;
int flipPin = 8;
int rot1=9;
int rot2=10;

//Variables
int received;
int normalRotSpeed = 120;
long start;
long current;
long swipeDistance;

/****************** Radio Config ***************************/
/***      0 for sender    1 for receiver    ***/
bool radioNumber = 1;
RF24 radio(1,2); /*change to pins 7 & 8 */
byte addresses[][6] = {"1Node","2Node"};
/**********************************************************/


void setup() {
  Serial.begin(9600);
  Serial.print("Setting up");

  pinMode(rotatorPinEnable,OUTPUT);
  pinMode(flipPin,OUTPUT);
  pinMode(rot1,OUTPUT);
  pinMode(rot2,OUTPUT);
  
  analogWrite(rotatorPinEnable,normalRotSpeed);
  flipper.attach(flipPin);
  go(STOP);
  flipper.write(180); //initially flat
  
  //radio
  radio.begin();
  initRadio();
  received = 0;

}

void loop() {
        
    Serial.println("Began loop");
    
    waitForAmbulance();  //waits for RF transmission, notifies ambulance upon receipt of transmission

    analogWrite(rotatorPinEnable,normalRotSpeed); //start moving
    go(COUNTER_CLOCKWISE);

    waitForEncoder(); //wait until moved a specific distance

    dropVictim(); //drops the victim
    
//    go(CLOCKWISE); //in case victim won't drop
//    delay(1000);

    sendDropped(); //sends RF transmission
    
    endLoop(); //does nothing, forever
      
}

void endLoop () {

  while(true) {
      Serial.println("ended");
      delay(10000);
    }
}

void waitForAmbulance() { //loops here until ambulance RF received

  while(true){
   //RF receive integer
     if( radio.available()){
        while (radio.available()) {              // While there is data ready
           radio.read( &received, sizeof(int) );  // Get the payload
           Serial.println(received); 
        }
     }
     if(received == 13) {
      Serial.println("Received transmission");
      radio.stopListening();
      sendReceived();
      break;
     }
  }   
}

void waitForEncoder() {
  start = encoder.read();
  while (true) {
    current = encoder.read();
    if(current - start >= swipeDistance) {
      go(STOP);
      break;
    }
    
  }
  
}

void dropVictim() {
   go(FLIP);
   delay(200);
}

void sendReceived() { //Notifies ambulance that transmission was received
  while(true) {
      if (!radio.write(101, sizeof(int) )){
       Serial.println("Failed transmission");
      }   
      else {
        Serial.println("Sent first transmission");
        break;
      }
  }
}


void sendDropped() { //Notifies ambulance that vitcim was dropped
  while(true) {
      if (!radio.write(237, sizeof(int) )){
       Serial.println("Failed transmission");
      }   
      else {
        Serial.println("Sent second transmission");
        break;
      }
  }
}

void initRadio() { //initializes the radio
  
  radio.setPALevel(RF24_PA_LOW);
  radio.setChannel(80);

  // Open a writing and reading pipe
  if(radioNumber){
    radio.openWritingPipe(addresses[1]);
    radio.openReadingPipe(1,addresses[0]);
  }else{
    radio.openWritingPipe(addresses[0]);
    radio.openReadingPipe(1,addresses[1]);
  }
  // Make sure the radio starts listening for data
  radio.startListening();
}

void go(int dir) { //movement states
  switch(dir) {
    case 0:  //stop
      analogWrite(rotatorPinEnable,0);
      digitalWrite(rot1,LOW);
      digitalWrite(rot2,LOW);
      break;
    case 1: //rotate clockwise
      analogWrite(rotatorPinEnable,normalRotSpeed);
      digitalWrite(rot1,LOW);
      digitalWrite(rot2,HIGH);
      break;
    case 2: //rotate counterclockwise
      analogWrite(rotatorPinEnable,normalRotSpeed);
      digitalWrite(rot1,HIGH);
      digitalWrite(rot2,LOW);
      break;
    case 3: //turn flipper
      flipper.write(90);
      break;
  }

}


/**
   Final Project Crane
   Nick, Hao, Andrew, Jeff
   4/22/2017
*/

//Libraries
#include <Servo.h>
#include <LSM303.h>
#include <SPI.h>
#include <Pixy.h>
#include <RF24.h>
#include <Encoder.h>

//Motors

//rotator---------5 PWM hilo
int rot1 = 9;
int rot2 = 10;

//lateral----------6 PWM hilo
int lat1 = 4;
int lat2 = 13;

//claw drop----------7 PWM hilo
int claw1 = 11;
int claw2 = 12;

Servo grabber;

#define STOP              0
#define CLOCKWISE         1
#define COUNTER_CLOCKWISE 2
#define LATERAL_FORWARD   3
#define LATERAL_BACK      4
#define CLAW_DOWN         5
#define CLAW_UP           6
#define CLAW_OPEN         8
#define CLAW_CLOSE        7
#define coTime 2500 //delay for claw opening
#define ccTime 2500 //delay for claw closing
#define AMBULANCE_DIST 3000 //time to drop claw

//Pins
int rotatorPinEnable = 5;
int lateralPinEnable = 3;
int clawPinEnable = 6;
int grabberPin = 8;

//Pixy
Pixy pixy;
int i; //for framerate
int firstCheck; //for error checking
uint16_t blocks;
char buf[32];

//Variables
int state;
int received;
bool failedToSend;
float dropDistance = 100;
int lateralTime;
int rotateTime;
int rotateStart;
int lateralStart;
int rotateOther;
int lateralOther;
int going = 10;

int normalRotSpeed = 90;
int normalLatSpeed = 255;
int normalClawSpeed = 60;
int jumpStart = 150;

/****************** Radio Config ***************************/
/***      0 for sender    1 for receiver    ***/
//bool radioNumber = 1;
//RF24 radio(1,2); /*change to pins 7 & 8 */
//byte addresses[][6] = {"1Node","2Node"};
/**********************************************************/



void setup() {
  Serial.begin(9600);
  Serial.print("Setting up");

  pinMode(rotatorPinEnable, OUTPUT);
  pinMode(lateralPinEnable, OUTPUT);
  pinMode(clawPinEnable, OUTPUT);
  pinMode(grabberPin, OUTPUT);
  pinMode(rot1, OUTPUT);
  pinMode(rot2, OUTPUT);
  pinMode(lat1, OUTPUT);
  pinMode(lat2, OUTPUT);
  pinMode(claw1, OUTPUT);
  pinMode(claw2, OUTPUT);

  analogWrite(rotatorPinEnable, normalRotSpeed);
  analogWrite(lateralPinEnable, normalLatSpeed );
  analogWrite(clawPinEnable, normalClawSpeed);
  grabber.attach(grabberPin);
  go(STOP);
  grabber.write(180); //initially open

  state = 4;

  //pixy
  pixy.init();
  i = 0;
  firstCheck = 0;

  //timing
  rotateTime = 0;
  lateralTime = 0;
  rotateStart = 0;
  lateralStart = 0;
  rotateOther = 0;
  lateralOther = 0;

  //radio
  //  radio.begin();
  //  initRadio();
  received = 0;
  failedToSend = true;


}

void loop() {

//    while(true) {
//        testPixyXY();
//      if (findVictimR()) { 
//        Serial.println("true"); 
//      }
//    }
  
  switch (state) {

    //Wait for Ambulance to connect
    case 0:
      Serial.println("CASE 0");

      waitForAmbulance();

      go(STOP);
      delay(200);
      analogWrite(rotatorPinEnable, jumpStart); //jump start
      go(COUNTER_CLOCKWISE);
      delay(300);
      analogWrite(rotatorPinEnable, normalRotSpeed); //reach normal speed
      go(COUNTER_CLOCKWISE);
      rotateStart = millis();
      rotateOther = 0;
      go(3); // del
      lateralStart = millis();//
      state = 1;
      openClaw();
      Serial.println("opened claw");
      break;

    //Locate victim (Theta coordinate)
    case 1:

      Serial.println("CASE 1");
      if (findVictimTheta()) {
        state = 2;
        rotateTime = millis() - rotateStart;
        break;
      }


      lateralStart = millis();

      if ((millis() - rotateStart) > 3000 && going == 1) {
        Serial.println("Switch DirectionC");
        Serial.println("1");
        rotateStart = millis();
        go(STOP);
        delay(200);
        analogWrite(rotatorPinEnable, jumpStart); //jump start
        go(COUNTER_CLOCKWISE);
        delay(300);
        analogWrite(rotatorPinEnable, normalRotSpeed); //reach normal speed
        go(COUNTER_CLOCKWISE);
      }
      else if ((millis() - rotateStart) > 3000 && going == 2) {
        Serial.println("Switch Direction");
        Serial.println("2");
        rotateStart = millis();
        go(STOP);
        delay(200);
        analogWrite(rotatorPinEnable, jumpStart); //jump start
        go(CLOCKWISE);
        delay(300);
        analogWrite(rotatorPinEnable, normalRotSpeed); //reach normal speed
        go(CLOCKWISE);
      }

      break;

    //Locate victim (r coordinate)
    case 2:

      Serial.println("CASE 2");
      if (findVictimR()) {
        lateralTime = millis() - lateralStart;
        state = 3;
        break;
      }
      //
      //          while(1) {
      //            go(LATERAL_BACK);
      //            delay(7000);
      //            go(LATERAL_FORWARD);
      //            delay(7000);
      //          }
      Serial.println(millis() - lateralStart);
      if ((millis() - lateralStart) > 5000 && going == 3) {
        Serial.println("Switch DirectionF");
        lateralStart = millis();
        go(STOP);
        delay(200);
        analogWrite(lateralPinEnable, jumpStart); //jump start
        go(LATERAL_BACK);
        delay(3000);
        analogWrite(lateralPinEnable, normalLatSpeed); //reach normal speed
        go(LATERAL_BACK);
      }
      else if ((millis() - lateralStart) > 5000 && going == 4) {
        Serial.println("Switch Direction");
        lateralStart = millis();
        go(STOP);
        delay(200);
        analogWrite(lateralPinEnable, jumpStart); //jump start
        go(LATERAL_FORWARD);
        delay(3000);
        analogWrite(lateralPinEnable, normalLatSpeed); //reach normal speed
        go(LATERAL_FORWARD);
      }
      break;

       //Lower claw
    case 3:
      Serial.println("CASE 3");
      
//      while(1) { 
//          Serial.println("CASE 3");
////          go(CLAW_DOWN);
////          delay(2000);
//          go(CLAW_UP);
//          delay(2000);
//      }
      go(CLAW_DOWN);
      findVictimZ();
      delay(1000); //hard code

      go(CLAW_UP);
      findVictimZ();
      delay(1000); //hard code
      state++;     //hard code
      break;

    //Close claw, then lift a few inches off the ground
    case 4:
      Serial.println("CASE 4");

      while (1) {
        closeClaw();
        analogWrite(clawPinEnable, 152);
        openClaw();
      }
      closeClaw();
      analogWrite(clawPinEnable, 152);
      go(CLAW_UP); //Lift up a little bit
      delay(500);

      Serial.println("closed claw");
      Serial.print("timeRotated: ");
      Serial.println(rotateTime);
      Serial.print("timeTheta: ");
      Serial.println(lateralTime);

      //Move crane to Origin
      toOrigin();
      Serial.println("returned to origin");

      //Lower claw to ambulance
      clawDrop();
      Serial.println("dropped claw");

      //Open claw
      openClaw();
      Serial.println("opened claw");

      //send - This will block until complete
      Serial.println("Victim deposited");
      //          while(failedToSend) {
      //            if (!radio.write(237, sizeof(int) )){
      //               Serial.println("failed");
      //            }
      //            else {
      //              failedToSend = false;
      //            }
      //          }

      //Raise claw
      raiseClaw(0);
      Serial.println("raised claw");
      state = 5;
      break;

    //end
    case 5:
      Serial.println("Done");
      delay(10000);
      break;
  }
}

/**
   All other functions

*/


void toOrigin() { //move crane to default position

  analogWrite(lateralPinEnable, 142); //jump start
  go(LATERAL_BACK);
  delay(300);
  analogWrite(lateralPinEnable, normalLatSpeed); //reach normal speed
  go(LATERAL_BACK);
  delay(lateralTime);

  analogWrite(rotatorPinEnable, 142); //jump start
  go(COUNTER_CLOCKWISE);
  delay(300);
  analogWrite(rotatorPinEnable, normalRotSpeed); //reach normal speed
  go(COUNTER_CLOCKWISE);
  delay(rotateTime);

}

void clawDrop() { //Lower claw to ambulance
  go(CLAW_DOWN);
  delay(AMBULANCE_DIST);
  go(STOP);
}

void openClaw() { //Open claw to release victim
  go(CLAW_OPEN);
  delay(coTime);
  go(STOP);
}

void closeClaw() { //Close claw to secure victim
  go(CLAW_CLOSE);
  delay(ccTime);
  go(STOP);

}

void raiseClaw(float location) { //Raise claw to default position z=0
  go(CLAW_UP);
  delay(2000);
  go(STOP);

}

boolean findVictimZ() { // (Signature 1)

  // grab blocks!
  blocks = pixy.getBlocks();

  // If there are detected blocks, print them!
  if (blocks) {
    i++;

    //print every 50 frames because printing every frame would bog down the Arduino
    if (i % 25 == 0) {

      if (pixy.blocks[0].signature == 1) {

        Serial.print("area: ");
        Serial.println(pixy.blocks[0].width * pixy.blocks[0].height);
        if ((pixy.blocks[0].width * pixy.blocks[0].height) > 15000) {

          go(STOP);
          Serial.println("found victim z");
          state++;
          return true; 
        }
      }
    }
  }
}

boolean findVictimR() { // (Signature 1)

  // grab blocks!
  blocks = pixy.getBlocks();

  // If there are detected blocks, print them!
  if (blocks) {

    i++;

    //print every 50 frames because printing every frame would bog down the Arduino
    if (i % 25 == 0) {

      if (pixy.blocks[0].signature == 1) {
        Serial.print("r0: ");
        Serial.print(pixy.blocks[0].y);
        Serial.println();
        if ((pixy.blocks[0].y < 135 && pixy.blocks[0].y > 85)) {
          state++;
          Serial.println("found victim r");
          go(STOP);
          delay(500);
          return true;
        }
      }
    }
  }
  return false;
}

boolean findVictimTheta() { // (Signature 1)

  // grab blocks!
  blocks = pixy.getBlocks();

  // If there are detected blocks, print them!
  if (blocks) {
    Serial.print("theta0: ");
    i++;

    //print every 25 frames
    if (i % 1 == 0) {

      if (pixy.blocks[0].signature == 1) {
        Serial.print("theta0: ");
        Serial.print(pixy.blocks[0].x);
        Serial.println();
        if ((pixy.blocks[0].x < 245 && pixy.blocks[0].x > 210)) {
          state++;
          Serial.println("found victim theta");

          go(STOP);
          delay(200);
          analogWrite(lateralPinEnable, 100); //jump start
          go(LATERAL_FORWARD);
          delay(300);
          analogWrite(lateralPinEnable, 83); //reach normal speed
          go(LATERAL_FORWARD);
          return true;
        }
      }
    }

  }
  return false;
}


void waitForAmbulance() { //loops here until ambulance color sensed (Signature 2)

  //RF receive integer
  //   if( radio.available()){
  //      while (radio.available()) {              // While there is data ready
  //         radio.read( &received, sizeof(int) );  // Get the payload
  //         Serial.println(received);
  //      }
  //   }
  //   if(received == 13) {

  while (true) {
    blocks = pixy.getBlocks();

    // If there are detected blocks, print them!
    if (blocks) {
      i++;
      if (i % 10 == 0) {
        Serial.print("x0: ");
        Serial.println(pixy.blocks[0].x);
        Serial.print("y0: ");
        Serial.println(pixy.blocks[0].y);
        Serial.println();
        if (pixy.blocks[0].signature == 2) {
          i = 0;
          break;
        }

      }
    }
  }

}

void testPixyXY() {
  // grab blocks!
  blocks = pixy.getBlocks();

  // If there are detected blocks, print them!
  if (blocks) {
    i++;
    if (i % 50 == 0) {
      if (pixy.blocks[0].signature == 1 ) {
        Serial.print("x0: ");
        Serial.println(pixy.blocks[0].x);
        Serial.print("y0: ");
        Serial.println(pixy.blocks[0].y);
        Serial.println();
      }
    }
  }
}

//void initRadio() {
//
//  radio.setPALevel(RF24_PA_LOW);
//  radio.setChannel(80);
//
//  // Open a writing and reading pipe
//  if(radioNumber){
//    radio.openWritingPipe(addresses[1]);
//    radio.openReadingPipe(1,addresses[0]);
//  }else{
//    radio.openWritingPipe(addresses[0]);
//    radio.openReadingPipe(1,addresses[1]);
//  }
//  // Make sure the radio starts listening for data
//  radio.startListening();
//}

void go(int dir) { //keep track of millis moved to know where origin is
  switch (dir) {
    case 0:  //stop
      analogWrite(rotatorPinEnable, 0);
      analogWrite(lateralPinEnable, 0 );
      analogWrite(clawPinEnable, 0);
      digitalWrite(rot1, LOW);
      digitalWrite(rot2, LOW);
      digitalWrite(lat1, LOW);
      digitalWrite(lat2, LOW);
      digitalWrite(claw1, LOW);
      digitalWrite(claw2, LOW);
      going = 0;
      break;
    case 1: //rotate clockwise
      analogWrite(rotatorPinEnable, normalRotSpeed);
      analogWrite(lateralPinEnable, 0 );
      analogWrite(clawPinEnable, 0);
      digitalWrite(rot1, LOW);
      digitalWrite(rot2, HIGH);
      digitalWrite(lat1, LOW);
      digitalWrite(lat2, LOW);
      digitalWrite(claw1, LOW);
      digitalWrite(claw2, LOW);
      going = 1;
      break;
    case 2: //rotate counterclockwise
      analogWrite(rotatorPinEnable, normalRotSpeed);
      analogWrite(lateralPinEnable, 0);
      analogWrite(clawPinEnable, 0);
      digitalWrite(rot1, HIGH);
      digitalWrite(rot2, LOW);
      digitalWrite(lat1, LOW);
      digitalWrite(lat2, LOW);
      digitalWrite(claw1, LOW);
      digitalWrite(claw2, LOW);
      going = 2;
      break;
    case 3: //go forward laterally
      analogWrite(rotatorPinEnable, 0);
      analogWrite(lateralPinEnable, normalLatSpeed );
      analogWrite(clawPinEnable, 255);
      digitalWrite(rot1, LOW);
      digitalWrite(rot2, LOW);
      digitalWrite(lat1, HIGH);
      digitalWrite(lat2, LOW);
      digitalWrite(claw1, HIGH);
      digitalWrite(claw2, LOW);
      going = 3;
      break;
    case 4: //go backward laterally/
      analogWrite(rotatorPinEnable, 0);
      analogWrite(lateralPinEnable, normalLatSpeed);
      analogWrite(clawPinEnable, 25);
      digitalWrite(rot1, LOW);
      digitalWrite(rot2, LOW);
      digitalWrite(lat1, LOW);
      digitalWrite(lat2, HIGH);
      digitalWrite(claw1, LOW);
      digitalWrite(claw2, HIGH);
      going = 4;
      break;
    case 5: //claw down
      analogWrite(rotatorPinEnable, 0);
      analogWrite(lateralPinEnable, 0);
      analogWrite(clawPinEnable, normalClawSpeed);
      digitalWrite(rot1, LOW);
      digitalWrite(rot2, LOW);
      digitalWrite(lat1, LOW);
      digitalWrite(lat2, LOW);
      digitalWrite(claw1, HIGH);
      digitalWrite(claw2, LOW);
      going = 5;
      break;
    case 6: //claw up
      analogWrite(rotatorPinEnable, 0);
      analogWrite(lateralPinEnable, 0);
      analogWrite(clawPinEnable, 255);
      digitalWrite(rot1, LOW);
      digitalWrite(rot2, LOW);
      digitalWrite(lat1, LOW);
      digitalWrite(lat2, LOW);
      digitalWrite(claw1, LOW);
      digitalWrite(claw2, HIGH);
      going = 6;
      break;
    case 7: //grabber close
      analogWrite(rotatorPinEnable, 0);
      analogWrite(lateralPinEnable, 0);
      analogWrite(clawPinEnable, 0);
      digitalWrite(rot1, LOW);
      digitalWrite(rot2, LOW);
      digitalWrite(lat1, LOW);
      digitalWrite(lat2, LOW);
      digitalWrite(claw1, LOW);
      digitalWrite(claw2, LOW);
      for ( int i = 100; i<=178; i=i+1) {
        grabber.write(i);
        delay(10);
      }
      
      going = 7;
      break;
    case 8: //grabber open
      analogWrite(rotatorPinEnable, 0);
      analogWrite(lateralPinEnable, 0);
      analogWrite(clawPinEnable, 0);
      digitalWrite(rot1, LOW);
      digitalWrite(rot2, LOW);
      digitalWrite(lat1, LOW);
      digitalWrite(lat2, LOW);
      digitalWrite(claw1, LOW);
      digitalWrite(claw2, LOW);
       for ( int i=178; i>=100; i=i-1) {
        grabber.write(i);
        delay(10);
      }
      going = 8;
      break;
  }

}


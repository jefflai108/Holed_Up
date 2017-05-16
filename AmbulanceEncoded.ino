/**
 * Final Project Ambulance
 * Nick, Hao, Andrew, Jeff
 * 4/28/2017
 */

//Libraries
#include <SPI.h>
#include <Encoder.h>
#include <Wire.h>
#include <RF24.h>
#include <LSM303.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h> //for OLED display
#include <RF24.h>
#include <TimedAction.h>
#include <Utility.h>

//Compass
LSM303 mag;
#define NORTH  300
#define EAST   80
#define SOUTH  190
#define WEST   240
LSM303::vector<int16_t> running_min = {32767, 32767, 32767}, running_max = {-32768, -32768, -32768};
char report[80];

Encoder encRight(2,3);
Encoder encLeft(27,28);

//IR sensors
#define sensA  0  //Front Right
#define sensB  1  //Front Left
#define sensC  2  //Back Right
#define sensD  3  //Back Left

//Motors
#define right1  14
#define right2  15
#define left1   16
#define left2   17
#define STOP       0
#define FORWARD    1
#define CLOCKWISE  2
#define COUNTER    3


//OLED display //for Mega
#define OLED_MOSI   51 //mosi
#define OLED_CLK   52  //sck
#define OLED_DC    50  //miso
#define OLED_CS    53  //ss
#define OLED_RESET 13
Adafruit_SSD1306 display(OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

//Threads
void changeText();
void soundPulse();
TimedAction textThread = TimedAction(3000, changeText);
TimedAction sirenThread = TimedAction(700, soundPulse);
boolean text = false;
boolean pulse = false;
int strength = 0;



<------------------------------------------------------------------------------------------------

//Thresholds
float threshold = 13.0;
float leftHandThreshold = 20.0; ////LOWER THIS TO MAKE SURE OBSTACLE AVOIDANCE WORKS (SO IT NEVER ENCOUNTERS A RIGHT HAND OBSTACLE)


<------------------------------------------------------------------------------------------------ 



//Radio      1 for sender    0 for receiver
bool radioNumber = 1;
RF24 radio(7,8); /*pins 7 & 8 */
byte addresses[][6] = {"1Node","2Node"};
bool failedToSend;
int received;

//Siren
#define sirenPin 8

//Variables                    ^
int state;              //     |
int dir; //NSEW --> 0123       0
int nextAngle;          //   3   1
                        //     2
float distA;
float distB;
float distC;
float distD;
long beginIt;

long forwardDistance;
long lateralDistance;
long backwardDistance;
long checkFwdDist;
long maxDistance = 40000;
long lateralMax = 20000;

int adj = 20;
int prevHeading;

int sirenPin1 = 12;
long lastEncoderDistance = 0;

bool moving = false;


/*****************FUNCTIONS*******************
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 */

/**
 * Siren audio pulse thread
 */
void soundPulse(){
   
   if (pulse) {
      //write High voltage to piezo
      analogWrite(sirenPin1, 255);
      pulse = !pulse;
   }
   else {
      //write Low voltage to piezo
      analogWrite(sirenPin1, 150);
      pulse = !pulse;
   }
}

/**
 * OLED update information thread
 * rotates between three messages
 */
void changeText(){  
  Serial.println("Changed Text");
  if (text) {
    display.clearDisplay();
    oledPrint("Emergency Rescue Vehicle");
    display.display();
    text = 0;
  }
  else {
    display.clearDisplay();
    oledPrint("EVERYBODY MOOOOVE");
    display.display();
    text = 1;
  }
}

/**
 * Read all the IR sensor values into their distance variables
 */
 void getIRVals() {
            
      distA = 0; int voltsA = 0;
      distB = 0; int voltsB = 0;
      distC = 0; int voltsC = 0;
      distD = 0; int voltsD = 0;
      
      delay(20);
      for (int i = 0; i < 10; i++) {      
        voltsA = analogRead(sensA); 
        distA = distA + 1191*pow(voltsA, -0.75);
        voltsB = analogRead(sensB); 
        distB = distB + 1136*pow(voltsB, -0.73);
        voltsC = analogRead(sensC); 
        distC = distC + 860*pow(voltsC, -0.65250);
        voltsD = analogRead(sensD);  
        distD = distD + 860*pow(voltsD, -0.65250);  
        delay(5);
      }   
    
      if (isinf(distA)) { distA = 1000;}
      else {
        distA = distA / 10;
      }
  
      if (isinf(distB)) { distB = 1000;}
      else {
        distB = distB / 10;
      }

      if (isinf(distC)) { distC = 1000;}
      else {
        distC = distC / 10;
      }
        
      if (isinf(distD)) { distD = 1000;}
      else {
        distD = distD / 10;
      }
//        Serial.print("distA: ");
//        Serial.println(distA);
//        Serial.print("distB: ");
//        Serial.println(distB);
//        Serial.print("distC: ");
//        Serial.println(distC);
//        Serial.print("distD:"); 
//        Serial.println(distD);   
//        Serial.print("\n\n");
}


/****Encoder turns ***



*****/

void encoderLeft() {
  
  beginIt = encRight.read();
  go(COUNTER);
  while (encRight.read() + 6300 > beginIt) {
    textThread.check();
    sirenThread.check();
  }
  go(FORWARD);
  forwardDistance += 6300;
  
}

void encoderRight() {
  beginIt = encRight.read();
  go(CLOCKWISE);
  while (encRight.read() - 5000 < beginIt) {
    textThread.check();
    sirenThread.check();
  }
  go(FORWARD);
  forwardDistance += 5000;
}


/*****




******End Encoder turns*/


/**
 * Sets the motor speeds
 */
void go(int dir) {
  switch (dir) {

    case 0: //Stop

      digitalWrite(right2,LOW);
      digitalWrite(right1,LOW);
      digitalWrite(left1,LOW);
      digitalWrite(left2,LOW);
      break;
    case 1: //Forward
      digitalWrite(right1,LOW);
      digitalWrite(right2,HIGH);
      digitalWrite(left1,HIGH);
      digitalWrite(left2,LOW);
      break;
    case 2: //clockwise
      digitalWrite(right1,HIGH);
      digitalWrite(right2,LOW);
      digitalWrite(left1,HIGH);
      digitalWrite(left2,LOW);
      break;
    case 3: //counterclockwise
      digitalWrite(right1,LOW);
      digitalWrite(right2,HIGH);
      digitalWrite(left1,LOW);
      digitalWrite(left2,HIGH);
      break;
  }

}


/**
 * Print message to OLED display
 */
void oledPrint(String message) {
  
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  //convert message to char array

  for (int i = 0; i < message.length(); i++) {
    display.print(message[i]);  
  }
  display.println();
  display.display();
  
}


/****START RF CODE********




*****************************************************************/

/**
 * Initializes radio r/w pipes
 */
void initRadio() {
  
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
  radio.stopListening();
}

/**
 * Waits for RF transmission confirming victim dropped
 */
 void waitForVictim() {
   radio.startListening();
   while (received != 237) {
     while(!radio.available()) {
      delay(10);
     }
       //RF receive integer
     if( radio.available()){
        while (radio.available()) {              // While there is data ready
           radio.read( &received, sizeof(int) );  // Get the payload
           Serial.println(received); 
        }
     } 
   }
   radio.stopListening();
   Serial.println("Victim Received");
}

/**
 * Sends RF transmission to crane when below crane arm
 */

void notifyArrived() { //when ambulance has reached the end
  Serial.println("Arrived");
  while(failedToSend) {
     if (!radio.write(13, sizeof(int))) {
       Serial.println(("failed"));
     }
     else{
      failedToSend = false;
     }
  }
  Serial.println("Sent arrival status");
}


/****END RF CODE***************




**********************************************************/


//updates distance traveled variables
void updateDistance() {
 
    switch(dir) { //just turned
  
      case 0:
        //increases distance
        forwardDistance += -encRight.read() - lastEncoderDistance;
        Serial.println(forwardDistance);
        break;
  
      case 1:
        //increases distance
        lateralDistance += -encRight.read() - lastEncoderDistance;
        Serial.println(lateralDistance);
        break;
            
      case 2:             
         
        forwardDistance -= (-encRight.read() - lastEncoderDistance);
        Serial.println(forwardDistance);
        break;
      case 3:
  
        lateralDistance -= (-encRight.read() - lastEncoderDistance);
        Serial.println(lateralDistance);
        break;
        
      default:
      //do nothing
      break;
    }
  
}

void movingUpdate() {
    checkFwdDist = -encRight.read() - lastEncoderDistance;
    if(checkFwdDist > maxDistance) {
        go(STOP);
        endGame();        
    } 
}



/*********************SETUP********************
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 */

void setup() {
  //Initialize serial
  Serial.begin(9600);
  Serial.print("Setting up...");

  //displays
  display.begin(SSD1306_SWITCHCAPVCC);
  display.display();
  Serial.println("Roll Out");
  display.clearDisplay();
  oledPrint("Roll Out");
  state = 0;
  
  //IR pins
  pinMode(sensA,INPUT); //left front
  pinMode(sensB,INPUT);  //right front
  pinMode(sensC,INPUT); //left side
  pinMode(sensD,INPUT);  //right side

  //Motor Pins
  pinMode(right1, OUTPUT);
  pinMode(right2, OUTPUT);
  pinMode(left1, OUTPUT);
  pinMode(left2, OUTPUT);
  pinMode(5,OUTPUT); //right motor pwm
  pinMode(6,OUTPUT); //left motor pwm
  analogWrite(5,255);
  analogWrite(6,215); 
  go(STOP);

  pinMode(sirenPin1, OUTPUT);
  //radio -- causing issues with other SPI
  radio.begin();
  initRadio();
  received = 0;
  failedToSend = true;

  //Text
  changeText();
  textThread.enable();

  //direction initially "north" (towards the table)
  dir = 0;
  nextAngle = 0;
  go(FORWARD);

  forwardDistance = 0;
  lateralDistance = 0;
  backwardDistance = 0;

  lastEncoderDistance = 0;
  
  go(FORWARD);
  delay(2000);
  moving = true;
  
}

/*********************LOOP********************
*
*
*
*
*
*
*
*
*
*
*
*
*
 */


void loop() {

//go forward until left side disappears or obstacle encountered
/*********************************************************************************************/
  getIRVals();
  while (distD < leftHandThreshold) { //while hand on left side
    Serial.println(distD);
    display.clearDisplay();
    oledPrint("Hand on Left");
    
    movingUpdate();  //if moving forward (north), check to see if past max forward
    
    getIRVals();
    
    textThread.check();
    sirenThread.check();
    
    if (distA < threshold || distB < threshold) { //if blocked, turn right to keep hand on left wall
      Serial.println("Blocked in Front");
      switch(dir) { //get angle of right turn final location depending on current location
        case 0:
          Serial.println("turning east");
          display.clearDisplay();
          oledPrint("turning east");
          break;
        case 1:
          Serial.println("turning south");
          display.clearDisplay();
          oledPrint("turning south");
          break;
        case 2:
          Serial.println("turning west");
          display.clearDisplay();
          oledPrint("turning west");
          break;
        case 3:
          Serial.println("turning north");
          display.clearDisplay();
          oledPrint("turning north");
          break;
      }
      updateDistance();  //save increase to old direction
      encoderRight();  //turn
      lastEncoderDistance = -encRight.read(); //update starting point for new distance
      dir += 1;
      if (dir == 4) {
        dir = 0;
      }
    }
    getIRVals();
  }


  //left side disappeared! Turn into the space
/*********************************************************************************************/
  delay(500);
    Serial.println("Left side disappeared");
    switch(dir) {
        case 0:
          Serial.println("turning west");
          display.clearDisplay();
          oledPrint("turning west");
          break;
        case 1:
          Serial.println("turning north");
          display.clearDisplay();
          oledPrint("turning north");
          break;
        case 2:
          Serial.println("turning east");
          display.clearDisplay();
          oledPrint("turning east");
          break;
        case 3:
          Serial.println("turning south");
          display.clearDisplay();
          oledPrint("turning south");
          break;
      }
      updateDistance(); //save increase to old direction
      encoderLeft();  //turn
      lastEncoderDistance = -encRight.read();
      dir -= 1;
      if (dir == -1) {
        dir = 3;
      }


//go forward until left reappears, turning until not blocked in front
/*********************************************************************************************/
 
    getIRVals();
    while(distD > leftHandThreshold) {
      display.clearDisplay();
      oledPrint("Waiting for HandL");

      sirenThread.check();
      textThread.check();
     
      movingUpdate();  //if moving forward (north), check to see if past max forward
     
      getIRVals();
      if (distA < threshold || distB < threshold) { //if blocked, turn right to keep hand on left wall
        Serial.println("Blocked in Front");
        updateDistance(); //save increase to old direction
        encoderRight();  //turn
        lastEncoderDistance = -encRight.read();
        dir += 1;
        if (dir == 4) {
          dir = 0;
        }
      }
      getIRVals();
    }

}

//ENDGAME - move to table and handle patient
/*****************************************************************************
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 */
void endGame() {
    delay(1000000);  //delete once checked to see if endgame reached

    if(lateralDistance > lateralMax) {
       go(FORWARD);
       delay(4000);
       go(STOP);       
    }

    else{
       go(FORWARD);
       delay(4000);
       go(STOP);
       encoderRight();
       go(FORWARD);
       delay(10000);
       go(STOP);
    }

    notifyArrived();
    waitForVictim();

    go(FORWARD);
    delay(3000);
    go(STOP);
    
    delay(1000000);
}


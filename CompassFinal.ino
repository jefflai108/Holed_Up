
#include <Wire.h>
#include <LSM303.h>


LSM303 mag; //compass


void setup() {
  // put your setup code here, to run once:


  Wire.begin();
  mag.init();
  mag.enableDefault();
  
  mag.m_min = (LSM303::vector<int16_t>){-236, -234, -527};
  mag.m_max = (LSM303::vector<int16_t>){+206, +238, -428};

}

void loop() {

//when going forward do this within forward loop:

if(get_heading() <= 67) {
        compassRightReturn();
        delay(10);
      }
      else if (get_heading() >= 225) {

        compassLeftReturn();
        delay(10);
      }
  
  }
}

float get_heading() {
  mag.read();
  return mag.heading();
}

void compassLeft() { //turns 90 degrees to left based on compass reading

  //go(3); //go left
  delay(50);
  get_heading();
  while  (get_heading() >= 69) {

    Serial.println(get_heading());
    delay(10);
    
  }
  go(1); //go forward

}

void compassRight() { //turns 90 degrees to right based on compass reading

 // go(4);   //go right
  delay(50);
  get_heading();
  while (get_heading() <= 240) {
    
    Serial.println(get_heading());
    delay(10);
    
  }
  go(1);

  
}

void compassLeftBack() { //turns 90 degrees to left back based on compass reading

  // go(3); //go left
  delay(50);
  get_heading();
  while  (get_heading() >= 162) {

    Serial.println(get_heading());
    delay(10);
    
  }
  go(1);

 
}

void compassRightBack() { //turns 90 degrees to right back based on compass reading 
             
  //go(4);  //go right
  delay(250);
  get_heading();
  while (get_heading() <= 158) {
    
    Serial.println(get_heading());
    delay(10);
    
  }
  go(1);

  
}


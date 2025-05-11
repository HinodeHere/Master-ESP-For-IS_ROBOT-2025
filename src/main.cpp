#include <Arduino.h>
#include <Wire.h>
#include "Adafruit_PWMServoDriver.h"
#include "ClawUpDown.h"
#include "ultrasound.h"
#include "colourSensor.h"

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

#define servoMinPlates 100
#define servoMaxPlates 520 //test your own servo!

#define ClawServoMin 50
#define ClawServoMax 560
// 250 to 350 for open and close

#define servoFreq 50

//PWM Pins
#define SCL 40
#define SDA 38

//Coms with Slave ESP
#define SLAVE_ADDR 0x08
#define freq 100000
uint8_t data[2];

#define heightOrigin 5900
#define ground -50
#define upperPlate 2600
#define plate 1750
#define AfterPlate 2400
#define standby 950
#define wallHeight 4800
#define upperWall 5100

//channel 0 = Red Table (Front Table)
// 1 = claw spinner
// 2 = Blue Table (Back Table)
// 3 = claw
#define redTable 0
#define clawSpinner 1
#define blueTable 14
#define claw 3

void setServoAngle(int channel, int angle){
  int pulse = map(angle,0,270,servoMinPlates,servoMaxPlates);
  pwm.setPWM(channel,0,pulse);
}

// @param 0 grab
// @param 1 full open
// @param 2 release on table
void clawControl(int mode){ //0 for grab, 1 for open, 2 for release on table
  if (mode == 0){
    pwm.setPWM(claw,0,360);
    delay(500);
  }
  else if(mode == 1){
    pwm.setPWM(claw,0,250);
    delay(500);
  }
  else if(mode == 2){
    pwm.setPWM(claw,0,300);
    delay(500);
  }
  
}

void spinBlueTable(){
  static int blueAngle = 90;
  setServoAngle(blueTable, blueAngle);
  blueAngle+=90;
  if (blueAngle == 270+90){
    blueAngle = 0;
  }
  Serial.println(blueAngle);
  delay(1500);
}

void spinRedTable(){
  static int redAngle = 90;
  setServoAngle(redTable, redAngle);
  redAngle+=90;
  if (redAngle == 270+90){
    redAngle = 0;
  }
  Serial.println(redAngle);
  
  delay(1500);
}


//@brief data (type, distance);
//@param type {0 = stop motors,1 = forward, 2 = back, 3 = left, 4 = right}
//@param speed （not using) should be 120rpm(?) (need to tune PID const!) 
//@param distance how many cm
void comWithSlave(int type, int distanceCM){
  data[0] = type; data[1] = distanceCM;
  Wire.beginTransmission(SLAVE_ADDR);
  Wire.write((uint8_t*)data,2);
  Wire.endTransmission();
  Serial.println("Sent data to test!");
}

void takeFire(){
  Serial.println("Taking Fire!");
  // clawControl(1); //already done when it is in standbymode and after placing fire on the tables
  clawHeightTo(ground);
  delay(100);
  clawControl(0);
  clawHeightTo(upperPlate);
}

void placeFireOnRedTable(){
  setServoAngle(clawSpinner,0);

  delay(500);

  clawHeightTo(plate);
  delay(150);
  clawControl(2);

  clawHeightTo(AfterPlate);
  setServoAngle(clawSpinner,135);

  clawHeightTo(standby);
  clawControl(1);
  spinRedTable();
}

void placeFireOnBlueTable(){
  setServoAngle(clawSpinner,270);

  delay(500);

  clawHeightTo(plate);
  delay(150);
  clawControl(2);

  clawHeightTo(AfterPlate);
  setServoAngle(clawSpinner,135);

  clawHeightTo(standby);
  clawControl(1);
  spinBlueTable();
}

void waitUntilMotorIsDone() {
  // Wait for slave to finish moving
  while (true) {
    Wire.requestFrom(SLAVE_ADDR, 1);
    if (Wire.available()) {
      uint8_t status = Wire.read();
      if (status == 0) break;  // done moving
    }
    delay(50);  // avoid spamming I2C
  }
}

//@brief data (type, distance);
//@param type {0 = stop motors,1 = forward, 2 = back, 3 = left, 4 = right}
//@param speed （not using) should be 120rpm(?) (need to tune PID const!) 
//@param distance how many cm
void moveWithWaiting(int type, int cm){
  comWithSlave(type,cm);
  waitUntilMotorIsDone();
  delay(50);
}

void putOneSetOnWall(){ //move 7 cm between 2 fires
  //go to above the blue fire
  setServoAngle(clawSpinner,270);
  clawHeightTo(upperPlate);
  clawControl(2);

  //grab fire
  clawHeightTo(plate);
  clawControl(0);

  //go up to the wall
  clawHeightTo(wallHeight);
  setServoAngle(clawSpinner,135);
  delay(1000);
  
  //release fire
  clawControl(1);
  clawHeightTo(upperWall);
  spinBlueTable();

  moveWithWaiting(1,7);

  //go to above the red fire
  setServoAngle(clawSpinner,0);
  clawHeightTo(upperPlate);
  clawControl(2);

  //grab fire
  clawHeightTo(plate);
  clawControl(0);

  //go up to the wall
  clawHeightTo(wallHeight);
  setServoAngle(clawSpinner,135);
  delay(1000);

  waitUntilMotorIsDone(); //just in case if it havent reach the next position!
  
  //release fire
  clawControl(1);
  clawHeightTo(upperWall);
  spinRedTable();
}


enum MoveDir {STOP = 0, FORWARD=1, BACK=2, LEFT=3, RIGHT=4};
float PickupDist = 4.6f;
float PutDownDist = 4.6f; //might be changed later

void moveToNextPoint(int rowNum){
  if (rowNum % 2 == 1){
    moveWithWaiting(BACK,70); //go backwards if it's in odd rows
  }
  else{
    moveWithWaiting(FORWARD,70); //go forward if it's in even rows
  }
}


void setup() {
  Serial.begin(115200);
  
  //Initialize different parts
  clawUpDownInit();
  setupUltrasound();
  ColourSensorSetup();


  Wire.begin(SDA,SCL,freq);
  pwm.begin();
  pwm.setPWMFreq(servoFreq);
  delay(500);

  //put them in the desired position
  // comWithSlave(0,0);
  clawControl(1);
  setServoAngle(clawSpinner,135);
  setServoAngle(blueTable,0);
  setServoAngle(redTable,0);
}


void loop(){
    clawHeightTo(standby);
    for (int i = 0; i < 2; i++){
      float distance = getPreciseDistance();
      while(distance > 4.6 || distance == -1){
        Serial.println(distance);
        delay(50);
        distance = getPreciseDistance();
      }


      takeFire();
      Serial.println("done taking, now scanning for colour");

      // Colour sensing
      String colour = MajorityVoteColourRead();
      Serial.println(colour);

      comWithSlave(1,70);
      
      if (colour == "Red"){
        placeFireOnRedTable();
      } else{
        placeFireOnBlueTable();
      }
      waitUntilMotorIsDone();
    }
    moveWithWaiting(2,140);


    clawHeightTo(heightOrigin-100);

    while(1);
}




// int rowNum = 1;
// int columnCount = 1;
// void loop() {
  
//   //put code to calibrate the claw to be on the max

//   clawHeightTo(standby);
//   while (getPreciseDistance() > 14){
//     delay(100); //wave your hand to start the run!
//   }

//   //move to the first Junciton
//   moveWithWaiting(FORWARD,140);
//   moveWithWaiting(LEFT,60);
  
//   //check for fire using ultrasound
//   rowNum = 1;
//   columnCount = 1;
//   while (rowNum != 4){
//     float distance = getPreciseDistance();
//     if (distance > 20 || distance == -1){ //if there's no fire
//       columnCount++;
//       moveToNextPoint(rowNum);


//     } else{
//       unsigned long t0 = millis(); //for timeout of 3s
//       while (distance > PickupDist){ //if there's a fire and it's near!
//         moveWithWaiting(LEFT,1); //go left for 1 cm if the fire is near but not reachable
//         distance = getPreciseDistance();
//         delay(100);
//       }

//       // //go to the next junction if we timed out
//       // if (millis() - t0 >= 3000){
//       //   columnCount++;
//       //   moveToNextPoint(rowNum);
//       //   continue; //move to the next column
//       // }


//       //now that it is in range, take the fire now!
//       takeFire();

//       //Colour sensing
//       String colour = MajorityVoteColourRead();
//       if (colour == "Red"){
//         placeFireOnRedTable();
//       } else{
//         placeFireOnBlueTable();
//       }
//       columnCount++;
//       moveToNextPoint(rowNum);
//     }


//     if(columnCount == 5){
//       columnCount = 1;
//       moveWithWaiting(LEFT,65);
//       rowNum++;
//     }
//   }


//   //for placing fire on wall
//   float distance = getPreciseDistance();
//   unsigned long t0 = millis(); //for timeout of 3s
//   while(distance > PutDownDist && millis()-t0 < 3000){
//     moveWithWaiting(LEFT,1); //go left for 1 cm if the fire is near but not reachable
//     distance = getPreciseDistance();
//     delay(100);
//   }

//   //put the fires on the wall 4 times
//   for (int i = 0; i < 4; i++){
//     putOneSetOnWall();
//     delay(250);
//     moveWithWaiting(FORWARD,62);
//   }

//   while(1){}; //the run is done!
// }



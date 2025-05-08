#include <Arduino.h>
#include "ClawUpDown.h"

#define C1 10  // PWM capable
#define C2 12 // PWM capable
#define EN1 17
#define EN2 16

//Target is 5900 from start to bottom(floor to get the fire)
volatile int height = 5900; //height in pulses

void readEncoder() {
  bool a = digitalRead(EN1);
  bool b = digitalRead(EN2);
  if (a == b)
    height++;
  else
    height--;
}

void clawUpDownInit() {
    pinMode(EN1, INPUT_PULLUP);
    pinMode(EN2, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(EN1), readEncoder, RISING);
  }

void goDown(int PWM){
  analogWrite(C1, PWM);  // PWM signal
  analogWrite(C2, 0); // LOW
}

void goUp(int PWM){
  analogWrite(C1,0);
  analogWrite(C2,PWM);
}

void stop(){
  analogWrite(C1, 0);
  analogWrite(C2, 0);
}

void clawHeightTo(int targetHeight){
  int PWM = 255;
  int tolerance = 5;
  
  if(height < targetHeight){
    goUp(PWM);
    while (height < targetHeight - tolerance){
      delay(1);
    }
  } else if (height > targetHeight){
    goDown(PWM);
    while (height > targetHeight + tolerance){
      delay(1);
    }
  }

  stop();
}

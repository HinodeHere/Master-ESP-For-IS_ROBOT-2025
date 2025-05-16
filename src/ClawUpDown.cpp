#include <Arduino.h>
#include "ClawUpDown.h"

#define C1 10  // PWM capable
#define C2 12 // PWM capable
#define EN1 17
#define EN2 16

//Target is 5900 from start to bottom(floor to get the fire)
volatile int height = 5900; //height in pulses


const int Switch = 18;
int buttonState = 0;
void returnBack(){
  buttonState = digitalRead(Switch);
  if (buttonState != 1){
    goUp(175);
    digitalWrite(LED_BUILTIN,HIGH);
    Serial.println("Going Up!");
  }

  while (buttonState != 1){ //keep reading the value of the light
    delayMicroseconds(50);
    buttonState = digitalRead(Switch);
  }

  stop();
  digitalWrite(LED_BUILTIN,LOW);
  Serial.println("REACHED!");
  height = 5900;
  return;
}



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

    pinMode(Switch,INPUT); //for Putting the claw on the top!
    pinMode(LED_BUILTIN,OUTPUT);
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
      Serial.println(height);
    }
  } else if (height > targetHeight){
    goDown(PWM);
    while (height > targetHeight + tolerance){
      delay(1);
      Serial.println(height);
    }
  }
  height -= 7;

  stop();
}


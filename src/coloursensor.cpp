#include <Arduino.h>

#define s0 15
#define s1 4
#define s2 5
#define s3 6
#define sensorOut 7

// variables for coour PULSE WIDTH (not rgb value)
int redPW = 0;
int greenPW = 0;
int bluePW = 0;


// Get these from callibrating the colour sensor
int redMin = 100;
int redMax = 910;

int greenMin = 110;
int greenMax = 900;

int blueMin = 90;
int blueMax = 800;

//variable for final value
int redValue;
int greenValue;
int blueValue;


void ColourSensorSetup() {
  pinMode(s0,OUTPUT);
  pinMode(s1,OUTPUT);
  pinMode(s2,OUTPUT);
  pinMode(s3,OUTPUT);

  pinMode(sensorOut,INPUT);

  digitalWrite(s0,HIGH);
  digitalWrite(s1,LOW); //set pulse width(frequency) scalling to 20%
}

int getRedPW(){
  digitalWrite(s2,LOW);
  digitalWrite(s3,LOW);
  //read red only
  delayMicroseconds(100);
  int PW;
  PW = pulseIn(sensorOut,LOW);

  return PW;
}

int getGreenPW(){
  digitalWrite(s2,HIGH);
  digitalWrite(s3,HIGH);
  //read green only
  delayMicroseconds(100);
  int PW;
  PW = pulseIn(sensorOut,LOW);

  return PW;
}

int getBluePW(){
  digitalWrite(s2,LOW);
  digitalWrite(s3,HIGH);
  //read blue only
  delayMicroseconds(100); 
  int PW;
  PW = pulseIn(sensorOut,LOW);

  return PW;
}

int findmax(int red, int blue, int green){
  int errorMargin = 20;

  if (abs(red - blue) <= errorMargin && abs(red - green) <= errorMargin && abs(blue - green) <= errorMargin) {
    int average = (red + blue + green) / 3;
    if (average > 127) { // brightness threshold: >127 means closer to white
      Serial.print("White ");
    } else {
      Serial.print("Black ");
    }
    return average;
  }


  // Now compare red, blue, green, considering error margin
  if (red >= blue - errorMargin && red >= green - errorMargin) {
    Serial.print("Red ");
    return red;
  } 
  else if (green >= red - errorMargin && green >= blue - errorMargin) {
    Serial.print("Green ");
    return green;
  } 
  else {
    Serial.print("Blue ");
    return blue;
  }
}

static String colourName(int r, int b, int g) {
    const int err = 20;
    if ( abs(r-b)<=err && abs(r-g)<=err && abs(b-g)<=err ) {
      int avg = (r + b + g) / 3;
      return (avg > 127) ? "White" : "Black";
    }
    if (r >= b - err && r >= g - err) return "Red";
    if (g >= r - err && g >= b - err) return "Green";
    return "Blue";
}

String readColourOnce() {
    // 1) Sample
    redPW   = getRedPW();
    redValue   = map(redPW,   redMin,   redMax,   255, 0);
    delay(10);
  
    bluePW  = getBluePW();
    blueValue  = map(bluePW,  blueMin,  blueMax,  255, 0);
    delay(10);
  
    greenPW = getGreenPW();
    greenValue = map(greenPW, greenMin, greenMax, 255, 0);
    delay(10);
  
    // 2) Bounds check (debug)
    // if (redValue < 0 || greenValue < 0 || blueValue < 0) {
    //   Serial.println("Too far!");
    // }
    // else if (redValue > 255 || greenValue > 255 || blueValue > 255) {
    //   Serial.println("Too near!");
    // }
    // else {
    //   Serial.print("Red: ");   Serial.print(redValue);
    //   Serial.print("  Blue: ");Serial.print(blueValue);
    //   Serial.print("  Green: ");Serial.println(greenValue);
    // }
  
    // 3) Determine “max” colour
    String c = colourName(redValue, blueValue, greenValue);
    // Serial.print("Detected: "); Serial.println(c);
    return c;
}
  
String readColourValid() {
  String c;
  do {
    c = readColourOnce();
    if (c == "White" || c == "Black" || c == "Green") {
      Serial.println("→ Ignored, re-reading…");
      delay(100);
    }
  } while (c == "White" || c == "Black" || c == "Green");
  // at this point, c is either "Red" or "Blue"
  return c;
}

String MajorityVoteColourRead(){
  const int numReads = 5;
  int redCount = 0, blueCount = 0;

  while (redCount + blueCount < 3){
    String colour = readColourOnce();
    if (colour == "Red") redCount++;
    else blueCount++;
    delay(10);
  }

  if (redCount > blueCount){
    return "Red";
  }
  else {
    return "Blue";
  }
}
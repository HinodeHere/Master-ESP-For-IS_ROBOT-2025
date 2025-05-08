#ifndef COLOUR_SENSOR_H
#define COLOUR_SENSOR_H

#include <Arduino.h>

#define s0 15
#define s1 4
#define s2 5
#define s3 6
#define sensorOut 7

// variables for color PULSE WIDTH (not RGB value)
extern int redPW;
extern int greenPW;
extern int bluePW;

// Calibration values for color sensor
extern int redMin;
extern int redMax;
extern int greenMin;
extern int greenMax;
extern int blueMin;
extern int blueMax;

// Variables for final color values
extern int redValue;
extern int greenValue;
extern int blueValue;

// Function declarations
void ColourSensorSetup();
int getRedPW();
int getGreenPW();
int getBluePW();
int findmax(int red, int blue, int green);

String readColourOnce();
String readColourValid();
String MajorityVoteColourRead();

#endif // COLOUR_SENSOR_H

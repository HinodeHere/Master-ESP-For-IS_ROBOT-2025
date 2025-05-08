#ifndef ULTRASOUND_H
#define ULTRASOUND_H

#include <Arduino.h>

// Pre-calculate speed of sound (cm/µs) at your set temperature
extern const float SPEED_OF_SOUND;

// Function declarations
void setupUltrasound();
float getPreciseDistance();

#endif // ULTRASOUND_H

#ifndef CLAWUPDOWN_H
#define CLAWUPDOWN_H

#include <Arduino.h>

// Global encoder position
extern volatile int height;

// Interrupt handler for encoder
void readEncoder();

//init everything
void clawUpDownInit();

// Motor movement functions
void goDown(int PWM);
void goUp(int PWM);
void stop();

// Position control function
void clawHeightTo(int height);

#endif
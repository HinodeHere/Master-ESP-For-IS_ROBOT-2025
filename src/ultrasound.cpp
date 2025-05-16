#include <Arduino.h>
#include "ultrasound.h"

// HC-SR04 Pins
const int trigPin = 41;   
const int echoPin = 42;  

// Precision Parameters
const int NUM_SAMPLES = 5;   // Odd number for median filter (more samples = smoother)
const float TEMPERATURE = 25.0; // Manually set room temp in °C (adjust this!)
const float MIN_DISTANCE = 2.0;  // 2cm minimum
const float MAX_DISTANCE = 400.0; // 400cm maximum

// Pre-calculate speed of sound (cm/µs) at your set temperature
const float SPEED_OF_SOUND = (331.3 + (0.606 * TEMPERATURE)) * 100 / 1e6;

void setupUltrasound() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
}

float getPreciseDistance() {
  float samples[NUM_SAMPLES];
  int validSamples = 0;

  // Collect multiple samples
  for (int i = 0; i < NUM_SAMPLES; i++) {
    // Trigger HC-SR04
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    // Measure echo duration
    long duration = pulseIn(echoPin, HIGH, 25000); // 25ms timeout (~4m)

    // Calculate distance
    float distance = duration * SPEED_OF_SOUND / 2.0;

    // Validate sample
    if (distance >= MIN_DISTANCE && distance <= MAX_DISTANCE) {
      samples[validSamples] = distance;
      validSamples++;
    }
    delay(5); // Short delay between samples
  }

  // Error if no valid samples
  if (validSamples == 0) return -1.0;

  // Sort samples (simple bubble sort)
  for (int i = 0; i < validSamples - 1; i++) {
    for (int j = i + 1; j < validSamples; j++) {
      if (samples[i] > samples[j]) {
        float temp = samples[i];
        samples[i] = samples[j];
        samples[j] = temp;
      }
    }
  }

  // Return median (middle value)
  return samples[validSamples / 2];
}
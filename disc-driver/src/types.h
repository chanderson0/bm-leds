#ifndef types_h
#define types_h

#include "FastLED.h"

#define NUM_PIXELS 255
#define NUM_RINGS 10
#define DATAPIN 7
#define CLOCKPIN 14

#define BUTTON_PIN 22

// Rings
// 48, 43, 40, 32, 29, 24, 20. 12, 6, 1
const uint8_t kRingIndexMap[NUM_RINGS][2] = {
    {254, 254}, // 0 Center Point
    {248, 253}, // 1
    {236, 247}, // 2
    {216, 235}, // 3
    {192, 215}, // 4
    {164, 191}, // 5
    {132, 163}, // 6
    {92, 131},  // 7
    {48, 91},   // 8
    {0, 47},    // 9 Outer Ring
};

// Forward declaration
class LEDPanel;

typedef struct
{
  LEDPanel *panel;

  unsigned long now = 0;
  unsigned long sceneStart = 0;

  uint8_t pixelCoords[NUM_PIXELS][2];      // x, y => 0-255
  uint8_t pixelCoordsPolar[NUM_PIXELS][2]; // r, theta => 0-255
  float pixelCoordsf[NUM_PIXELS][2];       // x, y => -0.5->0.5
  float pixelCoordsPolarf[NUM_PIXELS][2];  // r, theta => 0-0.5, 0-2pi

  // Button data
  bool buttonPressed = false;
  bool buttonHeld = false;
  unsigned long buttonChangeTime = 0;

  bool buttonDidTap = false;
  bool buttonDidRelease = false;
  bool buttonDidPress = false;
  bool buttonDidHold = false;
} LEDContext;

#endif

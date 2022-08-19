#ifndef types_h
#define types_h

#include "FastLED.h"

#define NUM_PIXELS 255
#define NUM_RINGS 10
#define DATAPIN 7
#define CLOCKPIN 14

#define BUTTON_PIN 22

#define SCENE_TIME 21
#define PALETTE_TIME 13
#define PALETTE_BLEND_TIME_MS 10

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

  // CRGBPalette16 *curPalette;

  bool buttonState = false;
  bool buttonHold = false;
  unsigned long buttonDownTime = 0;
  bool buttonUpHandled = true;
  bool buttonDownHandled = true;
  bool buttonHoldHandled = true;

  // Remote data
  // uint8_t remoteDist = 127;
  // unsigned long remoteDistTime = 0;

  // bool button1State = false;
  // unsigned long button1DownTime = 0;
  // bool button1DownHandled = true;
  // unsigned long button1UpTime = 0;
  // bool button1UpHandled = true;

  // bool button2State = false;
  // unsigned long button2DownTime = 0;
  // bool button2DownHandled = true;
  // unsigned long button2UpTime = 0;
  // bool button2UpHandled = true;
} LEDContext;

#endif

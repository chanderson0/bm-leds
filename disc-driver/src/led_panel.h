#ifndef MemoryPanel_h
#define MemoryPanel_h

#include "FastLED.h"
#include "Adafruit_GFX.h"

#include "types.h"

class LEDPanel : public GFXcanvas16
{
public:
  LEDPanel(uint16_t width, uint16_t height) : GFXcanvas16(width, height){};

  uint16_t sample(float x, float y, bool blur = false);
  uint16_t get(uint16_t x, uint16_t y);
  void drawToScreen(CRGB *leds, LEDContext &context, bool blur = false);
  void debug();
};

#endif

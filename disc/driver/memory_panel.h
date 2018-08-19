#ifndef MemoryPanel_h
#define MemoryPanel_h

#include "FastLED.h"
#include "Adafruit_GFX.h"

#include "types.h"

class MemoryPanel : public Adafruit_GFX
{
public:
  MemoryPanel(uint16_t width, uint16_t height);
  virtual void drawPixel(int16_t x, int16_t y, uint16_t color) override;
  uint16_t sample(float x, float y, bool blur = false);
  uint16_t get(uint16_t x, uint16_t y);

  void drawToScreen(CRGB *leds, LEDContext &context, bool blur = false);
  void debug();

private:
  uint16_t **screen;
};

#endif

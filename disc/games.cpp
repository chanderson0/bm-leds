#include "games.h"

void SpinGame::start(unsigned long now)
{
  startTime = now;
  startAngle = random8();
  curAngle = startAngle;
  endAngle = random8();
}

void SpinGame::draw(CRGB *leds, LEDContext &context)
{
  curAngle += 10;

  for (uint8_t i = 0; i < NUM_PIXELS; ++i)
  {
    uint8_t r = context.pixelCoordsPolar[i][0];
    uint8_t t = context.pixelCoordsPolar[i][1];
    uint8_t v = t * 5 + sin8(r - context.elapsed / 10);
    leds[i] = ColorFromPalette(*context.curPalette, v);
  }
}

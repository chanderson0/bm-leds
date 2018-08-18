#include "patterns.h"

void Pattern0::draw(CRGB *leds, LEDContext &context)
{
  for (uint8_t i = 0; i < NUM_PIXELS; ++i)
  {
    uint8_t r = context.pixelCoordsPolar[i][0];
    uint8_t t = context.pixelCoordsPolar[i][1];
    uint8_t h = t - sin8(r + context.elapsed / 50) + context.elapsed / 10;

    leds[i] = ColorFromPalette(*context.curPalette, h);
  }
}

void Pattern1::draw(CRGB *leds, LEDContext &context)
{
  for (uint8_t i = 0; i < NUM_PIXELS; ++i)
  {
    uint8_t r = context.pixelCoordsPolar[i][0];
    uint8_t h = sin8(-r + context.elapsed / 5);
    leds[i] = ColorFromPalette(*context.curPalette, h);
  }
}

void Pattern2::draw(CRGB *leds, LEDContext &context)
{
  for (uint8_t i = 0; i < NUM_PIXELS; ++i)
  {
    uint8_t x = context.pixelCoords[i][0];
    uint8_t h = sin8(x + context.elapsed / 5);
    leds[i] = ColorFromPalette(*context.curPalette, h);
  }
}

void Pattern3::draw(CRGB *leds, LEDContext &context)
{
  for (uint8_t i = 0; i < NUM_PIXELS; ++i)
  {
    uint8_t x = context.pixelCoords[i][0];
    uint8_t y = context.pixelCoords[i][1];
    uint8_t dx = sin8(x + context.elapsed / 10);
    uint8_t dy = sin8(y * 2);
    leds[i] = ColorFromPalette(*context.curPalette, dx - dy);
  }
}

void Pattern4::draw(CRGB *leds, LEDContext &context)
{
  for (uint8_t i = 0; i < NUM_PIXELS; ++i)
  {
    uint8_t r = context.pixelCoordsPolar[i][0];
    uint8_t t = context.pixelCoordsPolar[i][1];
    uint8_t dr = sin8(r + context.elapsed / 10);
    uint8_t dt = sin8(t * 2 + context.elapsed / 15);
    leds[i] = ColorFromPalette(*context.curPalette, dr - dt);
  }
}

void Pattern5::draw(CRGB *leds, LEDContext &context)
{
  for (uint8_t i = 0; i < NUM_PIXELS; ++i)
  {
    uint8_t r = context.pixelCoordsPolar[i][0];
    uint8_t t = context.pixelCoordsPolar[i][1];
    uint8_t v = scale8(sin8(r + context.elapsed / 20), sin8(t * 3 - context.elapsed / 10));
    leds[i] = ColorFromPalette(*context.curPalette, v);
  }
}

void Pattern6::draw(CRGB *leds, LEDContext &context)
{
  for (uint8_t i = 0; i < NUM_PIXELS; ++i)
  {
    uint8_t r = context.pixelCoordsPolar[i][0];
    uint8_t t = context.pixelCoordsPolar[i][1];
    uint8_t v = t * 5 + sin8(r - context.elapsed / 10);
    leds[i] = ColorFromPalette(*context.curPalette, v);
  }
}

void Pattern7::draw(CRGB *leds, LEDContext &context)
{
  for (uint8_t i = 0; i < NUM_PIXELS; ++i)
  {
    uint8_t x = context.pixelCoords[i][0];
    uint8_t y = context.pixelCoords[i][1];
    uint8_t v = mul8(x, 3) + sin8(y + context.elapsed / 10);
    leds[i] = ColorFromPalette(*context.curPalette, v);
  }
}

void Pattern8::draw(CRGB *leds, LEDContext &context)
{
  for (uint8_t i = 0; i < NUM_PIXELS; ++i)
  {
    uint8_t r = context.pixelCoordsPolar[i][0];
    uint8_t t = context.pixelCoordsPolar[i][1];
    uint8_t v = t + r - context.elapsed / 20; // + context.elapsed / 30;
    leds[i] = ColorFromPalette(*context.curPalette, v);
  }
}

void Pattern9::draw(CRGB *leds, LEDContext &context)
{
  for (uint8_t i = 0; i < NUM_PIXELS; ++i)
  {
    uint8_t x = context.pixelCoords[i][0];
    uint8_t y = context.pixelCoords[i][1];
    uint8_t v = sin8(x / 2 + context.elapsed / 30) + sin8(y / 2 - context.elapsed / 25) - context.elapsed / 20;
    leds[i] = ColorFromPalette(*context.curPalette, v);
  }
}

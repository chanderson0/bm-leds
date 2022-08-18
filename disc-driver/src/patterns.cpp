#include "patterns.h"
#include "memory_panel.h"
#include "util.h"

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

void Pattern10::draw(CRGB *leds, LEDContext &context)
{
  MemoryPanel *p = context.sharedPanel;
  p->fillScreen(0);

  for (int i = 0; i < 20; ++i)
  {
    CRGB color = ColorFromPalette(*context.curPalette, sin8(context.elapsed / 15 + i * 5));

    float x = p->width() / 2.0;
    float y = p->height() / 2.0;
    float r = p->width() / (1.0 + i / 2.0);

    float angle = (float)context.elapsed / 3000.0f * M_PI * 2.0 + (float)i / 20.0 * 1.5;
    float x1 = x + cos(angle + M_PI * 0.0 / 3.0) * r;
    float y1 = y + sin(angle + M_PI * 0.0 / 3.0) * r;
    float x2 = x + cos(angle + M_PI * 2.0 / 3.0) * r;
    float y2 = y + sin(angle + M_PI * 2.0 / 3.0) * r;
    float x3 = x + cos(angle + M_PI * 4.0 / 3.0) * r;
    float y3 = y + sin(angle + M_PI * 4.0 / 3.0) * r;

    p->fillTriangle(x1, y1, x2, y2, x3, y3, convert888(color));
  }

  p->drawToScreen(leds, context, true);
}

void Pattern11::draw(CRGB *leds, LEDContext &context)
{
  MemoryPanel *p = context.sharedPanel;
  p->fillScreen(0);

  for (float t = 0; t < M_PI / 2.0; t += 0.02)
  {
    float rt = t + (float)context.elapsed / 500.0f;

    CRGB color = ColorFromPalette(*context.curPalette, context.elapsed / 15 + t * (255.0 / M_PI * 2.0));

    float x = sin(2.0 * rt + M_PI / 2.0 + (float)context.elapsed / 20000.0f);
    float y = sin(3.0 * rt);

    x *= 0.5 * 0.7;
    y *= 0.5 * 0.7;
    x += 0.5;
    y += 0.5;
    x *= p->width();
    y *= p->height();

    p->drawPixel(x, y, convert888(color));
  }

  p->drawToScreen(leds, context, true);
}
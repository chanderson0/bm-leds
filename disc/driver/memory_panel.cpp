#include "memory_panel.h"
#include "util.h"

MemoryPanel::MemoryPanel(uint16_t width, uint16_t height) : Adafruit_GFX(width, height)
{
  screen = (uint16_t **)malloc(sizeof(uint16_t *) * width);
  for (uint16_t x = 0; x < width; ++x)
  {
    screen[x] = (uint16_t *)malloc(sizeof(uint16_t) * height);
  }
}

void MemoryPanel::drawPixel(int16_t x, int16_t y, uint16_t color)
{
  if (x < 0 || x >= width())
    return;
  if (y < 0 || y >= height())
    return;
  screen[x][y] = color;
}

uint16_t MemoryPanel::sample(float x, float y, bool blur)
{
  uint16_t ax = x * width();
  uint16_t ay = y * height();

  if (blur)
  {
    uint16_t c0 = get(ax + 0, ay + 0);
    uint16_t cl = get(ax - 1, ay + 0);
    uint16_t cr = get(ax + 1, ay + 0);
    uint16_t ct = get(ax + 0, ay + 1);
    uint16_t cb = get(ax + 0, ay - 1);

    CRGB out;
    out += convert565(c0).nscale8_video(87);
    out += convert565(cl).nscale8_video(42);
    out += convert565(cr).nscale8_video(42);
    out += convert565(ct).nscale8_video(42);
    out += convert565(cb).nscale8_video(42);

    return convert888(out);
  }

  return get(ax, ay);
}

uint16_t MemoryPanel::get(uint16_t x, uint16_t y)
{
  if (x < 0 || x >= width())
    return 0;
  if (y < 0 || y >= height())
    return 0;
  return screen[x][y];
}

void MemoryPanel::drawToScreen(CRGB *leds, LEDContext &context, bool blur)
{
  for (uint8_t i = 0; i < NUM_PIXELS; ++i)
  {
    float x = context.pixelCoordsf[i][0] + 0.5;
    float y = context.pixelCoordsf[i][1] + 0.5;

    uint16_t color = sample(x, y, blur);
    leds[i] = convert565(color);
  }
}

void MemoryPanel::debug()
{
  char out[128];
  for (int y = 0; y < height(); ++y)
  {
    int outChar = 0;
    for (int x = 0; x < width(); ++x)
    {
      out[outChar++] = screen[x][y] > 0 ? '1' : '0';
    }
    out[outChar] = 0;
    Serial.println(out);
  }
}
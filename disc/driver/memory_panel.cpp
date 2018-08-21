#include "memory_panel.h"
#include "util.h"

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

  const uint16_t *buf = getBuffer();
  return buf[x + y * WIDTH];
}

void MemoryPanel::drawToScreen(CRGB *leds, LEDContext &context, bool blur)
{
  for (uint8_t i = 0; i < NUM_PIXELS; ++i)
  {
    float x = context.pixelCoordsf[i][0];
    float y = context.pixelCoordsf[i][1];

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
      out[outChar++] = get(x, y) > 0 ? '1' : '0';
    }
    out[outChar] = 0;
    Serial.println(out);
  }
}
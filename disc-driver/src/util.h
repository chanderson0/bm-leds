#ifndef util_h
#define util_h

#include "FastLED.h"

#define RUN_ONCE(runcode)     \
  {                           \
    static bool code_ran = 0; \
    if (!code_ran)            \
    {                         \
      \ 
        code_ran = 1;         \
      runcode;                \
    }                         \
  }

float easeInQuad(float t, float b, float c, float d);
float easeOutQuad(float t, float b, float c, float d);
float easeInOutQuad(float t, float b, float c, float d);

float angleDiff(float x, float y);

inline CRGB convert565(uint16_t src)
{
  return CRGB(
      ((((src >> 11) & 0x1F) * 527) + 23) >> 6,
      ((((src >> 5) & 0x3F) * 259) + 33) >> 6,
      (((src & 0x1F) * 527) + 23) >> 6);
}

inline uint16_t convert888(const CRGB &src)
{
  return ((src.r & 0b11111000) << 8) | ((src.g & 0b11111100) << 3) | (src.b >> 3);
}

inline float randFloat()
{
  return (float)random(100000) / 100000.0f;
}

#endif

float easeInQuad(float t, float b, float c, float d)
{
  if (d == 0)
  {
    return 0;
  }
  return c * (t /= d) * t + b;
}
float easeOutQuad(float t, float b, float c, float d)
{
  if (d == 0)
  {
    return 0;
  }
  return -c * (t /= d) * (t - 2) + b;
}

float easeInOutQuad(float t, float b, float c, float d)
{
  if (d == 0)
  {
    return 0;
  }
  if ((t /= d / 2) < 1)
  {
    return c / 2 * t * t + b;
  }

  return -c / 2 * ((--t) * (t - 2) - 1) + b;
}

float angleDiff(float x, float y)
{
  float arg;

  arg = fmod(y - x, M_PI * 2.0);
  if (arg < 0)
    arg = arg + M_PI * 2.0;
  if (arg > M_PI)
    arg = arg - M_PI * 2.0;

  return -arg;
}

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

inline float randFloat() {
  return (float)random(100000) / 100000.0f;
}

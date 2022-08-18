#include "util.h"

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

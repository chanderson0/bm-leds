float easeInQuad(float t, float b, float c, float d)
{
  return c * (t /= d) * t + b;
}
float easeOutQuad(float t, float b, float c, float d)
{
  return -c * (t /= d) * (t - 2) + b;
}

float easeInOutQuad(float t, float b, float c, float d)
{
  if ((t /= d / 2) < 1)
    return c / 2 * t * t + b;
  return -c / 2 * ((--t) * (t - 2) - 1) + b;
}

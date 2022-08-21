#include "patterns.h"
#include "led_panel.h"
#include "util.h"

CRGB backBuffer[NUM_PIXELS];

void PatternWhirlpool::draw(CRGB *leds, LEDContext &context, CRGBPalette16 *palette)
{
  for (uint8_t i = 0; i < NUM_PIXELS; ++i)
  {
    uint8_t r = context.pixelCoordsPolar[i][0];
    uint8_t t = context.pixelCoordsPolar[i][1];
    uint8_t h = t - sin8(r + context.now / 50) + context.now / 10;

    leds[i] = ColorFromPalette(*palette, h);
  }
}

void PatternConcentricRings::draw(CRGB *leds, LEDContext &context, CRGBPalette16 *palette)
{
  for (uint8_t i = 0; i < NUM_PIXELS; ++i)
  {
    uint8_t r = context.pixelCoordsPolar[i][0];
    uint8_t h = sin8(-r + context.now / 5);
    leds[i] = ColorFromPalette(*palette, h);
  }
}

void PatternStripe::draw(CRGB *leds, LEDContext &context, CRGBPalette16 *palette)
{
  for (uint8_t i = 0; i < NUM_PIXELS; ++i)
  {
    uint8_t x = context.pixelCoords[i][0];
    uint8_t h = sin8(x + context.now / 5);
    leds[i] = ColorFromPalette(*palette, h);
  }
}

void PatternDiamonds::draw(CRGB *leds, LEDContext &context, CRGBPalette16 *palette)
{
  for (uint8_t i = 0; i < NUM_PIXELS; ++i)
  {
    uint8_t x = context.pixelCoords[i][0];
    uint8_t y = context.pixelCoords[i][1];
    uint8_t dx = sin8(x + context.now / 10);
    uint8_t dy = sin8(y * 2);
    leds[i] = ColorFromPalette(*palette, dx - dy);
  }
}

void PatternWhirlpool2::draw(CRGB *leds, LEDContext &context, CRGBPalette16 *palette)
{
  for (uint8_t i = 0; i < NUM_PIXELS; ++i)
  {
    uint8_t r = context.pixelCoordsPolar[i][0];
    uint8_t t = context.pixelCoordsPolar[i][1];
    uint8_t dr = sin8(r + context.now / 10);
    uint8_t dt = sin8(t * 2 + context.now / 15);
    leds[i] = ColorFromPalette(*palette, dr - dt);
  }
}

void PatternWhirlpool3::draw(CRGB *leds, LEDContext &context, CRGBPalette16 *palette)
{
  for (uint8_t i = 0; i < NUM_PIXELS; ++i)
  {
    uint8_t r = context.pixelCoordsPolar[i][0];
    uint8_t t = context.pixelCoordsPolar[i][1];
    uint8_t v = scale8(sin8(r + context.now / 20), sin8(t * 3 - context.now / 10));
    leds[i] = ColorFromPalette(*palette, v);
  }
}

void PatternWavyArms::draw(CRGB *leds, LEDContext &context, CRGBPalette16 *palette)
{
  for (uint8_t i = 0; i < NUM_PIXELS; ++i)
  {
    uint8_t r = context.pixelCoordsPolar[i][0];
    uint8_t t = context.pixelCoordsPolar[i][1];
    uint8_t v = t * 5 + sin8(r - context.now / 10);
    leds[i] = ColorFromPalette(*palette, v);
  }
}

void PatternWaves::draw(CRGB *leds, LEDContext &context, CRGBPalette16 *palette)
{
  for (uint8_t i = 0; i < NUM_PIXELS; ++i)
  {
    uint8_t x = context.pixelCoords[i][0];
    uint8_t y = context.pixelCoords[i][1];
    uint8_t v = mul8(x, 3) + sin8(y + context.now / 10);
    leds[i] = ColorFromPalette(*palette, v);
  }
}

void PatternNautilus::draw(CRGB *leds, LEDContext &context, CRGBPalette16 *palette)
{
  for (uint8_t i = 0; i < NUM_PIXELS; ++i)
  {
    uint8_t r = context.pixelCoordsPolar[i][0];
    uint8_t t = context.pixelCoordsPolar[i][1];
    uint8_t v = t + r - context.now / 20; // + context.elapsed / 30;
    leds[i] = ColorFromPalette(*palette, v);
  }
}

void PatternSineHills::draw(CRGB *leds, LEDContext &context, CRGBPalette16 *palette)
{
  for (uint8_t i = 0; i < NUM_PIXELS; ++i)
  {
    uint8_t x = context.pixelCoords[i][0];
    uint8_t y = context.pixelCoords[i][1];
    uint8_t v = sin8(x / 2 + context.now / 30) + sin8(y / 2 - context.now / 25) - context.now / 20;
    leds[i] = ColorFromPalette(*palette, v);
  }
}

void PatternTriangles::draw(CRGB *leds, LEDContext &context, CRGBPalette16 *palette)
{
  LEDPanel *p = context.panel;
  p->fillScreen(0);

  for (int i = 0; i < 2; ++i)
  {
    CRGB color = ColorFromPalette(*palette, sin8(context.now / 15 + i * 10));

    float x = p->width() / 2.0;
    float y = p->height() / 2.0;
    float r = p->width() / (2.0 + i);

    float angle = sin((float)context.now / 10000.0f + (float)i * 1.0) * M_PI * 0.5;
    float x1 = x + cos(angle + M_PI * 0.0 / 3.0) * r;
    float y1 = y + sin(angle + M_PI * 0.0 / 3.0) * r;
    float x2 = x + cos(angle + M_PI * 2.0 / 3.0) * r;
    float y2 = y + sin(angle + M_PI * 2.0 / 3.0) * r;
    float x3 = x + cos(angle + M_PI * 4.0 / 3.0) * r;
    float y3 = y + sin(angle + M_PI * 4.0 / 3.0) * r;

    p->fillTriangle(x1, y1, x2, y2, x3, y3, convert888(color));
  }

  p->drawToScreen(leds, context, false);
}

void PatternLissajous::draw(CRGB *leds, LEDContext &context, CRGBPalette16 *palette)
{
  LEDPanel *p = context.panel;
  p->fillScreen(0);

  for (float t = 0; t < M_PI / 2.0; t += 0.02)
  {
    float rt = t + (float)context.now / 500.0f;

    CRGB color = ColorFromPalette(*palette, context.now / 15 + t * (255.0 / M_PI * 2.0));

    float x = sin(2.0 * rt + M_PI / 2.0 + (float)context.now / 20000.0f);
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

void PatternNoise::draw(CRGB *leds, LEDContext &context, CRGBPalette16 *palette)
{
  for (uint8_t i = 0; i < NUM_PIXELS; ++i)
  {
    uint8_t x = context.pixelCoords[i][0];
    uint8_t y = context.pixelCoords[i][1];

    uint8_t v = inoise8(x * 3, y * 3, context.now / 5) + context.now / 20;
    backBuffer[i] = ColorFromPalette(*palette, v);
  }

  EVERY_N_MILLISECONDS(5)
  {
    nblend(leds, backBuffer, NUM_PIXELS, 15);
  }
}

// void PatternRibbons::draw(CRGB *leds, LEDContext &context, CRGBPalette16 *palette)
// {
//   for (uint8_t i = 0; i < NUM_PIXELS; ++i)
//   {
//     uint8_t x = context.pixelCoords[i][0];
//     uint8_t y = context.pixelCoords[i][1];

//     uint8_t shiftAmt = inoise8(y * 2);
//     uint8_t v = inoise8(shiftAmt, x * 2 + context.now / 10);
//     leds[i] = ColorFromPalette(*palette, v);
//   }
// }

PatternGameOfLife::PatternGameOfLife()
{
  randomBoard();
}

void PatternGameOfLife::draw(CRGB *leds, LEDContext &context, CRGBPalette16 *palette)
{
  EVERY_N_MILLISECONDS(100)
  {
    int alive = updateBoard();
    if (alive == 0)
    {
      randomBoard();
    }
  }

  LEDPanel *p = context.panel;
  p->fillScreen(0);

  float w = p->width();
  float h = p->height();

  for (int8_t x = 0; x < PATTERN_GOL_SIZE; ++x)
  {
    for (int8_t y = 0; y < PATTERN_GOL_SIZE; ++y)
    {
      if (!board[x][y])
      {
        continue;
      }

      p->fillRect(
          (float)x / PATTERN_GOL_SIZE * w,
          (float)y / PATTERN_GOL_SIZE * h,
          w / PATTERN_GOL_SIZE,
          h / PATTERN_GOL_SIZE,
          0xFFFF);
    }
  }

  p->drawToScreen(leds, context, false);
}

void PatternGameOfLife::randomBoard()
{
  for (int8_t x = 0; x < PATTERN_GOL_SIZE; ++x)
  {
    for (int8_t y = 0; y < PATTERN_GOL_SIZE; ++y)
    {
      board[x][y] = random8() <= 127;
    }
  }
}

int PatternGameOfLife::updateBoard()
{
  for (int8_t x = 0; x < PATTERN_GOL_SIZE; ++x)
  {
    for (int8_t y = 0; y < PATTERN_GOL_SIZE; ++y)
    {
      uint8_t neighbors = 0;
      for (int8_t dx = -1; dx <= 1; ++dx)
      {
        for (int8_t dy = -1; dy <= 1; ++dy)
        {
          if (dx == 0 && dy == 0)
          {
            continue;
          }

          int8_t checkX = x + dx;
          int8_t checkY = y + dy;

          if (checkX < 0)
          {
            checkX = PATTERN_GOL_SIZE - 1;
          }
          else if (checkX >= PATTERN_GOL_SIZE)
          {
            checkX = 0;
          }

          if (checkY < 0)
          {
            checkY = PATTERN_GOL_SIZE - 1;
          }
          else if (checkY >= PATTERN_GOL_SIZE)
          {
            checkY = 0;
          }

          if (board[checkX][checkY])
          {
            neighbors++;
          }
        }
      }

      if (board[x][y])
      {
        nextBoard[x][y] = neighbors >= 2 && neighbors <= 3;
      }
      else
      {
        nextBoard[x][y] = neighbors == 3;
      }
    }
  }

  bool changedFromPrev = false;
  for (int8_t x = 0; x < PATTERN_GOL_SIZE; ++x)
  {
    for (int8_t y = 0; y < PATTERN_GOL_SIZE; ++y)
    {
      if (prevBoard[x][y] == nextBoard[x][y])
      {
        changedFromPrev = true;
        break;
      }
    }
    if (changedFromPrev)
    {
      break;
    }
  }
  if (!changedFromPrev)
  {
    return 0;
  }

  int alive = 0;
  bool didChange = false;
  for (int8_t x = 0; x < PATTERN_GOL_SIZE; ++x)
  {
    for (int8_t y = 0; y < PATTERN_GOL_SIZE; ++y)
    {
      if (board[x][y] != nextBoard[x][y])
      {
        didChange = true;
      }

      prevBoard[x][y] = board[x][y];
      board[x][y] = nextBoard[x][y];

      if (board[x][y])
      {
        alive++;
      }
    }
  }

  return didChange ? alive : 0;
}
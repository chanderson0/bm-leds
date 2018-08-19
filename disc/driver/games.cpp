#include "games.h"
#include "util.h"
#include "palettes.h"

#include <Fonts/FreeSansOblique12pt7b.h>

void SpinGame::start(LEDContext &context)
{
  startTime = context.elapsed;
  startAngle = curAngle;
  endAngle = ((float)random(0, 10000) / 10000.0 + random(5, 7)) * M_PI * 2.0f;
  duration = random(6000, 8000);

  palette = rainbow_gp;
}

void SpinGame::draw(CRGB *leds, LEDContext &context)
{
  bool inAnimation = context.elapsed - startTime < duration;
  if (!inAnimation)
  {
    if (context.remoteDist < 30 && context.elapsed - context.remoteDistTime < 200)
    {
      if (isClose)
      {
        microsCloseEnough += micros() - lastFrameTime;
      }
      if (microsCloseEnough > 2 * 1000 * 1000)
      {
        microsCloseEnough = 0;
        return start(context);
      }
      isClose = true;
    }
    else
    {
      isClose = false;
      microsCloseEnough = 0;
    }
  }
  lastFrameTime = micros();

  if (context.elapsed - context.button1DownTime > 200 && !context.button1DownHandled)
  {
    start(context);
    context.button1DownHandled = true;
  }

  float a = easeInOutQuad(constrain(context.elapsed - startTime, 0, duration), startAngle, endAngle, duration);
  curAngle = fmod(a, M_PI * 2.0f);

  uint8_t thresh = float((microsCloseEnough * 100) / (2 * 1000 * 1000)) / 100.f * 255;

  for (uint8_t i = 0; i < NUM_PIXELS; ++i)
  {
    uint8_t r = context.pixelCoordsPolar[i][0];
    uint8_t t = context.pixelCoordsPolar[i][1];
    float tf = context.pixelCoordsPolarf[i][1];

    float diff = abs(angleDiff(tf, curAngle));
    uint8_t b = 0;
    if (r < 50)
    {
      if (diff < 0.5)
      {
        b = 50;
      }
    }
    else if (r < 170)
    {
      if (diff < 0.2)
      {
        b = 50;
      }
    }
    else if (r < 190)
    {
      if (diff < 0.4)
      {
        b = 50;
      }
    }
    else if (r < 215)
    {
      if (diff < 0.2)
      {
        b = 50;
      }
    }
    else if (r < 235)
    {
      if (diff < 0.1)
      {
        b = 50;
      }
    }
    else
    {
      if (diff < 0.05)
      {
        b = 50;
      }
    }

    if (!inAnimation)
    {
      if (r < thresh)
      {
        b = 80;
      }
    }

    leds[i] = ColorFromPalette(palette, t, b);
  }
}

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

const String stringChoices[] = {
    "Burning Man",
    "Galapagos",
    "Not today",
};

const String magic8Ball[] = {
    "Signs point to yes.",
    "Yes.",
    "Reply hazy, try again.",
    "Without a doubt.",
    "My sources say no.",
    "As I see it, yes.",
    "You may rely on it.",
    "Concentrate and ask again.",
    "Outlook not so good.",
    "It is decidedly so.",
    "Better not tell you now.",
    "Very doubtful.",
    "Yes - definitely.",
    "It is certain.",
    "Cannot predict now.",
    "Most likely.",
    "Ask again later.",
    "My reply is no.",
    "Outlook good.",
    "Don't count on it."};

TextGame::TextGame()
{
  panel = new MemoryPanel(30, 30);
  panel->setFont(&FreeSansOblique12pt7b);

  panel->setTextWrap(false);
  panel->setCursor(0, 0);
  panel->setTextColor(0xffff);
  panel->setTextSize(1);
}

void TextGame::start(LEDContext &context)
{
  startTime = context.elapsed;
  setState(EightBallBoot, context);
  prevX = 0;
  loops = 0;
}

void TextGame::newPrediction(LEDContext &context)
{
  curString = magic8Ball[random(0, sizeof(magic8Ball) / sizeof(String))];
  setState(EightBallText, context);
}

void TextGame::setState(EightBallState newState, LEDContext &context)
{
  Serial.print("Eightball state: ");
  Serial.println(newState);

  state = newState;
  stateStartTime = context.elapsed;
}

void TextGame::drawPanel(CRGB *leds, LEDContext &context)
{
  for (uint8_t i = 0; i < NUM_PIXELS; ++i)
  {
    uint8_t x = context.pixelCoords[i][0];
    uint8_t y = context.pixelCoords[i][1];

    uint8_t v = map(panel->sample((float)x / 255.0f, (float)y / 255.0f), 0, 0xffff, 0, 255);

    leds[i] = CHSV(v, 100, v);
  }
}

void TextGame::draw(CRGB *leds, LEDContext &context)
{
  if (state == EightBallBoot)
  {
    panel->fillScreen(0);
    panel->setCursor(5, 20);
    panel->println("8");
    drawPanel(leds, context);

    if (context.elapsed - stateStartTime > 1000)
    {
      fade_video(leds, NUM_PIXELS, map(context.elapsed - stateStartTime, 1000, 5000, 0, 255));
    }

    if (context.elapsed - stateStartTime > 5000)
    {
      isClose = false;
      setState(EightBallIdle, context);
    }
  }
  else if (state == EightBallIdle)
  {
    unsigned long timeDelta = context.elapsed - closeEnoughStart;

    if (context.remoteDist < 30 && context.elapsed - context.remoteDistTime < 200)
    {
      if (!isClose)
      {
        closeEnoughStart = context.elapsed;
      }
      isClose = true;
    }
    else
    {
      closeEnoughStart += timeDelta * 0.01;
      isClose = false;
    }

    timeDelta = context.elapsed - closeEnoughStart;
    if (isClose && timeDelta > 4000)
    {
      newPrediction(context);
    }

    for (uint8_t i = 0; i < NUM_PIXELS; ++i)
    {
      uint8_t r = context.pixelCoordsPolar[i][0];

      uint8_t v = map(timeDelta, 0, 4000, 0, 255);
      uint8_t v2 = sin8(r * 2 + context.elapsed / 5);

      leds[i] = CHSV(max(v, v2), 100, max(v, v2));
    }
  }
  else if (state == EightBallText)
  {
    int16_t tx, ty;
    uint16_t tw, th;
    panel->getTextBounds((char *)(curString.c_str()), 0, 0, &tx, &ty, &tw, &th);

    int16_t x = (panel->width() + 50) - (((context.elapsed - stateStartTime) / 25) % (tw + panel->width() + 100));
    if (x > prevX)
    {
      if (++loops >= 2)
      {
        isClose = false;
        setState(EightBallIdle, context);
      }
    }
    prevX = x;

    panel->fillScreen(0);
    panel->setCursor(x, 20);
    panel->println(curString.c_str());

    drawPanel(leds, context);
  }

  if (context.elapsed - context.button1DownTime > 200 && !context.button1DownHandled)
  {
    start(context);
    context.button1DownHandled = true;
  }
}

Starfish::Starfish()
{
  panel = new MemoryPanel(32, 32);
  palette = Coral_reef_gp;
}

void Starfish::start(LEDContext &context)
{
  startTime = context.elapsed;
  smoothValue = 127;
}

void Starfish::draw(CRGB *leds, LEDContext &context)
{
  float dist = 0.5f;
  if (context.elapsed - context.remoteDistTime < 200)
  {
    uint8_t realDist = map(constrain(context.remoteDist, 10, 200), 10, 200, 0, 255);
    dist = 1.0 - (float)(realDist) / 255.0f;
  }
  smoothValue += (dist - smoothValue) * 0.2;

  const uint16_t centerX = panel->width() / 2;
  const uint16_t centerY = panel->height() / 2;

  panel->fillScreen(0);

  static const int numArms = 7;
  for (int i = 0; i < numArms; ++i)
  {
    float angle1 = (float)i / numArms * M_PI * 2.0f + smoothValue * M_PI;

    CRGB color2 = ColorFromPalette(palette, (float)context.elapsed * 0.1f + 15 + i * 255 / (numArms + 2));
    const uint16_t color565_2 = convert888(color2);

    int16_t x1 = centerX + cos(angle1) * panel->width() / 4;
    int16_t y1 = centerY + sin(angle1) * panel->width() / 4;
    panel->drawLine(centerX, centerY, x1, y1, color565_2);

    CRGB color3 = ColorFromPalette(palette, (float)context.elapsed * 0.1f + 30 + i * 255 / (numArms + 2));
    const uint16_t color565_3 = convert888(color3);

    float angle2 = angle1 + (smoothValue * 1.5 + 0.5) * M_PI * 2.0;
    int16_t x2 = x1 + cos(angle2) * panel->width() / 5;
    int16_t y2 = y1 + sin(angle2) * panel->width() / 5;
    panel->drawLine(x1, y1, x2, y2, color565_3);
  }

  for (uint8_t i = 0; i < NUM_PIXELS; ++i)
  {
    float x = context.pixelCoordsf[i][0] + 0.5;
    float y = context.pixelCoordsf[i][1] + 0.5;

    uint16_t color = panel->sample(x, y, true);
    leds[i] = convert565(color);
  }
}
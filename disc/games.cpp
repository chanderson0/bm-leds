#include "games.h"
#include "util.h"

#include <Fonts/FreeSansOblique12pt7b.h>

void SpinGame::start(LEDContext &context)
{
  startTime = context.elapsed;
  startAngle = (float)random(0, 10000) / 10000.0 * M_PI * 2.0f;
  endAngle = ((float)random(0, 10000) / 10000.0 + random(5, 10)) * M_PI * 2.0f;
  duration = random(5000, 10000);

  Serial.println(startAngle);
  Serial.println(endAngle);
}

void SpinGame::draw(CRGB *leds, LEDContext &context)
{
  if (context.elapsed - context.button1DownTime > 200 && !context.button1DownHandled)
  {
    start(context);
    context.button1DownHandled = true;
  }

  float a = easeOutQuad(constrain(context.elapsed - startTime, 0, duration), startAngle, endAngle, duration);
  uint8_t nowAngle = fmod(a, M_PI * 2.0f) / (M_PI * 2.0f) * 255;

  for (uint8_t i = 0; i < NUM_PIXELS; ++i)
  {
    uint8_t r = context.pixelCoordsPolar[i][0];
    uint8_t t = context.pixelCoordsPolar[i][1];

    int8_t dt = abs8(int8_t(t) - int8_t(nowAngle));
    // dt = scale8(dt, 255 - r);
    uint8_t v = dt < 15 ? 50 : 0;

    leds[i] = CHSV(0, 0, v);
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

uint16_t MemoryPanel::sample(float x, float y)
{
  uint16_t ax = x * width();
  uint16_t ay = y * width();

  uint16_t c0 = get(ax + 0, ay + 0);
  uint16_t cl = get(ax - 1, ay + 0);
  uint16_t cr = get(ax + 1, ay + 0);
  uint16_t ct = get(ax + 0, ay + 1);
  uint16_t cb = get(ax + 0, ay - 1);

  return c0 * 0.5 + cl * 0.125 + cr * 0.125 + ct * 0.125 + cb * 0.125;
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
  char out[1024];
  Serial.println("hello");
  Serial.println(width());
  Serial.println(height());
  delay(100);

  for (int y = 0; y < height(); ++y)
  {
    int outChar = 0;
    for (int x = 0; x < width(); ++x)
    {
      out[outChar++] = screen[x][y] > 0 ? '1' : '0';
    }
    out[outChar] = 0;
    Serial.println(out);
    delay(20);
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
  curString = magic8Ball[random(0, sizeof(magic8Ball) / sizeof(String))];
}

void TextGame::draw(CRGB *leds, LEDContext &context)
{
  if (context.elapsed - context.button1DownTime > 200 && !context.button1DownHandled)
  {
    start(context);
    context.button1DownHandled = true;
  }

  int16_t tx, ty;
  uint16_t tw, th;
  panel->getTextBounds((char *)(curString.c_str()), 0, 0, &tx, &ty, &tw, &th);

  int16_t x = (panel->width() + 50) - (((context.elapsed - startTime) / 25) % (tw + panel->width() + 100));

  panel->fillScreen(0);
  panel->setCursor(x, 20);
  panel->println(curString.c_str());

  for (uint8_t i = 0; i < NUM_PIXELS; ++i)
  {
    uint8_t x = context.pixelCoords[i][0];
    uint8_t y = context.pixelCoords[i][1];

    uint8_t v = map(panel->sample((float)x / 255.0f, (float)y / 255.0f), 0, 0xffff, 0, 255);

    leds[i] = CHSV(0, 0, v);
  }
}

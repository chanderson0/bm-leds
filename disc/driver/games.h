#ifndef Games_h
#define Games_h

#include "FastLED.h"
#include "types.h"
#include "Adafruit_GFX.h"

class Game
{
public:
  Game(){};
  virtual void start(LEDContext &context)
  {
    startTime = context.elapsed;
  };
  virtual void draw(CRGB *leds, LEDContext &context){};

protected:
  unsigned long startTime;
};

class SpinGame : public Game
{
public:
  SpinGame(){};
  void start(LEDContext &context) override;
  void draw(CRGB *leds, LEDContext &context) override;

private:
  float startAngle, curAngle, endAngle;
  uint16_t duration;
};

class MemoryPanel : public Adafruit_GFX
{
public:
  MemoryPanel(uint16_t width, uint16_t height);
  virtual void drawPixel(int16_t x, int16_t y, uint16_t color) override;
  uint16_t sample(float x, float y);
  uint16_t get(uint16_t x, uint16_t y);
  void debug();

private:
  uint16_t **screen;
};

class TextGame : public Game
{
public:
  TextGame();
  void start(LEDContext &context) override;
  void draw(CRGB *leds, LEDContext &context) override;

private:
  MemoryPanel *panel;
  String curString;
};

#endif

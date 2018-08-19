#ifndef Games_h
#define Games_h

#include "FastLED.h"
#include "Adafruit_GFX.h"

#include "types.h"

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
  CRGBPalette16 palette;

  float startAngle = 0, curAngle = 0, endAngle;
  uint16_t duration;

  bool isClose = false;
  uint32_t microsCloseEnough = 0;
  unsigned long lastFrameTime = 0;
};

class MemoryPanel : public Adafruit_GFX
{
public:
  MemoryPanel(uint16_t width, uint16_t height);
  virtual void drawPixel(int16_t x, int16_t y, uint16_t color) override;
  uint16_t sample(float x, float y, bool blur = false);
  uint16_t get(uint16_t x, uint16_t y);
  void debug();

private:
  uint16_t **screen;
};

enum EightBallState
{
  EightBallBoot,
  EightBallIdle,
  EightBallText
};

class TextGame : public Game
{
public:
  TextGame();
  void start(LEDContext &context) override;
  void draw(CRGB *leds, LEDContext &context) override;

private:
  void setState(EightBallState newState, LEDContext &context);
  void newPrediction(LEDContext &context);
  void drawPanel(CRGB *leds, LEDContext &context);

  EightBallState state;
  unsigned long stateStartTime;

  unsigned long closeEnoughStart;
  bool isClose;

  int prevX;
  int loops;

  MemoryPanel *panel;
  String curString;

  CRGBPalette16 palette;
};

class Starfish : public Game
{
public:
  Starfish();
  void start(LEDContext &context) override;
  void draw(CRGB *leds, LEDContext &context) override;

private:
  float smoothValue;
  MemoryPanel *panel;
  CRGBPalette16 palette;
};

#endif

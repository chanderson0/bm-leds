#ifndef Games_h
#define Games_h

#include "FastLED.h"

#include "types.h"
#include "memory_panel.h"
#include "util.h"
#include "math3d.h"

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

enum EightBallState
{
  EightBallBoot,
  EightBallIdle,
  EightBallText
};

class EightballGame : public Game
{
public:
  EightballGame();
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
  CRGBPalette16 palette;
};

class RingGame : public Game
{
public:
  RingGame();
  void start(LEDContext &context) override;
  void draw(CRGB *leds, LEDContext &context) override;

private:
  vec3_t ball, ballVel;
  float playerAngle;
  int score;

  unsigned long stateStartTime;
};

// class Eyes : public Game
// {
// public:
//   Eyes();
//   void start(LEDContext &context) override;
//   void draw(CRGB *leds, LEDContext &context) override;
// };

#endif

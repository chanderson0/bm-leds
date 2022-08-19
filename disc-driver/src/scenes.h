#ifndef Games_h
#define Games_h

#include "FastLED.h"

#include "patterns.h"
#include "types.h"
#include "led_panel.h"
#include "util.h"
#include "math3d.h"

class Scene
{
public:
  Scene(){};
  virtual void start(LEDContext &context)
  {
    startTime = context.now;
  };
  virtual void draw(CRGB *leds, LEDContext &context){};

protected:
  unsigned long startTime;
};

class PatternScene : public Scene
{
public:
  PatternScene(const int maxPatterns, const TProgmemRGBGradientPalettePtr palettes[], const int numPalettes);
  void addPattern(Pattern *pattern);

  void start(LEDContext &context) override;
  void draw(CRGB *leds, LEDContext &context) override;

  void advancePattern(LEDContext &context);
  void advancePalette(LEDContext &context);

private:
  CRGB prevLeds[NUM_PIXELS];

  Pattern **patterns;
  int numPatterns = 0;
  uint8_t curPattern = 0;
  uint8_t prevPattern = 0;
  uint8_t patternAmt = 255;

  const TProgmemRGBGradientPalettePtr *palettes;
  int numPalettes = 0;
  uint8_t curPaletteIdx = 0;
  CRGBPalette16 curPalette;
  CRGBPalette16 targetPalette;
};

class SpinGameScene : public Scene
{
public:
  SpinGameScene(){};
  void start(LEDContext &context) override;
  void draw(CRGB *leds, LEDContext &context) override;

  void startSpin(LEDContext &context);

private:
  CRGBPalette16 palette;

  bool started = false;

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

class EightballScene : public Scene
{
public:
  EightballScene();
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

  LEDPanel *panel;
  String curString;

  CRGBPalette16 palette;
};

class StarfishScene : public Scene
{
public:
  StarfishScene();
  void start(LEDContext &context) override;
  void draw(CRGB *leds, LEDContext &context) override;

private:
  float smoothValue;
  CRGBPalette16 palette;
};

class RingGameScene : public Scene
{
public:
  RingGameScene();
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

#ifndef Games_h
#define Games_h

#include "FastLED.h"
#include "types.h"

class Game
{
public:
  Game(){};
  virtual void start(unsigned long now)
  {
    startTime = now;
  };
  virtual void draw(CRGB *leds, LEDContext &context){};
  virtual void onWhitePressed(){};
  virtual void onBlackPressed(){};

protected:
  unsigned long startTime;
};

class SpinGame : public Game
{
public:
  SpinGame();
  void start(unsigned long now) override;
  void draw(CRGB *leds, LEDContext &context) override;

private:
  uint8_t startAngle, curAngle, endAngle;
};

#endif

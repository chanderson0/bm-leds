#ifndef Patterns_h
#define Patterns_h

#include "FastLED.h"
#include "types.h"

#define PATTERN_CLASS(n)                                                         \
  class Pattern##n : public Pattern                                              \
  {                                                                              \
  public:                                                                        \
    void draw(CRGB *leds, LEDContext &context, CRGBPalette16 *palette) override; \
  };

class Pattern
{
public:
  Pattern(){};
  virtual void draw(CRGB *leds, LEDContext &context, CRGBPalette16 *palette) { Serial.println(context.now); };
};

PATTERN_CLASS(Whirlpool)
PATTERN_CLASS(ConcentricRings)
PATTERN_CLASS(Stripe)
PATTERN_CLASS(Diamonds)
PATTERN_CLASS(Whirlpool2)
PATTERN_CLASS(Whirlpool3)
PATTERN_CLASS(WavyArms)
PATTERN_CLASS(Waves)
PATTERN_CLASS(Nautilus)
PATTERN_CLASS(SineHills)
PATTERN_CLASS(Triangles)
PATTERN_CLASS(Lissajous)
PATTERN_CLASS(Noise)
// PATTERN_CLASS(Ribbons)

#define PATTERN_GOL_SIZE 16

class PatternGameOfLife : public Pattern
{
public:
  PatternGameOfLife();
  void draw(CRGB *leds, LEDContext &context, CRGBPalette16 *palette) override;

protected:
  int updateBoard();
  void randomBoard();

  bool board[PATTERN_GOL_SIZE][PATTERN_GOL_SIZE];
  bool nextBoard[PATTERN_GOL_SIZE][PATTERN_GOL_SIZE];
  bool prevBoard[PATTERN_GOL_SIZE][PATTERN_GOL_SIZE];
};

#endif

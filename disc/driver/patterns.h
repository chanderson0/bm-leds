#ifndef Patterns_h
#define Patterns_h

#include "FastLED.h"
#include "types.h"

#define PATTERN_CLASS_N(n)                                       \
  class Pattern##n : public Pattern                              \
  {                                                              \
  public:                                                        \
    void draw(CRGB *leds, LEDContext &context) override; \
  };

class Pattern
{
public:
  Pattern(){};
  virtual void draw(CRGB *leds, LEDContext &context) { Serial.println(context.elapsed); };
};

PATTERN_CLASS_N(0)
PATTERN_CLASS_N(1)
PATTERN_CLASS_N(2)
PATTERN_CLASS_N(3)
PATTERN_CLASS_N(4)
PATTERN_CLASS_N(5)
PATTERN_CLASS_N(6)
PATTERN_CLASS_N(7)
PATTERN_CLASS_N(8)
PATTERN_CLASS_N(9)
PATTERN_CLASS_N(10)
PATTERN_CLASS_N(11)

#endif

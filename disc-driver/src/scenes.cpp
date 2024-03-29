#define MATH_3D_IMPLEMENTATION

#include "scenes.h"
#include "palettes.h"
#include "patterns.h"

#include <Fonts/FreeSansOblique12pt7b.h>

const uint8_t kPatternTimeS = 21;
// const uint16_t kPatternTimeS = 1000; // used for debugging patterns
const uint16_t kPaletteTimeS = 13;
const uint16_t kPaletteBlendTimeMs = 10;

PatternScene::PatternScene(const int maxPatterns, const TProgmemRGBGradientPalettePtr palettes[], const int numPalettes)
{
  patterns = (Pattern **)malloc(sizeof(Pattern *) * maxPatterns);
  numPatterns = 0;

  this->palettes = palettes;
  this->numPalettes = numPalettes;
}

void PatternScene::addPattern(Pattern *pattern)
{
  patterns[numPatterns++] = pattern;
}

void PatternScene::start(LEDContext &context)
{
  curPalette = (CRGBPalette16)palettes[0];
  targetPalette = (CRGBPalette16)palettes[0];

  curPaletteIdx = 0;
  curPattern = 0;
  patternAmt = 0;

  shouldRotatePatterns = true;
}

void PatternScene::advancePattern(LEDContext &context)
{
  prevPattern = curPattern;
  curPattern = addmod8(curPattern, 1, numPatterns);
  patternAmt = 0;

  context.sceneStart = context.now;

  Serial.print("Advancing pattern from ");
  Serial.print(prevPattern);
  Serial.print(" to ");
  Serial.println(curPattern);
}

void PatternScene::advancePalette(LEDContext &context)
{
  curPaletteIdx = addmod8(curPaletteIdx, 1, numPalettes);
  targetPalette = (CRGBPalette16)palettes[curPaletteIdx];

  Serial.print("Advancing palette to: ");
  Serial.println(curPaletteIdx);
}

void PatternScene::draw(CRGB *leds, LEDContext &context)
{
  if (context.buttonDidTap)
  {
    shouldRotatePatterns = !shouldRotatePatterns;
  }

  EVERY_N_SECONDS(kPatternTimeS)
  {
    if (shouldRotatePatterns)
    {
      advancePattern(context);
    }
  }

  EVERY_N_SECONDS(kPaletteTimeS)
  {
    advancePalette(context);
  }

  EVERY_N_MILLISECONDS(kPaletteBlendTimeMs)
  {
    nblendPaletteTowardPalette(curPalette, targetPalette, 64);
  }

  patterns[curPattern]->draw(leds, context, &curPalette);

  if (patternAmt != 255)
  {
    patterns[prevPattern]->draw(prevLeds, context, &curPalette);

    EVERY_N_MILLISECONDS(10)
    {
      patternAmt = qadd8(patternAmt, 8);
    }

    nblend(leds, prevLeds, NUM_PIXELS, 255 - patternAmt);
  }
}

void SpinGameScene::start(LEDContext &context)
{
  started = false;
}

void SpinGameScene::startSpin(LEDContext &context)
{
  started = true;

  startTime = context.now;
  startAngle = curAngle;
  endAngle = ((float)random(0, 10000) / 10000.0 + random(5, 7)) * M_PI * 2.0f;
  duration = random(4000, 10000);
}

void SpinGameScene::draw(CRGB *leds, LEDContext &context)
{
  bool inAnimation = started && context.now - startTime < duration;
  if (!inAnimation)
  {
    if (context.buttonDidTap)
    {
      startSpin(context);
    }
  }

  float a = easeInOutQuad(constrain(context.now - startTime, 0, duration), startAngle, endAngle, duration);
  curAngle = fmod(a, M_PI * 2.0f);

  uint8_t thresh = float((microsCloseEnough * 100) / (2 * 1000 * 1000)) / 100.f * 255;

  for (uint8_t i = 0; i < NUM_PIXELS; ++i)
  {
    uint8_t r = context.pixelCoordsPolar[i][0];
    uint8_t t = context.pixelCoordsPolar[i][1];
    float tf = context.pixelCoordsPolarf[i][1];

    float diff = abs(angleDiff(tf, curAngle));
    uint8_t b = 0;

    uint8_t on = 255;
    if (r < 50)
    {
      if (diff < 0.5)
      {
        b = on;
      }
    }
    else if (r < 170)
    {
      if (diff < 0.2)
      {
        b = on;
      }
    }
    else if (r < 190)
    {
      if (diff < 0.4)
      {
        b = on;
      }
    }
    else if (r < 215)
    {
      if (diff < 0.2)
      {
        b = on;
      }
    }
    else if (r < 235)
    {
      if (diff < 0.1)
      {
        b = on;
      }
    }
    else
    {
      if (diff < 0.05)
      {
        b = on;
      }
    }

    if (!inAnimation)
    {
      if (r < thresh)
      {
        b = on;
      }
    }

    leds[i] = CHSV(0, 255, b);
  }
}

const String magic8Ball[] = {
    "Signs point to yes.",
    "Yes.",
    // "Reply hazy, try again.",
    "Without a doubt.",
    "My sources say no.",
    "As I see it, yes.",
    "You may rely on it.",
    "Concentrate and ask again.",
    "Outlook not so good.",
    "It is decidedly so.",
    // "Better not tell you now.",
    "Very doubtful.",
    "Yes - definitely.",
    "It is certain.",
    "Cannot predict now.",
    "Most likely.",
    // "Ask again later.",
    "My reply is no.",
    "Outlook good.",
    "Don't count on it."};

EightballScene::EightballScene()
{
  panel = new LEDPanel(30, 30);
  panel->setRotation(1);

  panel->setFont(&FreeSansOblique12pt7b);
  panel->setTextWrap(false);
  panel->setCursor(0, 0);
  panel->setTextColor(0xffff);
  panel->setTextSize(1);
}

void EightballScene::start(LEDContext &context)
{
  startTime = context.now;
  setState(EightBallBoot, context);
  prevX = 0;
  loops = 0;
}

void EightballScene::newPrediction(LEDContext &context)
{
  curString = magic8Ball[random(0, sizeof(magic8Ball) / sizeof(String))];
  // closeEnoughStart = 0;
  setState(EightBallText, context);
}

void EightballScene::setState(EightBallState newState, LEDContext &context)
{
  Serial.print("Eightball state: ");
  Serial.println(newState);

  state = newState;
  stateStartTime = context.now;
}

void EightballScene::drawPanel(CRGB *leds, LEDContext &context)
{
  for (uint8_t i = 0; i < NUM_PIXELS; ++i)
  {
    uint8_t x = context.pixelCoords[i][0];
    uint8_t y = context.pixelCoords[i][1];

    uint8_t v = map(panel->sample((float)x / 255.0f, (float)y / 255.0f), 0, 0xffff, 0, 255);

    leds[i] = CHSV(v, 100, v);
  }
}

void EightballScene::draw(CRGB *leds, LEDContext &context)
{
  if (state == EightBallBoot)
  {
    panel->fillScreen(0);
    panel->setCursor(7, 22);
    context.panel->setTextColor(0xFFFF);
    panel->println("8");
    panel->drawToScreen(leds, context, true);

    if (context.now - stateStartTime > 500)
    {
      fade_video(leds, NUM_PIXELS, map(context.now - stateStartTime, 1000, 2000, 0, 255));
    }

    if (context.now - stateStartTime > 750)
    {
      setState(EightBallIdle, context);
    }
  }
  else if (state == EightBallIdle)
  {
    if (context.buttonDidTap)
    {
      loops = 0;
      newPrediction(context);
    }

    panel->fillScreen(0);
    panel->setCursor(7, 22);
    panel->setTextColor(0xFFFF);
    panel->println("?");
    panel->drawToScreen(leds, context, true);
  }
  else if (state == EightBallText)
  {
    int16_t tx, ty;
    uint16_t tw, th;
    panel->getTextBounds((char *)(curString.c_str()), 0, 0, &tx, &ty, &tw, &th);

    int16_t x = (panel->width() + 50) - (((context.now - stateStartTime) / 25) % (tw + panel->width() + 100));
    if (++loops >= 2 && x > prevX)
    {
      setState(EightBallIdle, context);
    }
    prevX = x;

    panel->fillScreen(0);
    panel->setCursor(x, 20);
    panel->setTextColor(0xFFFF);
    panel->println(curString.c_str());
    panel->drawToScreen(leds, context, true);
  }
}

// StarfishScene::StarfishScene()
// {
//   palette = Coral_reef_gp;
// }

// void StarfishScene::start(LEDContext &context)
// {
//   startTime = context.now;
//   smoothValue = 127;
// }

// void StarfishScene::draw(CRGB *leds, LEDContext &context)
// {
//   LEDPanel *panel = context.panel;

//   float dist = 0.5f;
//   // if (context.elapsed - context.remoteDistTime < 200)
//   // {
//   //   uint8_t realDist = map(constrain(context.remoteDist, 10, 200), 10, 200, 0, 255);
//   //   dist = 1.0 - (float)(realDist) / 255.0f;
//   // }
//   smoothValue += (dist - smoothValue) * 0.2;

//   const uint16_t centerX = panel->width() / 2;
//   const uint16_t centerY = panel->height() / 2;

//   panel->fillScreen(0);

//   static const int numArms = 7;
//   for (int i = 0; i < numArms; ++i)
//   {
//     float angle1 = (float)i / numArms * M_PI * 2.0f + smoothValue * M_PI;

//     CRGB color2 = ColorFromPalette(palette, (float)context.now * 0.1f + 15 + i * 255 / (numArms + 2));
//     const uint16_t color565_2 = convert888(color2);

//     int16_t x1 = centerX + cos(angle1) * panel->width() / 4;
//     int16_t y1 = centerY + sin(angle1) * panel->width() / 4;
//     panel->drawLine(centerX, centerY, x1, y1, color565_2);

//     CRGB color3 = ColorFromPalette(palette, (float)context.now * 0.1f + 30 + i * 255 / (numArms + 2));
//     const uint16_t color565_3 = convert888(color3);

//     float angle2 = angle1 + (smoothValue * 1.5 + 0.5) * M_PI * 2.0;
//     int16_t x2 = x1 + cos(angle2) * panel->width() / 5;
//     int16_t y2 = y1 + sin(angle2) * panel->width() / 5;
//     panel->drawLine(x1, y1, x2, y2, color565_3);
//   }

//   panel->drawToScreen(leds, context, true);
// }

// RingGameScene::RingGameScene()
// {
// }

// void RingGameScene::start(LEDContext &context)
// {
//   startTime = context.now;

//   score = 0;
//   playerAngle = 0;
//   ball = vec3(0, 0, 0);
//   float angle = randFloat() * M_PI * 2.0;
//   const float speed = 0.001;
//   ballVel = vec3(cos(angle) * speed, sin(angle) * speed, 0);
// }

// void RingGameScene::draw(CRGB *leds, LEDContext &context)
// {
//   LEDPanel *p = context.panel;

//   // playerAngle = float(context.remoteDist) / 255 * M_PI * 2.0 * 2.0;

//   const int difficulty = (context.now - startTime) / 10000;
//   float speedModifier = float(difficulty) * 0.2 + 1.0;
//   ball = v3_add(ball, v3_muls(ballVel, speedModifier));

//   float ballAngle = atan2(ball.y, ball.x);
//   const float playerBallDiff = abs(angleDiff(ballAngle, playerAngle));

//   const float dist = v3_length(ball);
//   if (dist > 0.5)
//   {
//     if (playerBallDiff < M_PI * 0.3)
//     {
//       vec3_t norm = v3_norm(v3_muls(ball, -1));
//       norm = m4_mul_dir(m4_rotation_z(randFloat() * 0.1 - 0.05), norm);
//       norm = v3_norm(norm);

//       vec3_t refl = v3_sub(ballVel, v3_muls(norm, 2.0f * v3_dot(ballVel, norm)));
//       ballVel = refl;
//       score++;
//     }
//     else
//     {
//       return start(context);
//     }
//   }

//   p->fillScreen(0);
//   p->fillCircle((ball.x + 0.5) * p->width(), (ball.y + 0.5) * p->height(), p->width() / 15, convert888(CHSV(score * 50, qadd8(50, qmul8(difficulty, 75)), 255)));
//   p->drawToScreen(leds, context, false);

//   for (uint8_t i = 0; i < NUM_PIXELS; ++i)
//   {
//     const uint8_t r = context.pixelCoordsPolar[i][0];
//     const float t = context.pixelCoordsPolarf[i][1];
//     const float diff = abs(angleDiff(t, playerAngle));

//     if (r > 225 && diff < M_PI * 0.25)
//     {
//       leds[i] = CRGB::White;
//     }
//   }
// }

Eyes::Eyes()
{
}

void Eyes::start(LEDContext &context)
{
  eyeColor = CRGB(255, 255, 255);
  pupilColor = CRGB(0, 0, 0);
  nTaps = 0;
}

void Eyes::draw(CRGB *leds, LEDContext &context)
{
  if (context.buttonDidTap)
  {
    if (++nTaps % 4 == 0)
    {
      eyeColor = CRGB(255, 255, 255);
      pupilColor = CRGB(0, 0, 0);
    }
    else
    {
      eyeColor = CHSV(random8(), 255, 255);
      pupilColor = CHSV(random8(), 255, 255);
    }
  }

  LEDPanel *p = context.panel;

  const float w = p->width();
  const float h = p->height();
  const float lX = w * 1.0 / 4.0;
  const float rX = w * 3.0 / 4.0;
  const float cY = h / 2.0;
  const float largeR = w / 4.0;
  const float smallR = w / 10.0;

  p->fillScreen(0);

  // Eyes
  p->fillCircle(lX, cY, largeR, convert888(eyeColor));
  p->fillCircle(rX, cY, largeR, convert888(eyeColor));

  // Pupils
  float eyeMoveXAmt = (float)(inoise8(context.now * 0.2)) / 255.f;
  float eyeMoveYAmt = (float)(inoise8(context.now * 0.1 + 4.5)) / 255.f;
  float eyeMovementX = (eyeMoveXAmt - 0.5) * 2.0 * smallR * 1.0;
  float eyeMovementY = (eyeMoveYAmt - 0.5) * 2.0 * smallR * 0.8;
  p->fillCircle(lX + eyeMovementX, cY + eyeMovementY + smallR, smallR, convert888(pupilColor));
  p->fillCircle(rX + eyeMovementX, cY + eyeMovementY + smallR, smallR, convert888(pupilColor));

  // Blink
  uint8_t blinkAmt = inoise8(context.now * 0.35 + 20);
  if (blinkAmt > 200)
  {
    uint8_t blinkKind = blinkAmt > 225 ? 0 : 2;
    p->fillCircle(lX, cY - blinkKind, largeR, 0); // Eyelids are always black
    p->fillCircle(rX, cY - blinkKind, largeR, 0);
  }

  p->drawToScreen(drawBuffer, context, false);

  // Blend so we don't have as many flashing artifacts
  EVERY_N_MILLISECONDS(5)
  {
    nblend(leds, drawBuffer, NUM_PIXELS, 15);
  }
}

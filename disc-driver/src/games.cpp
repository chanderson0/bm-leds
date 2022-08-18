#define MATH_3D_IMPLEMENTATION

#include "games.h"
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

// const String stringChoices[] = {
//     "Burning Man",
//     "Galapagos",
//     "Not today",
// };

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

EightballGame::EightballGame()
{
  panel = new MemoryPanel(30, 30);
  panel->setRotation(1);

  panel->setFont(&FreeSansOblique12pt7b);
  panel->setTextWrap(false);
  panel->setCursor(0, 0);
  panel->setTextColor(0xffff);
  panel->setTextSize(1);
}

void EightballGame::start(LEDContext &context)
{
  startTime = context.elapsed;
  setState(EightBallBoot, context);
  prevX = 0;
  loops = 0;
}

void EightballGame::newPrediction(LEDContext &context)
{
  curString = magic8Ball[random(0, sizeof(magic8Ball) / sizeof(String))];
  closeEnoughStart = 0;
  setState(EightBallText, context);
}

void EightballGame::setState(EightBallState newState, LEDContext &context)
{
  Serial.print("Eightball state: ");
  Serial.println(newState);

  state = newState;
  stateStartTime = context.elapsed;
}

void EightballGame::drawPanel(CRGB *leds, LEDContext &context)
{
  for (uint8_t i = 0; i < NUM_PIXELS; ++i)
  {
    uint8_t x = context.pixelCoords[i][0];
    uint8_t y = context.pixelCoords[i][1];

    uint8_t v = map(panel->sample((float)x / 255.0f, (float)y / 255.0f), 0, 0xffff, 0, 255);

    leds[i] = CHSV(v, 100, v);
  }
}

void EightballGame::draw(CRGB *leds, LEDContext &context)
{
  if (state == EightBallBoot)
  {
    panel->fillScreen(0);
    panel->setCursor(7, 22);
    panel->println("8");
    panel->drawToScreen(leds, context, true);

    if (context.elapsed - stateStartTime > 1000)
    {
      fade_video(leds, NUM_PIXELS, map(context.elapsed - stateStartTime, 1000, 2000, 0, 255));
    }

    if (context.elapsed - stateStartTime > 2000)
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
      closeEnoughStart = 0;
      isClose = false;
    }

    timeDelta = context.elapsed - closeEnoughStart;
    if (isClose && timeDelta > 4000)
    {
      newPrediction(context);
    }

    if (!isClose)
    {
      timeDelta = 0;
    }

    uint8_t m = map(timeDelta, 0, 4000, 0, 255);
    for (uint8_t i = 0; i < NUM_PIXELS; ++i)
    {
      uint8_t r = context.pixelCoordsPolar[i][0];
      uint8_t t = context.pixelCoordsPolar[i][1];

      uint8_t v = scale8(m, t - context.elapsed / 3);
      uint8_t v2 = sin8(r * 2 + context.elapsed / 5) / 2;

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

    panel->drawToScreen(leds, context, true);
  }

  if (context.elapsed - context.button1DownTime > 200 && !context.button1DownHandled)
  {
    start(context);
    context.button1DownHandled = true;
  }
}

Starfish::Starfish()
{
  palette = Coral_reef_gp;
}

void Starfish::start(LEDContext &context)
{
  startTime = context.elapsed;
  smoothValue = 127;
}

void Starfish::draw(CRGB *leds, LEDContext &context)
{
  MemoryPanel *panel = context.sharedPanel;

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

  panel->drawToScreen(leds, context, true);
}

RingGame::RingGame()
{
}

void RingGame::start(LEDContext &context)
{
  startTime = context.elapsed;

  score = 0;
  playerAngle = 0;
  ball = vec3(0, 0, 0);
  float angle = randFloat() * M_PI * 2.0;
  const float speed = 0.001;
  ballVel = vec3(cos(angle) * speed, sin(angle) * speed, 0);
}

void RingGame::draw(CRGB *leds, LEDContext &context)
{
  MemoryPanel *p = context.sharedPanel;

  playerAngle = float(context.remoteDist) / 255 * M_PI * 2.0 * 2.0;

  const int difficulty = (context.elapsed - startTime) / 10000;
  float speedModifier = float(difficulty) * 0.2 + 1.0;
  ball = v3_add(ball, v3_muls(ballVel, speedModifier));

  float ballAngle = atan2(ball.y, ball.x);
  const float playerBallDiff = abs(angleDiff(ballAngle, playerAngle));

  const float dist = v3_length(ball);
  if (dist > 0.5)
  {
    if (playerBallDiff < M_PI * 0.3)
    {
      vec3_t norm = v3_norm(v3_muls(ball, -1));
      norm = m4_mul_dir(m4_rotation_z(randFloat() * 0.1 - 0.05), norm);
      norm = v3_norm(norm);

      vec3_t refl = v3_sub(ballVel, v3_muls(norm, 2.0f * v3_dot(ballVel, norm)));
      ballVel = refl;
      score++;
    }
    else
    {
      return start(context);
    }
  }

  p->fillScreen(0);
  p->fillCircle((ball.x + 0.5) * p->width(), (ball.y + 0.5) * p->height(), p->width() / 15, convert888(CHSV(score * 50, qadd8(50, qmul8(difficulty, 75)), 255)));
  p->drawToScreen(leds, context, false);

  for (uint8_t i = 0; i < NUM_PIXELS; ++i)
  {
    const uint8_t r = context.pixelCoordsPolar[i][0];
    const float t = context.pixelCoordsPolarf[i][1];
    const float diff = abs(angleDiff(t, playerAngle));

    if (r > 225 && diff < M_PI * 0.25)
    {
      leds[i] = CRGB::White;
    }
  }
}

// Eyes::Eyes()
// {
// }

// void Eyes::start(LEDContext &context)
// {
// }

// void Eyes::draw(CRGB *leds, LEDContext &context)
// {
//   MemoryPanel *p = context.sharedPanel;

//   float dist = sin(float(context.elapsed) / 500.0f);
//   dist = pow(dist, 3.0);
//   dist = dist * 0.5 + 0.5;
//   if (context.elapsed - context.remoteDistTime < 200)
//   {
//     dist = float(map(constrain(context.remoteDist, 10, 200), 10, 200, 0, 255)) / 255.0f;
//   }

//   const float w = p->width();
//   const float h = p->height();
//   const float lX = w * 1.0 / 4.0;
//   const float rX = w * 3.0 / 4.0;
//   const float cY = h / 2.0;
//   const float largeR = w / 4.0;
//   const float smallR = w / 10.0;

//   p->fillScreen(0);

//   // Eyes
//   p->fillCircle(lX, cY, largeR, convert888(CHSV(0, 0, 150)));
//   p->fillCircle(rX, cY, largeR, convert888(CHSV(0, 0, 150)));

//   // Pupils
//   float eyeMovement = (dist - 0.5) * 2.0 * smallR;
//   p->fillCircle(lX + eyeMovement, cY + smallR, smallR, convert888(CHSV(0, 0, 0)));
//   p->fillCircle(rX + eyeMovement, cY + smallR, smallR, convert888(CHSV(0, 0, 0)));

//   // Blink
//   uint8_t n = inoise8(context.elapsed * 0.35);
//   if (n > 200)
//   {
//     uint8_t blinkAmt = n > 225 ? 0 : 2;
//     p->fillCircle(lX, cY - blinkAmt, largeR, convert888(CHSV(0, 0, 0)));
//     p->fillCircle(rX, cY - blinkAmt, largeR, convert888(CHSV(0, 0, 0)));
//   }

//   p->drawToScreen(leds, context, false);
// }

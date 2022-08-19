#define FASTLED_FORCE_SOFTWARE_SPI

#include <FastLED.h>
#include <SPI.h>

#include "types.h"
#include "led_panel.h"
#include "patterns.h"
#include "scenes.h"
#include "palettes.h"

CRGB leds[NUM_PIXELS];

LEDContext context;

bool rawButtonDown = false;
unsigned long rawButtonDownTime = 0;

Scene *scenes[32];
int kNumScenes = 0;

uint8_t curScene = 0;

extern const TProgmemRGBGradientPalettePtr gGradientPalettes[];
extern const uint8_t gGradientPaletteCount;

void setup()
{
  Serial.begin(115200);
  Serial.println("Start init");

  // Assign pattern
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Startup LEDs
  Serial.println("LED setup...");
  FastLED.addLeds<APA102, DATAPIN, CLOCKPIN, BGR, DATA_RATE_MHZ(4)>(leds, NUM_PIXELS);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 100);
  FastLED.setBrightness(64);

  for (uint8_t i = 0; i < NUM_PIXELS; ++i)
  {
    uint8_t ring = 0;
    while (!(i >= kRingIndexMap[ring][0] && i <= kRingIndexMap[ring][1]))
      ++ring;

    uint8_t pixelsInRing = kRingIndexMap[ring][1] - kRingIndexMap[ring][0];
    uint8_t pixelInRing = i - kRingIndexMap[ring][0];

    float angleFrac = float(pixelInRing) / float(pixelsInRing + 1);
    float radiusFrac = float(ring) / float(NUM_RINGS);

    context.pixelCoordsPolar[i][0] = 255 * radiusFrac;
    context.pixelCoordsPolar[i][1] = 255 * angleFrac;
    context.pixelCoordsPolarf[i][0] = radiusFrac * 0.5;
    context.pixelCoordsPolarf[i][1] = angleFrac * M_PI * 2.0;

    float angleRad = 2.0f * 3.14159f * angleFrac;
    float x = cos(angleRad) * radiusFrac; // [-1, 1]
    float y = sin(angleRad) * radiusFrac;

    float xNorm = x * 0.5 + 0.5;
    float yNorm = y * 0.5 + 0.5;

    // The central pixel, avoid undefined behavior
    if (i == NUM_PIXELS - 1)
    {
      xNorm = 0.5;
      yNorm = 0.5;
    }

    context.pixelCoords[i][0] = 255 * xNorm;
    context.pixelCoords[i][1] = 255 * yNorm;
    context.pixelCoordsf[i][0] = xNorm;
    context.pixelCoordsf[i][1] = yNorm;
  }

  // Build panel
  context.panel = new LEDPanel(32, 32);
  context.panel->setRotation(1);

  PatternScene *patternScene = new PatternScene(16, gGradientPalettes, gGradientPaletteCount);
  patternScene->addPattern(new Pattern0());
  patternScene->addPattern(new Pattern1());
  // patternScene->addPattern(new Pattern2());
  patternScene->addPattern(new Pattern3());
  patternScene->addPattern(new Pattern4());
  patternScene->addPattern(new Pattern5());
  patternScene->addPattern(new Pattern6());
  patternScene->addPattern(new Pattern7());
  patternScene->addPattern(new Pattern8());
  patternScene->addPattern(new Pattern9());
  patternScene->addPattern(new Pattern10());
  patternScene->addPattern(new Pattern11());

  // Build scenes
  randomSeed(micros() + analogRead(1) + micros() + analogRead(2) + micros());
  scenes[kNumScenes++] = patternScene;
  scenes[kNumScenes++] = new SpinGameScene();
  scenes[kNumScenes++] = new EightballScene();

  scenes[0]->start(context);

  Serial.println("Done init!");
}

void advanceScene()
{
  curScene = addmod8(curScene, 1, kNumScenes);
  scenes[curScene]->start(context);

  context.sceneStart = context.now;

  Serial.print("Advancing scene to: ");
  Serial.println(curScene);
}

void readButton()
{
  bool newButtonDown = !digitalRead(BUTTON_PIN);
  // Serial.println(newButtonDown ? "down" : "up");

  if (newButtonDown && rawButtonDownTime == 0)
  {
    rawButtonDown = true;
    rawButtonDownTime = context.now;
  }
  else if (!newButtonDown && rawButtonDownTime != 0)
  {
    rawButtonDown = false;
    rawButtonDownTime = 0;
  }

  // On release
  if (!rawButtonDown && context.buttonState)
  {
    Serial.println("Button relase");
    context.buttonState = false;
    context.buttonHold = false;
    context.buttonDownTime = 0;
    context.buttonDownHandled = true;
    context.buttonHoldHandled = true;
    context.buttonUpHandled = false;
  }

  // On press for 25ms
  if (rawButtonDown && !context.buttonState && context.now - rawButtonDownTime > 25)
  {
    Serial.println("Button press");
    context.buttonState = true;
    context.buttonHold = false;
    context.buttonDownTime = context.now;
    context.buttonDownHandled = false;
    context.buttonHoldHandled = true;
    context.buttonUpHandled = true;
  }

  // On press for 500ms
  if (context.buttonState && !context.buttonHold && context.now - context.buttonDownTime > 500)
  {
    Serial.println("Button hold");
    context.buttonHold = true;
    context.buttonHoldHandled = false;
    context.buttonUpHandled = true;
  }
}

void loop()
{
  context.now = millis();
  readButton();

  scenes[curScene]->draw(leds, context);

  if (!context.buttonHoldHandled) {
    advanceScene();
    context.buttonHoldHandled = true;
  }

  FastLED.show();
}

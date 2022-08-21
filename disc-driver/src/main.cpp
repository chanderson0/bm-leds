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

bool buttonPressed = false;
bool lastButtonReading = false;
unsigned long buttonReadingChangeTime = 0;
const uint16_t kButtonDebounceMs = 50;
const uint16_t kButtonTapTimeMs = 250;
const uint16_t kButtonHoldTimeMs = 750;

Scene *scenes[32];
int kNumScenes = 0;

uint8_t curScene = 0;

extern const TProgmemRGBGradientPalettePtr gGradientPalettes[];
extern const uint8_t gGradientPaletteCount;

void computeLEDs()
{
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
}

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

  // Compute LED coordinates
  computeLEDs();

  // Build panel
  context.panel = new LEDPanel(32, 32);
  context.panel->setRotation(1);

  PatternScene *patternScene = new PatternScene(16, gGradientPalettes, gGradientPaletteCount);
  patternScene->addPattern(new PatternNoise());      // 5/5
  patternScene->addPattern(new PatternDiamonds());   // 4/5
  patternScene->addPattern(new PatternWhirlpool2()); // 4/5
  patternScene->addPattern(new PatternWavyArms());   // 4/5
  patternScene->addPattern(new PatternSineHills());  // 4/5
  patternScene->addPattern(new PatternNautilus());   // 3/5

  // patternScene->addPattern(new PatternWhirlpool()); // 3/5
  // patternScene->addPattern(new PatternConcentricRings()); // 0/5
  // patternScene->addPattern(new PatternStripe()); // 1/5
  // patternScene->addPattern(new PatternWhirlpool3()); // 3/5
  // patternScene->addPattern(new PatternWaves()); // 2/5
  // patternScene->addPattern(new PatternLissajous()); // 2/5
  // patternScene->addPattern(new PatternTriangles()); // 2/5
  // patternScene->addPattern(new PatternGameOfLife()); // unfinished

  // Build scenes
  randomSeed(micros() + analogRead(1) + micros() + analogRead(2) + micros());
  scenes[kNumScenes++] = patternScene;
  scenes[kNumScenes++] = new Eyes();
  scenes[kNumScenes++] = new SpinGameScene();
  scenes[kNumScenes++] = new EightballScene();

  // Start the first scene
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
  const bool reading = !digitalRead(BUTTON_PIN);

  // Reset debounce timer
  if (reading != lastButtonReading)
  {
    lastButtonReading = reading;
    buttonReadingChangeTime = context.now;
  }

  // If it hasn't changed in a fixed amount of time, assume it's a true value
  if (context.now - buttonReadingChangeTime > kButtonDebounceMs)
  {
    Serial.println(reading ? "down" : "up");
    buttonPressed = reading;
  }

  // Reset instant events
  context.buttonDidPress = false;
  context.buttonDidRelease = false;
  context.buttonDidTap = false;
  context.buttonDidHold = false;

  // Read new state
  if (context.buttonPressed != buttonPressed)
  {
    // Assign press and release instant states
    context.buttonDidPress = buttonPressed;
    context.buttonDidRelease = !buttonPressed;

    // On falling edge, do some additional checks
    if (!buttonPressed)
    {
      context.buttonHeld = false;

      if (context.now - context.buttonChangeTime < kButtonTapTimeMs)
      {
        context.buttonDidTap = true;
      }
    }

    // Set the general states
    context.buttonPressed = buttonPressed;
    context.buttonChangeTime = context.now;
  }

  if (context.buttonPressed && !context.buttonHeld && (context.now - context.buttonChangeTime) > kButtonHoldTimeMs)
  {
    context.buttonHeld = true;
    context.buttonDidHold = true;
  }
}

void loop()
{
  context.now = millis();
  readButton();

  scenes[curScene]->draw(leds, context);

  if (context.buttonDidHold)
  {
    advanceScene();
  }
  if (context.buttonPressed)
  {
    leds[0] = CRGB(255, 0, 0);
  }

  FastLED.show();
}

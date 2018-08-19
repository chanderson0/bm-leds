#define FASTLED_FORCE_SOFTWARE_SPI

#include <FastLED.h>
#include <SPI.h>
#include "RF24.h"

#include "types.h"
#include "memory_panel.h"
#include "patterns.h"
#include "games.h"
#include "palettes.h"

CRGB leds[NUM_PIXELS];
CRGB prevLeds[NUM_PIXELS];

const uint64_t kDiscAddr = 0xF0F0F0F0E1LL;
const uint64_t kRemoteAddr = 0xF0F0F0F0D2LL;

RF24 radio(9, 10);
bool role = 0;
#define MESSAGE_LENGTH 3
byte sendMessage[MESSAGE_LENGTH] = {0x0, 0x0, 0x0};
byte receiveMessage[MESSAGE_LENGTH] = {0x0, 0x0, 0x0};
uint8_t receiveIdx = 0;

unsigned long now;
unsigned long elapsed;

LEDContext context;

Pattern *patterns[25];
int kNumPatterns = 0;

Game *games[25];
int kNumGames = 0;

enum DiscState
{
  DiscPatterns = 0,
  DiscGames = 1,
  NumDiscStates = 2
};

DiscState state = DiscPatterns;

uint8_t curPattern = 0;
uint8_t prevPattern = 0;
uint8_t patternAmt = 255;
bool rotatingPattern = true;
// bool rotatingPattern = false;
unsigned long rotateChangedTime = 0;

uint8_t curGame = 0;

// Borrowed from https://learn.adafruit.com/twinkling-led-parasol/code
extern const TProgmemRGBGradientPalettePtr gGradientPalettes[];
extern const uint8_t gGradientPaletteCount;

// Current palette number from the 'playlist' of color palettes
uint8_t gCurrentPaletteNumber = 0;

CRGBPalette16 gCurrentPalette(gGradientPalettes[0]);
CRGBPalette16 gTargetPalette(gGradientPalettes[0]);

void setup()
{
  Serial.begin(9600);

  Serial.println("Start init");
  Serial.println("Starting radio...");
  radio.begin();
  radio.setPALevel(RF24_PA_LOW);
  radio.openWritingPipe(kRemoteAddr);
  radio.openReadingPipe(1, kDiscAddr);
  radio.setAutoAck(true);
  radio.startListening();
  radio.printDetails();
  Serial.println(radio.isChipConnected() ? "Radio setup success" : "ERROR: Radio is not connected.");

  Serial.println("LED setup...");
  FastLED.addLeds<APA102, DATAPIN, CLOCKPIN, BGR, DATA_RATE_MHZ(4)>(leds, NUM_PIXELS);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 100);
  FastLED.setBrightness(64);

  for (uint8_t i = 0; i < NUM_PIXELS; ++i)
  {
    uint8_t ring = 0;
    while (!(i >= ringIndexMap[ring][0] && i <= ringIndexMap[ring][1]))
      ++ring;

    uint8_t pixelsInRing = ringIndexMap[ring][1] - ringIndexMap[ring][0];
    uint8_t pixelInRing = i - ringIndexMap[ring][0];

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
    context.pixelCoordsf[i][0] = xNorm - 0.5;
    context.pixelCoordsf[i][1] = yNorm - 0.5;
  }

  context.sharedPanel = new MemoryPanel(32, 32);
  context.curPalette = &gCurrentPalette;

  patterns[kNumPatterns++] = new Pattern0();
  patterns[kNumPatterns++] = new Pattern1();
  patterns[kNumPatterns++] = new Pattern2();
  patterns[kNumPatterns++] = new Pattern3();
  patterns[kNumPatterns++] = new Pattern4();
  patterns[kNumPatterns++] = new Pattern5();
  patterns[kNumPatterns++] = new Pattern6();
  patterns[kNumPatterns++] = new Pattern7();
  patterns[kNumPatterns++] = new Pattern8();
  patterns[kNumPatterns++] = new Pattern9();
  patterns[kNumPatterns++] = new Pattern10();
  patterns[kNumPatterns++] = new Pattern11();

  randomSeed(analogRead(0));
  games[kNumGames++] = new Starfish();
  games[kNumGames++] = new SpinGame();
  games[kNumGames++] = new TextGame();
  if (state == DiscGames)
  {
    games[0]->start(context);
  }

  Serial.println("Done init!");
}

bool txMessage(uint8_t v1, uint8_t v2)
{
  radio.stopListening();
  sendMessage[0] = v1;
  sendMessage[1] = v2;
  radio.setRetries(0, 1);
  bool success = radio.write(sendMessage, sizeof(byte) * MESSAGE_LENGTH);
  radio.startListening();
  return success;
}

void handleMessage()
{
  if (receiveMessage[0] == MSG_BUTTON_1)
  {
    if (receiveMessage[1] == MSG_BUTTON_DOWN)
    {
      context.button1State = true;
      context.button1DownTime = now;
      context.button1DownHandled = false;
    }
    else if (receiveMessage[1] == MSG_BUTTON_UP)
    {
      context.button1State = false;
      context.button1UpTime = now;
      context.button1UpHandled = false;
    }
  }
  else if (receiveMessage[0] == MSG_BUTTON_2)
  {
    if (receiveMessage[1] == MSG_BUTTON_DOWN)
    {
      context.button2State = true;
      context.button2DownTime = now;
      context.button2DownHandled = false;
    }
    else if (receiveMessage[1] == MSG_BUTTON_UP)
    {
      context.button2State = false;
      context.button2UpTime = now;
      context.button2UpHandled = false;
    }
  }
  else if (receiveMessage[0] == MSG_DISTANCE)
  {
    context.remoteDistTime = now;
    context.remoteDist = receiveMessage[1];
  }
}

void advancePattern()
{
  prevPattern = curPattern;
  curPattern = addmod8(curPattern, 1, kNumPatterns);
  patternAmt = 0;

  context.curSceneStart = now;

  Serial.print("Advancing pattern from ");
  Serial.print(prevPattern);
  Serial.print(" to ");
  Serial.println(curPattern);
}

void advancePalette()
{
  gCurrentPaletteNumber = addmod8(gCurrentPaletteNumber, 1, gGradientPaletteCount);
  gTargetPalette = gGradientPalettes[gCurrentPaletteNumber];

  Serial.print("Advancing palette to: ");
  Serial.println(gCurrentPaletteNumber);
}

void advanceGame()
{
  curGame = addmod8(curGame, 1, kNumGames);
  games[curGame]->start(context);

  Serial.print("Advancing game to: ");
  Serial.println(curGame);
}

void advanceState()
{
  state = (DiscState)addmod8((uint8_t)state, 1, NumDiscStates);

  if (state == DiscGames)
  {
    games[curGame]->start(context);
  }

  Serial.print("State is now: ");
  Serial.println(state);
}

void toggleRotate()
{
  rotatingPattern = !rotatingPattern;
  rotateChangedTime = now;

  Serial.print("Toggling rotate to: ");
  Serial.println(rotatingPattern ? "on" : "off");
}

void drawPatterns()
{
  if (now - context.button1DownTime > 100 && !context.button1DownHandled)
  {
    if (!context.button2DownHandled)
    {
      context.button2DownHandled = true;
      toggleRotate();
    }
    else
    {
      advancePattern();
    }
    context.button1DownHandled = true;
  }
  if (now - context.button2DownTime > 100 && !context.button2DownHandled)
  {
    if (!context.button1DownHandled)
    {
      context.button1DownHandled = true;
      toggleRotate();
    }
    else
    {
      advancePalette();
    }
    context.button2DownHandled = true;
  }

  EVERY_N_SECONDS(SCENE_TIME)
  {
    if (rotatingPattern)
    {
      advancePattern();
    }
  }

  EVERY_N_SECONDS(PALETTE_TIME)
  {
    if (rotatingPattern)
    {

      advancePalette();
    }
  }

  EVERY_N_MILLISECONDS(10)
  {
    nblendPaletteTowardPalette(gCurrentPalette, gTargetPalette, 64);
  }

  patterns[curPattern]->draw(leds, context);

  if (patternAmt != 255)
  {
    patterns[prevPattern]->draw(prevLeds, context);

    EVERY_N_MILLISECONDS(10)
    {
      patternAmt = qadd8(patternAmt, 8);
    }

    nblend(leds, prevLeds, NUM_PIXELS, 255 - patternAmt);
  }
}

void drawGames()
{
  if (now - context.button1DownTime > 100 && !context.button1DownHandled)
  {
    if (!context.button2DownHandled)
    {
      context.button1DownHandled = true;
      context.button2DownHandled = true;
      advanceGame();
    }
  }
  if (now - context.button2DownTime > 100 && !context.button2DownHandled)
  {
    if (!context.button1DownHandled && context.button1State)
    {
      context.button1DownHandled = true;
      context.button2DownHandled = true;
      advanceGame();
    }
  }

  games[curGame]->draw(leds, context);
}

void handleMetaButtons()
{
  if (context.button1State && context.button2State && now - context.button1DownTime > 3000 && now - context.button2DownTime > 3000)
  {
    advanceState();
    context.button1DownTime = now;
    context.button2DownTime = now;
  }
}

void drawMeta()
{
  // TODO: fix
  // if (now > 1500 && now - rotateChangedTime < 1500)
  // {
  //   int diff = now - rotateChangedTime;
  //   for (uint8_t i = 0; i < NUM_PIXELS; ++i)
  //   {
  //     uint8_t r = context.pixelCoordsPolar[i][0];
  //     // uint8_t t = context.pixelCoordsPolar[i][1];

  //     if (r > 200)
  //     {
  //       CRGB blendColor = CHSV(0, 0, rotatingPattern ? 255 : 0);
  //       leds[i] = blend(leds[i], blendColor, map(diff, 0, 1500, 0, r));
  //     }
  //   }
  // }
}

void loop()
{
  now = millis();
  context.elapsed = now;

  handleMetaButtons();

  if (state == DiscPatterns)
  {
    drawPatterns();
  }
  else if (state == DiscGames)
  {
    drawGames();
  }

  drawMeta();

  FastLED.show();

  while (radio.available())
  {
    radio.read(&receiveMessage, sizeof(byte) * MESSAGE_LENGTH);
    handleMessage();
  }

  EVERY_N_MILLISECONDS(500)
  {
    txMessage(MSG_HB, 0x0);
  }
}

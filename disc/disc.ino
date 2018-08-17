#define FASTLED_FORCE_SOFTWARE_SPI

#include <FastLED.h>
#include <SPI.h>
#include "RF24.h"

// Rings
// 48, 43, 40, 32, 29, 24, 20. 12, 6, 1

#define NUM_PIXELS 255
#define NUM_RINGS 10
#define DATAPIN 7
#define CLOCKPIN 14

#define SCENE_TIME 21
#define PALETTE_TIME 13

#define MSG_HB 0x10
#define MSG_DISTANCE 0x20
#define MSG_BUTTON_1 0x30
#define MSG_BUTTON_2 0x31

#define BUTTON_PIN_1 23
#define BUTTON_PIN_2 22
#define LED_PIN 21

CRGB leds[NUM_PIXELS];
CRGB prevLeds[NUM_PIXELS];
uint8_t ringIndexMap[NUM_RINGS][2] = {
    {254, 254}, //0 Center Point
    {248, 253}, //1
    {236, 247}, //2
    {216, 235}, //3
    {192, 215}, //4
    {164, 191}, //5
    {132, 163}, //6
    {92, 131},  //7
    {48, 91},   //8
    {0, 47},    //9 Outer Ring
};
uint8_t pixelCoords[NUM_PIXELS][2];
uint8_t pixelCoordsPolar[NUM_PIXELS][2];

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
uint8_t remoteDist = 127;
unsigned long remoteDistTime = 0;
unsigned long button1Time = 0;
bool button1Handled = true;
unsigned long button2Time = 0;
bool button2Handled = true;

#define NUM_SCENES 10
uint8_t curScene = 0;
uint8_t prevScene = 0;
uint8_t sceneAmt = 255;
bool rotatingScene = true;
// bool rotatingScene = false;
unsigned long rotatingSceneTime = 0;

// Borrowed from https://learn.adafruit.com/twinkling-led-parasol/code
extern const TProgmemRGBGradientPalettePtr gGradientPalettes[];
extern const uint8_t gGradientPaletteCount;

// Current palette number from the 'playlist' of color palettes
uint8_t gCurrentPaletteNumber = 0;

CRGBPalette16 gCurrentPalette(gGradientPalettes[0]);
CRGBPalette16 gTargetPalette(gGradientPalettes[0]);

void setup()
{
  Serial.println("Starting radio...");
  radio.begin();
  radio.setPALevel(RF24_PA_LOW);
  radio.openWritingPipe(kRemoteAddr);
  radio.openReadingPipe(1, kDiscAddr);
  radio.powerUp();
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

    pixelCoordsPolar[i][0] = 255 * radiusFrac;
    pixelCoordsPolar[i][1] = 255 * angleFrac;

    float angleRad = 2.0f * 3.14159f * angleFrac;
    float x = cos(angleRad) * radiusFrac; // [-1, 1]
    float y = sin(angleRad) * radiusFrac;

    float xNorm = x * 0.5 + 0.5;
    float yNorm = y * 0.5 + 0.5;

    if (i == NUM_PIXELS - 1)
    {
      xNorm = 0.5;
      yNorm = 0.5;
    }

    pixelCoords[i][0] = 255 * xNorm;
    pixelCoords[i][1] = 255 * yNorm;
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
  // Serial.println("Got message:");
  // Serial.print(receiveMessage[0], HEX);
  // Serial.print(" ");
  // Serial.print(receiveMessage[1], HEX);
  // Serial.print(" ");
  // Serial.print(receiveMessage[2], HEX);
  // Serial.println();

  if (receiveMessage[0] == MSG_BUTTON_1)
  {
    button1Time = now;
    button1Handled = false;
  }
  else if (receiveMessage[0] == MSG_BUTTON_2)
  {
    button2Time = now;
    button2Handled = false;
  }
  else if (receiveMessage[0] == MSG_DISTANCE)
  {
    remoteDistTime = now;
    remoteDist = receiveMessage[1];

    if (remoteDist)
  }
}

void scenes(int scene, CRGB *leds)
{
  if (scene == 0)
  {
    for (uint8_t i = 0; i < NUM_PIXELS; ++i)
    {
      uint8_t r = pixelCoordsPolar[i][0];
      uint8_t t = pixelCoordsPolar[i][1];
      uint8_t h = t - sin8(r + elapsed / 50) + elapsed / 10;

      leds[i] = ColorFromPalette(gCurrentPalette, h);
    }
  }
  else if (scene == 1)
  {
    for (uint8_t i = 0; i < NUM_PIXELS; ++i)
    {
      uint8_t r = pixelCoordsPolar[i][0];
      uint8_t h = sin8(-r + elapsed / 5);
      leds[i] = ColorFromPalette(gCurrentPalette, h);
    }
  }
  else if (scene == 2)
  {
    for (uint8_t i = 0; i < NUM_PIXELS; ++i)
    {
      uint8_t x = pixelCoords[i][0];
      uint8_t h = sin8(x + elapsed / 5);
      leds[i] = ColorFromPalette(gCurrentPalette, h);
    }
  }
  else if (scene == 3)
  {
    for (uint8_t i = 0; i < NUM_PIXELS; ++i)
    {
      uint8_t x = pixelCoords[i][0];
      uint8_t y = pixelCoords[i][1];
      uint8_t dx = sin8(x + elapsed / 10);
      uint8_t dy = sin8(y * 2);
      leds[i] = ColorFromPalette(gCurrentPalette, dx - dy);
    }
  }
  else if (scene == 4)
  {
    for (uint8_t i = 0; i < NUM_PIXELS; ++i)
    {
      uint8_t r = pixelCoordsPolar[i][0];
      uint8_t t = pixelCoordsPolar[i][1];
      uint8_t dr = sin8(r + elapsed / 10);
      uint8_t dt = sin8(t * 2 + elapsed / 15);
      leds[i] = ColorFromPalette(gCurrentPalette, dr - dt);
    }
  }
  else if (scene == 5)
  {
    for (uint8_t i = 0; i < NUM_PIXELS; ++i)
    {
      uint8_t r = pixelCoordsPolar[i][0];
      uint8_t t = pixelCoordsPolar[i][1];
      uint8_t v = scale8(sin8(r + elapsed / 20), sin8(t * 3 - elapsed / 10));
      leds[i] = ColorFromPalette(gCurrentPalette, v);
    }
  }
  else if (scene == 6)
  {
    for (uint8_t i = 0; i < NUM_PIXELS; ++i)
    {
      uint8_t r = pixelCoordsPolar[i][0];
      uint8_t t = pixelCoordsPolar[i][1];
      uint8_t v = t * 5 + sin8(r - elapsed / 10);
      leds[i] = ColorFromPalette(gCurrentPalette, v);
    }
  }
  else if (scene == 7)
  {
    for (uint8_t i = 0; i < NUM_PIXELS; ++i)
    {
      uint8_t x = pixelCoords[i][0];
      uint8_t y = pixelCoords[i][1];
      uint8_t v = mul8(x, 3) + sin8(y + elapsed / 10);
      leds[i] = ColorFromPalette(gCurrentPalette, v);
    }
  }
  else if (scene == 8)
  {
    for (uint8_t i = 0; i < NUM_PIXELS; ++i)
    {
      uint8_t r = pixelCoordsPolar[i][0];
      uint8_t t = pixelCoordsPolar[i][1];
      uint8_t v = t + r - elapsed / 20; // + elapsed / 30;
      leds[i] = ColorFromPalette(gCurrentPalette, v);
    }
  }
  else if (scene == 9)
  {
    for (uint8_t i = 0; i < NUM_PIXELS; ++i)
    {
      uint8_t x = pixelCoords[i][0];
      uint8_t y = pixelCoords[i][1];
      uint8_t v = sin8(x / 2 + elapsed / 30) + sin8(y / 2 - elapsed / 25) - elapsed / 20;
      leds[i] = ColorFromPalette(gCurrentPalette, v);
    }
  }
}

void advanceScene()
{
  prevScene = curScene;
  curScene = addmod8(curScene, 1, NUM_SCENES);
  sceneAmt = 0;

  Serial.print("Advancing scene from ");
  Serial.print(prevScene);
  Serial.print(" to ");
  Serial.println(curScene);
}

void advancePalette()
{
  gCurrentPaletteNumber = addmod8(gCurrentPaletteNumber, 1, gGradientPaletteCount);
  gTargetPalette = gGradientPalettes[gCurrentPaletteNumber];

  Serial.print("Advancing palette to: ");
  Serial.println(gCurrentPaletteNumber);
}

void toggleRotate()
{
  rotatingScene = !rotatingScene;
  rotatingSceneTime = now;

  Serial.print("Toggling rotate to: ");
  Serial.println(rotatingScene ? "on" : "off");
}

unsigned long lastFrameMicros = 0;
unsigned long long elapsedMicros = 0x88888888;
void loop()
{
  now = millis();

  unsigned long nowMicros = micros();
  unsigned long deltaMicros = nowMicros - lastFrameMicros;
  if (now - remoteDistTime < 500)
  {
    elapsedMicros += float(deltaMicros) * map(float(remoteDist), 0.0f, 255.0f, -1.0f, 1.0f);
  }
  else
  {
    elapsedMicros += deltaMicros;
  }
  elapsed = elapsedMicros / 1000;
  lastFrameMicros = nowMicros;

  if (now - button1Time > 100 && !button1Handled)
  {
    if (!button2Handled)
    {
      button2Handled = true;
      toggleRotate();
    }
    else
    {
      advanceScene();
    }
    button1Handled = true;
  }
  if (now - button2Time > 100 && !button2Handled)
  {
    if (!button1Handled)
    {
      button1Handled = true;
      toggleRotate();
    }
    else
    {
      advancePalette();
    }
    button2Handled = true;
  }

  EVERY_N_SECONDS(SCENE_TIME)
  {
    if (rotatingScene)
    {
      advanceScene();
    }
  }

  EVERY_N_SECONDS(PALETTE_TIME)
  {
    if (rotatingScene)
    {
      advancePalette();
    }
  }

  EVERY_N_MILLISECONDS(10)
  {
    nblendPaletteTowardPalette(gCurrentPalette, gTargetPalette, 64);
  }

  scenes(curScene, leds);

  if (sceneAmt != 255)
  {
    scenes(prevScene, prevLeds);

    EVERY_N_MILLISECONDS(10)
    {
      sceneAmt = qadd8(sceneAmt, 8);
    }

    nblend(leds, prevLeds, NUM_PIXELS, 255 - sceneAmt);
  }

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

// Also from https://learn.adafruit.com/twinkling-led-parasol/code
DEFINE_GRADIENT_PALETTE(rainbow_gp){
    0,
    255,
    0,
    0,
    85,
    0,
    255,
    0,
    170,
    0,
    0,
    255,
    255,
    255,
    0,
    0,
};

DEFINE_GRADIENT_PALETTE(Coral_reef_gp){
    0, 40, 199, 197,
    50, 10, 152, 155,
    96, 1, 111, 120,
    96, 43, 127, 162,
    139, 10, 73, 111,
    255, 1, 34, 71};

DEFINE_GRADIENT_PALETTE(fire_gp){
    0, 1, 1, 0,
    76, 32, 5, 0,
    146, 192, 24, 0,
    197, 220, 105, 5,
    240, 252, 255, 31,
    250, 252, 255, 111,
    255, 255, 255, 255};

// Gradient palette "My_Heart_Is_Crippled_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/colo/lightningmccarl/tn/My_Heart_Is_Crippled.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 40 bytes of program space.

DEFINE_GRADIENT_PALETTE(My_Heart_Is_Crippled_gp){
    0, 255, 199, 93,
    61, 255, 199, 93,
    61, 190, 107, 55,
    96, 190, 107, 55,
    96, 135, 45, 29,
    104, 135, 45, 29,
    104, 92, 10, 12,
    181, 92, 10, 12,
    181, 58, 1, 3,
    255, 58, 1, 3};

DEFINE_GRADIENT_PALETTE(white_gp){
    0,
    30,
    30,
    30,
    50,
    50,
    50,
    50,
    127,
    30,
    30,
    30,
    255,
    0,
    0,
    0,
};

DEFINE_GRADIENT_PALETTE(red_gp){
    0,
    0,
    0,
    0,
    75,
    40,
    0,
    0,
    150,
    0,
    0,
    0,
    255,
    0,
    0,
    0,
};

// Gradient palette "Caribbean_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/vh/tn/Caribbean.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 144 bytes of program space.

DEFINE_GRADIENT_PALETTE(Caribbean_gp){
    0, 0, 0, 43,
    24, 0, 5, 85,
    36, 0, 10, 106,
    48, 1, 22, 130,
    60, 1, 32, 147,
    72, 1, 57, 138,
    84, 1, 79, 138,
    97, 1, 91, 147,
    109, 3, 104, 156,
    121, 12, 104, 166,
    133, 17, 118, 176,
    133, 17, 118, 138,
    145, 35, 118, 176,
    157, 64, 118, 176,
    170, 82, 133, 156,
    182, 88, 149, 186,
    194, 95, 175, 207,
    206, 110, 184, 230,
    213, 173, 203, 242,
    218, 206, 213, 242,
    218, 33, 53, 14,
    219, 44, 62, 14,
    219, 46, 62, 14,
    221, 58, 68, 18,
    223, 75, 73, 24,
    225, 95, 79, 27,
    228, 110, 91, 27,
    230, 126, 97, 31,
    233, 144, 111, 27,
    235, 163, 118, 27,
    237, 184, 125, 26,
    237, 184, 125, 27,
    240, 206, 141, 25,
    242, 229, 149, 25,
    245, 242, 166, 24,
    255, 242, 223, 197};

// Gradient palette "rgi_03_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/ds/rgi/tn/rgi_03.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 20 bytes of program space.

DEFINE_GRADIENT_PALETTE(rgi_03_gp){
    0, 247, 79, 17,
    95, 80, 27, 32,
    191, 11, 3, 52,
    223, 16, 4, 45,
    255, 22, 6, 38};

// Gradient palette "Paired_05_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/cb/qual/tn/Paired_05.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 40 bytes of program space.

DEFINE_GRADIENT_PALETTE(Paired_05_gp){
    0, 83, 159, 190,
    51, 1, 48, 106,
    102, 100, 189, 54,
    153, 3, 91, 3,
    204, 244, 84, 71};

const TProgmemRGBGradientPalettePtr gGradientPalettes[] = {
    fire_gp,
    rainbow_gp,
    rgi_03_gp,
    Caribbean_gp,
    Paired_05_gp,
    red_gp,
};

// Count of how many cpt-city gradients are defined:
const uint8_t gGradientPaletteCount =
    sizeof(gGradientPalettes) / sizeof(TProgmemRGBGradientPalettePtr);

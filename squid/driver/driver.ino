#define USE_OCTOWS2811

#include <OctoWS2811.h>
#include <XBee.h>
#include <FastLED.h>

#define NUM_LEDS_PER_STRIP 150
#define NUM_STRIPS         6
#define NUM_PIXELS         (NUM_LEDS_PER_STRIP*NUM_STRIPS)

#define MSG_DIST  10
#define MSG_WHITE 20
#define MSG_BLACK 21
#define MSG_BOTH  22
#define MSG_HB    99

#define SCENE_TIME   35
#define PALETTE_TIME 27

// LED data
CRGB leds[NUM_PIXELS];
CRGB prevLeds[NUM_PIXELS];
uint8_t ledFracs[NUM_LEDS_PER_STRIP];

// XBee comms
XBee xbee = XBee();
int clientAddr = 0x1001;
uint8_t payload[] = { 0, 0 };
Tx16Request tx = Tx16Request(clientAddr, payload, sizeof(payload));
TxStatusResponse txStatus = TxStatusResponse();
Rx16Response rx16 = Rx16Response();

// Scenes
#define NUM_SCENES 3
uint8_t curSceneIdx = 0;
uint8_t prevSceneIdx = 0;
uint8_t sceneTransitionAmt = 255;
bool rotatingScene = false;
unsigned long rotatingSceneTime = 0;

// Palletes
extern const TProgmemRGBGradientPalettePtr gGradientPalettes[];
extern const uint8_t gGradientPaletteCount;
CRGBPalette16 curPalette(gGradientPalettes[0]);
CRGBPalette16 targetPalette(gGradientPalettes[0]);
uint8_t curPaletteIdx = 0;

// Timing
unsigned long now = 0;
unsigned long elapsed = 0;
unsigned long lastMsgReceived = 0;
unsigned long lastWhiteReceived = 0;
unsigned long lastBlackReceived = 0;
uint8_t heartbeatIdx = 0;
unsigned long controllerDistTime = 0;
uint8_t controllerDist = 0;

uint8_t sawtoothElapsed(uint8_t bpm, uint32_t ts) {
  return (ts * (bpm << 8) * 280 * 4) >> 16 >> 8;
}

void setup() {
  Serial.begin(57600);
  Serial.println("Starting setup...");
  delay(100);

  Serial4.begin(57600);
  xbee.setSerial(Serial4);

  // Pin layouts on the teensy 3:
  // OctoWS2811: 2,14,7,8,6,20,21,5
  LEDS.addLeds<OCTOWS2811>(leds, NUM_LEDS_PER_STRIP);
  LEDS.setBrightness(32);

  for(uint8_t j = 0; j < NUM_LEDS_PER_STRIP; j++) {
    ledFracs[j] = uint8_t(float(j) / float(NUM_LEDS_PER_STRIP) * 255.0);
  }
  
  Serial.println("Setup complete!");
}

void advanceScene() {
  prevSceneIdx = curSceneIdx;
  curSceneIdx = addmod8(curSceneIdx, 1, NUM_SCENES);
  sceneTransitionAmt = 0;
}

void advancePalette() {
  curPaletteIdx = addmod8(curPaletteIdx, 1, gGradientPaletteCount);
  targetPalette = gGradientPalettes[curPaletteIdx];
}

void handleXbee() {
  uint8_t msgType = rx16.getData(0);
  uint8_t msgData = rx16.getData(1);

  switch (msgType) {
    case MSG_DIST:
      if (msgData <= 254 && msgData >= 1) {
        controllerDistTime = now;
        controllerDist = msgData;
      }
      break;
    case MSG_WHITE:
      Serial.println("XBee Message: white");
      lastWhiteReceived = now;
      advanceScene();
      break;
    case MSG_BLACK:
      Serial.println("XBee Message: black");
      lastBlackReceived = now;
      advancePalette();
      break;
    case MSG_BOTH:
      Serial.println("XBee Message: both");
      lastWhiteReceived = now;
      lastBlackReceived = now;
      rotatingScene = !rotatingScene;
      break;
    default:
      Serial.print("Error: XBee received unknown message: ");
      Serial.println(msgType);
      return;
  }
}

unsigned long lastFrameMicros = 0;
unsigned long long elapsedMicros = 0x88888888;
void loop() {
  now = millis();

  unsigned long nowMicros = micros();
  unsigned long deltaMicros = nowMicros - lastFrameMicros;
  if (now - controllerDistTime < 500) {
    elapsedMicros += float(deltaMicros) * map(float(controllerDist), 0.0f, 255.0f, -1.0f, 1.0f);
  } else {
    elapsedMicros += deltaMicros;
  }
  elapsed = elapsedMicros / 1000;
  lastFrameMicros = nowMicros;

  // Blend palettes towards target quickly
  EVERY_N_MILLISECONDS(10) {
    nblendPaletteTowardPalette(curPalette, targetPalette, 64);
  }

  // EVERY_N_SECONDS(SCENE_TIME) {
  //   if (rotatingScene) {
  //     advanceScene();
  //   }
  // }

  EVERY_N_SECONDS(PALETTE_TIME) {
    if (rotatingScene) {
      advancePalette();
    }
  }

  // Render current scene
  drawScene(curSceneIdx, leds);

  // Move toward current scene
  EVERY_N_MILLISECONDS(10) {
    sceneTransitionAmt = qadd8(sceneTransitionAmt, 8);
  }
  // If not fully transitioned, render the previous scene and blend
  if (sceneTransitionAmt != 255) {
    drawScene(prevSceneIdx, prevLeds);
    nblend(leds, prevLeds, NUM_PIXELS, 255 - sceneTransitionAmt);
  }

  if (now - lastWhiteReceived < 1000) {
    leds[0] = (int((now - lastWhiteReceived) / 250) % 2) == 0 ? CRGB::White : CRGB::Black;
  }
  if (now - lastBlackReceived < 1000) {
    leds[0 + NUM_LEDS_PER_STRIP] = (int((now - lastBlackReceived) / 250) % 2) == 0 ? CRGB::Red : CRGB::Black;
  }

  // Push values
  LEDS.show();

  // Send a heardbeat regularly
  EVERY_N_MILLISECONDS(250) {
    Serial.print("Sending heartbeat ");
    Serial.println(heartbeatIdx);
    payload[0] = MSG_HB;
    payload[1] = heartbeatIdx++;
    xbee.send(tx);
  }

  // Check for new messages
  xbee.readPacket();
  if (xbee.getResponse().isAvailable()) {
    if (xbee.getResponse().getApiId() == TX_STATUS_RESPONSE) {
      xbee.getResponse().getTxStatusResponse(txStatus);
      if (txStatus.getStatus() == SUCCESS) {
        // Serial.println("Successful send!");
      } else {
        Serial.println("Error in sending.");
      }
    } else if (xbee.getResponse().getApiId() == RX_16_RESPONSE) {
      lastMsgReceived = now;
      
      xbee.getResponse().getRx16Response(rx16);
      handleXbee();

      // Serial.print("XBee received: ");
      // for (int i = 0; i < rx16.getDataLength(); i++) { 
      //   Serial.write(rx16.getData(i)); 
      // }
      // Serial.println();
    } else {
      Serial.print("Got unrecognized packet: ");
      Serial.println(xbee.getResponse().getApiId());
    }
  } else if (xbee.getResponse().isError()) {
    int errorCode = xbee.getResponse().getErrorCode();
    Serial.print("XBee error: ");
    Serial.println(errorCode);
  }
}

void drawScene(uint8_t sceneIdx, CRGB *leds) {
  int ledIdx;

  if (sceneIdx == 0) {
    for(uint8_t i = 0; i < NUM_STRIPS; i++) {
      for(uint8_t j = 0; j < NUM_LEDS_PER_STRIP; j++) {
        ledIdx = (i*NUM_LEDS_PER_STRIP) + j;

        uint8_t v = sawtoothElapsed(60, -elapsed / 15 + ledFracs[j] * 1);
        leds[ledIdx] = ColorFromPalette(curPalette, v);
      }
    }
  } else if (sceneIdx == 1) {
    for(uint8_t i = 0; i < NUM_STRIPS; i++) {
      for(uint8_t j = 0; j < NUM_LEDS_PER_STRIP; j++) {
        ledIdx = (i*NUM_LEDS_PER_STRIP) + j;

        uint8_t v = sawtoothElapsed(120, -elapsed / 15 + ledFracs[j] * 2);
        leds[ledIdx] = ColorFromPalette(curPalette, v);        
      }
    }
  } else if (sceneIdx == 2) {
    for(uint8_t i = 0; i < NUM_STRIPS; i++) {
      for(uint8_t j = 0; j < NUM_LEDS_PER_STRIP; j++) {
        ledIdx = (i*NUM_LEDS_PER_STRIP) + j;

        uint8_t v = sawtoothElapsed(180, -elapsed / 15 + ledFracs[j] * 4);
        leds[ledIdx] = ColorFromPalette(curPalette, v);        
      }
    }
  } else {
    // Error state
    for(uint8_t i = 0; i < NUM_STRIPS; i++) {
      for(uint8_t j = 0; j < NUM_LEDS_PER_STRIP; j++) {
        ledIdx = (i*NUM_LEDS_PER_STRIP) + j;
        uint8_t b = j <= i ? 255 : 0;
        leds[ledIdx] = CHSV(255, 255, b);
      }
    }
  }
}

// Also from https://learn.adafruit.com/twinkling-led-parasol/code
DEFINE_GRADIENT_PALETTE( rainbow_gp ) {
  0,   255,   0,   0,
  85,    0, 255,   0,
  170,   0,   0, 255,
  255, 255,   0,   0,
};

DEFINE_GRADIENT_PALETTE( Coral_reef_gp ) {
  0,  40, 199, 197,
  50,  10, 152, 155,
  96,   1, 111, 120,
  96,  43, 127, 162,
  139,  10, 73, 111,
  255,   1, 34, 71
};

DEFINE_GRADIENT_PALETTE( fire_gp ) {
  0,   1,  1,  0,
  76,  32,  5,  0,
  146, 192, 24,  0,
  197, 220, 105,  5,
  240, 252, 255, 31,
  250, 252, 255, 111,
  255, 255, 255, 255
};

DEFINE_GRADIENT_PALETTE( My_Heart_Is_Crippled_gp ) {
  0, 255, 199, 93,
  61, 255, 199, 93,
  61, 190, 107, 55,
  96, 190, 107, 55,
  96, 135, 45, 29,
  104, 135, 45, 29,
  104,  92, 10, 12,
  181,  92, 10, 12,
  181,  58,  1,  3,
  255,  58,  1,  3
};


DEFINE_GRADIENT_PALETTE( white_gp ) {
  0, 30, 30, 30,
  50, 50, 50, 50,
  127, 30, 30, 30,
  255, 0, 0, 0,
};

DEFINE_GRADIENT_PALETTE( red_gp ) {
  0, 0, 0, 0,
  75, 40, 0, 0,
  150, 0, 0, 0,
  255, 0, 0, 0,
};

// Gradient palette "Caribbean_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/vh/tn/Caribbean.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 144 bytes of program space.

DEFINE_GRADIENT_PALETTE( Caribbean_gp ) {
  0,    0,  0, 43,
  24,   0,  5, 85,
  36,   0, 10, 106,
  48,   1, 22, 130,
  60,   1, 32, 147,
  72,   1, 57, 138,
  84,   1, 79, 138,
  97,   1, 91, 147,
  109,   3, 104, 156,
  121,  12, 104, 166,
  133,  17, 118, 176,
  133,  17, 118, 138,
  145,  35, 118, 176,
  157,  64, 118, 176,
  170,  82, 133, 156,
  182,  88, 149, 186,
  194,  95, 175, 207,
  206, 110, 184, 230,
  213, 173, 203, 242,
  218, 206, 213, 242,
  218,  33, 53, 14,
  219,  44, 62, 14,
  219,  46, 62, 14,
  221,  58, 68, 18,
  223,  75, 73, 24,
  225,  95, 79, 27,
  228, 110, 91, 27,
  230, 126, 97, 31,
  233, 144, 111, 27,
  235, 163, 118, 27,
  237, 184, 125, 26,
  237, 184, 125, 27,
  240, 206, 141, 25,
  242, 229, 149, 25,
  245, 242, 166, 24,
  255, 242, 223, 197
};

// Gradient palette "rgi_03_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/ds/rgi/tn/rgi_03.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 20 bytes of program space.

DEFINE_GRADIENT_PALETTE( rgi_03_gp ) {
  0,   247, 79, 17,
  95,   80, 27, 32,
  191,  11,  3, 52,
  223,  16,  4, 45,
  255,  22,  6, 38
};

// Gradient palette "goddess_moon_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/pj/1/tn/goddess-moon.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 40 bytes of program space.

DEFINE_GRADIENT_PALETTE( goddess_moon_gp ) {
    0,   2,  3, 20,
   40,   2,  3, 27,
   76,   2,  3, 19,
  101,   5,  2, 73,
  137,   5,  2, 73,
  158, 255,255,174,
  175,  11,  4,127,
  221,   5,  2, 73,
  249,   2,  3, 27,
  255,   2,  3, 27};

// Gradient palette "slyvana_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/pj/6/tn/slyvana.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 28 bytes of program space.

DEFINE_GRADIENT_PALETTE( slyvana_gp ) {
    0,   7,162,138,
   63, 232,241,203,
  117,  82,205, 92,
  170,   7,162,138,
  196, 232,241,203,
  244,   2, 48, 95,
  255,   2, 48, 95};



const TProgmemRGBGradientPalettePtr gGradientPalettes[] = {
  slyvana_gp,
  goddess_moon_gp,
  Coral_reef_gp,
  fire_gp,
  My_Heart_Is_Crippled_gp,
  white_gp,
  red_gp,
  Caribbean_gp,
  rgi_03_gp,
};

// Count of how many gradients are defined:
const uint8_t gGradientPaletteCount =
  sizeof( gGradientPalettes) / sizeof( TProgmemRGBGradientPalettePtr );

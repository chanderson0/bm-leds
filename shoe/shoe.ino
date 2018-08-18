#include <I2C.h>
#include <LIS331.h>
#include <FastLED.h>

#define ACCELEROMETER_ADDR 0x19
#define NUM_LEDS 96

#define SCENE_TIME 35
#define PALETTE_TIME 13

// Note - these are exactly backwards from the wiring
const uint8_t kLedData[] = {
    0, 0, 0,
    0, 0, 23,
    0, 0, 46,
    0, 0, 69,
    0, 0, 92,
    0, 0, 115,
    0, 0, 139,
    0, 0, 162,
    0, 0, 185,
    0, 0, 208,
    0, 0, 231,
    0, 0, 255,
    24, 72, 231,
    24, 0, 208,
    24, 0, 185,
    24, 0, 162,
    24, 0, 139,
    24, 0, 115,
    24, 0, 92,
    24, 0, 69,
    24, 0, 46,
    24, 0, 23,
    24, 0, 0,
    56, 0, 0,
    56, 0, 23,
    56, 0, 46,
    56, 0, 69,
    56, 0, 92,
    56, 0, 115,
    56, 0, 139,
    56, 0, 162,
    56, 0, 185,
    56, 0, 208,
    56, 72, 231,
    56, 109, 231,
    24, 109, 231,
    30, 182, 231,
    35, 218, 231,
    40, 255, 231,
    46, 218, 231,
    51, 182, 231,
    56, 109, 231,
    88, 0, 231,
    88, 0, 208,
    88, 0, 185,
    88, 0, 162,
    88, 0, 139,
    88, 0, 115,
    88, 0, 92,
    88, 0, 69,
    88, 0, 46,
    88, 0, 23,
    88, 0, 0,
    116, 0, 0,
    116, 0, 23,
    116, 0, 46,
    116, 0, 69,
    116, 0, 92,
    116, 0, 115,
    116, 0, 139,
    116, 0, 162,
    116, 0, 185,
    116, 0, 208,
    116, 0, 231,
    116, 0, 255,
    138, 0, 231,
    138, 0, 208,
    138, 0, 185,
    138, 0, 162,
    138, 0, 139,
    138, 0, 115,
    138, 0, 92,
    138, 0, 69,
    138, 0, 46,
    138, 0, 23,
    138, 0, 0,
    170, 0, 69,
    170, 0, 92,
    170, 0, 115,
    170, 0, 139,
    170, 0, 162,
    170, 0, 185,
    170, 0, 208,
    170, 0, 231,
    170, 0, 255,
    219, 0, 231,
    219, 0, 208,
    219, 0, 185,
    219, 0, 162,
    219, 0, 139,
    219, 0, 115,
    219, 0, 92,
    219, 0, 69,
    219, 0, 46,
    219, 0, 23,
    219, 0, 0};

const unsigned long kDebouceFootstepMillis = 200;
const unsigned long kAnimFootstepMillis = 1000;
const float kChangeFactor = 2.0;

LIS331 accelerometer{ACCELEROMETER_ADDR};
CRGBArray<NUM_LEDS> leds;

extern const TProgmemRGBGradientPalettePtr gGradientPalettes[];
extern const uint8_t gGradientPaletteCount;
CRGBPalette16 curPalette(gGradientPalettes[0]);
CRGBPalette16 targetPalette(gGradientPalettes[0]);
uint8_t curPaletteIdx = 0;

void print_err(byte err_code)
{
    switch (err_code)
    {
    case START:
        Serial.println(F("START"));
        break;
    case REPEATED_START:
        Serial.println(F("REPEATED_START"));
        break;
    case MT_SLA_ACK:
        Serial.println(F("MT_SLA_ACK"));
        break;
    case MT_SLA_NACK:
        Serial.println(F("MT_SLA_NACK"));
        break;
    case MT_DATA_ACK:
        Serial.println(F("MT_DATA_ACK"));
        break;
    case MT_DATA_NACK:
        Serial.println(F("MT_DATA_NACK"));
        break;
    case MR_SLA_ACK:
        Serial.println(F("MR_SLA_ACK"));
        break;
    case MR_SLA_NACK:
        Serial.println(F("MR_SLA_NACK"));
        break;
    case MR_DATA_ACK:
        Serial.println(F("MR_DATA_ACK"));
        break;
    case MR_DATA_NACK:
        Serial.println(F("MR_DATA_NACK"));
        break;
    case LOST_ARBTRTN:
        Serial.println(F("LOST_ARBTRTN"));
        break;
    case E_OK:
        Serial.println(F("E_OK"));
        break;
    case E_WRONG_INTERRUPT:
        Serial.println(F("E_WRONG_INTERRUPT"));
        break;
    case E_NUM_TOO_BIG:
        Serial.println(F("E_NUM_TOO_BIG"));
        break;
    case E_WRONG_SCALE:
        Serial.println(F("E_WRONG_SCALE"));
        break;
    default:
        Serial.print(F("Unknown error message: 0x"));
        Serial.println(err_code, HEX);
        break;
    }
}

void setup()
{
    Serial.begin(230400);

    I2c.begin();
    I2c.pullup(1);
    I2c.setSpeed(1);   // 400 kHz
    I2c.timeOut(2000); // Reset communication when it's blocking the system for more than 2 seconds

    byte err = accelerometer.setPowerMode(LIS331::PowerMode::normalMode);
    print_err(err);
    err = accelerometer.setDataRate(LIS331::DataRate::odr1000Hz);
    print_err(err);
    err = accelerometer.setBDUEnabled(true);
    print_err(err);
    err = accelerometer.setScale(LIS331::Scale::scale24g);
    print_err(err);
    err = accelerometer.setXEnabled(true);
    print_err(err);
    err = accelerometer.setYEnabled(true);
    print_err(err);
    err = accelerometer.setZEnabled(true);
    print_err(err);

    FastLED.addLeds<WS2811, 11, RGB>(leds, NUM_LEDS);
    FastLED.setMaxPowerInVoltsAndMilliamps(5, 300);
    FastLED.setBrightness(32);

    for (int i = 0; i < NUM_LEDS; i++)
    {
        leds[i] = 0;
    }
    FastLED.show();
}

uint8_t loopNum = 0;
float slowMag = 0;
float fastMag = 0;

int16_t xAvg = 0, yAvg = 0, zAvg = 0;

unsigned long now;
unsigned long lastFootstep = 0;

void footstepAnim()
{
    uint8_t hue = (now - lastFootstep) / 3;
    for (int i = 0; i < NUM_LEDS; i++)
    {
        //    if (i % 2 == 0) {
        //      leds[i] = CHSV(hue++, 255, 255);
        //    }
        if (i < (now - lastFootstep) / 5)
        {
            leds[i] = CHSV(hue--, 255, 255);
        }
    }
}

void advancePalette()
{
    curPaletteIdx = addmod8(curPaletteIdx, 1, gGradientPaletteCount);
    targetPalette = gGradientPalettes[curPaletteIdx];
}

void loop()
{
    now = millis();

    //    int16_t x = 0, y = 0, z = 0;
    //    accelerometer.getXValue(x);
    //    accelerometer.getYValue(y);
    //    accelerometer.getZValue(z);

    float x, y, z;
    uint8_t o = accelerometer.getAxisValuesG(x, y, z);
    float mag = sqrt(x * x + y * y + z * z);
    slowMag += (mag - slowMag) * 0.01;

    float factor = 0.05;
    if (mag > fastMag)
    {
        factor = 0.15;
    }
    fastMag += (mag - fastMag) * factor;

    EVERY_N_SECONDS(PALETTE_TIME)
    {
        advancePalette();
    }

    EVERY_N_MILLISECONDS(10)
    {
        nblendPaletteTowardPalette(curPalette, targetPalette, 64);
    }

    for (uint8_t i = 0; i < NUM_LEDS; ++i)
    {
        // uint8_t frac = ledVals[i];
        // uint8_t h = frac + now / 20;
        // uint8_t s = 255 - max(min(255 * abs(fastMag - 1.0), 255), 0); //scale8(frac + now / 10, 127);
        // uint8_t v = 255;                                              //max(min(255 * mag, 255), 0);

        uint8_t angle = kLedData[(NUM_LEDS - i - 1) * 3 + 0];
        uint8_t radius = kLedData[(NUM_LEDS - i - 1) * 3 + 1];
        uint8_t height = kLedData[(NUM_LEDS - i - 1) * 3 + 2];

        uint8_t v = (uint8_t)(((uint16_t)height + (uint16_t)angle) / 2) + now / 10;

        leds[i] = ColorFromPalette(curPalette, v);
    }

    //    int16_t mag = sqrt16(x*x + y*y + z*z) * 10;
    //    slowMag += (mag - slowMag) * 0.01;
    //    fastMag += (mag - fastMag) * 0.10;

    //    int16_t magDiff = abs(fastMag - kBaselineAccel);

    //    Serial.print(F("Mag: "));
    //    Serial.print(mag);
    //    Serial.println();
    //    Serial.print(F("Slow Mag: "));
    //    Serial.print(slowMag);
    //    Serial.println();
    //    Serial.print(F("Fast Mag: "));
    //    Serial.print(fastMag);
    //    Serial.println();

    // Let it come up to normal
    //    if (now < 2000) {
    //      return;
    //    }
    //
    //    bool changeFound = false;
    //    if (mag > slowMag * kChangeFactor) {
    //      changeFound = true;
    //    }
    //
    //    bool footstep = false;
    //    if (changeFound && now - lastFootstep > kDebouceFootstepMillis) {
    //      lastFootstep = now;
    //    }
    //
    //    if (now - lastFootstep < kAnimFootstepMillis) {
    //      footstepAnim();
    //    } else {
    //      for (uint8_t i = 0; i < NUM_LEDS; ++i) {
    //        leds[i] = CHSV(ledVals[i] + now / 20, 255 - scale8(ledVals[i] + now / 10, 127), 255);
    //      }
    //    }

    FastLED.show();
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
    0, 0, 0, 0,    
    127, 90, 90, 90,
    255, 255, 255, 255,
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

// Gradient palette "goddess_moon_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/pj/1/tn/goddess-moon.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 40 bytes of program space.

DEFINE_GRADIENT_PALETTE(goddess_moon_gp){
    0, 2, 3, 20,
    40, 2, 3, 27,
    76, 2, 3, 19,
    101, 5, 2, 73,
    137, 5, 2, 73,
    158, 255, 255, 174,
    175, 11, 4, 127,
    221, 5, 2, 73,
    249, 2, 3, 27,
    255, 2, 3, 27};

// Gradient palette "slyvana_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/pj/6/tn/slyvana.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 28 bytes of program space.

DEFINE_GRADIENT_PALETTE(slyvana_gp){
    0, 7, 162, 138,
    63, 232, 241, 203,
    117, 82, 205, 92,
    170, 7, 162, 138,
    196, 232, 241, 203,
    244, 2, 48, 95,
    255, 2, 48, 95};

// Gradient palette "gr66_hult_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/hult/tn/gr66_hult.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 12 bytes of program space.

DEFINE_GRADIENT_PALETTE(gr66_hult_gp){
    0, 247, 176, 247,
    122, 190, 53, 194,
    255, 42, 5, 1};

const TProgmemRGBGradientPalettePtr gGradientPalettes[] = {
    white_gp,
    gr66_hult_gp,
    slyvana_gp,
    goddess_moon_gp,
    fire_gp,
    My_Heart_Is_Crippled_gp,
    rgi_03_gp,
};

// Count of how many gradients are defined:
const uint8_t gGradientPaletteCount =
    sizeof(gGradientPalettes) / sizeof(TProgmemRGBGradientPalettePtr);

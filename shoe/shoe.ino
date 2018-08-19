#include <I2C.h>
#include <LIS331.h>
#include <FastLED.h>

#define ACCELEROMETER_ADDR 0x19
#define NUM_LEDS 96

#define SCENE_TIME 35
#define PALETTE_TIME 20

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

// Gradient palette "sky_32_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/rafi/tn/sky-32.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 20 bytes of program space.

DEFINE_GRADIENT_PALETTE( sky_32_gp ) {
    0, 203,128,151,
   51, 252,176,212,
  127, 229,161,228,
  219, 182,141,240,
  255, 117,124,223};

// Gradient palette "a_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/ds9/tn/a.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 28 bytes of program space.

DEFINE_GRADIENT_PALETTE( a_gp ) {
    0,   0,  0,  0,
   31,   0, 55,  0,
   63,   0,255, 45,
  127, 255,  0,255,
  163, 255,  0, 45,
  196, 255,  0,  0,
  255, 255,255,  0};

// Gradient palette "es_seadreams_06_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/es/sea_dreams/tn/es_seadreams_06.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 20 bytes of program space.

DEFINE_GRADIENT_PALETTE( es_seadreams_06_gp ) {
    0, 237,207,197,
   45, 224,187,174,
  178,  46,199,190,
  242,   1, 96,172,
  255,   1, 96,172};

  // Gradient palette "es_autumn_05_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/es/autumn/tn/es_autumn_05.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 12 bytes of program space.

DEFINE_GRADIENT_PALETTE( es_autumn_05_gp ) {
    0, 208, 13,  5,
  204, 255,213,156,
  255, 255,213,156};

  // Gradient palette "es_rosa_68_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/es/rosa/tn/es_rosa_68.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 16 bytes of program space.

DEFINE_GRADIENT_PALETTE( es_rosa_68_gp ) {
    0, 255,  2,144,
  127, 255,111,210,
  188, 255,168,230,
  255, 255,209,242};

// Gradient palette "knoza_16_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/gacruxa/knoza/tn/knoza-16.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 68 bytes of program space.

DEFINE_GRADIENT_PALETTE( knoza_16_gp ) {
    0,   2,  1,  1,
    0,   2,  1,  1,
    2, 237,186,  1,
   49, 237,186,  1,
   51, 237,130, 46,
   98, 237,130, 46,
  101, 115,  1,  1,
  116, 115,  1,  1,
  118,  56, 56,237,
  136,  56, 56,237,
  137, 115,  1,  1,
  153, 115,  1,  1,
  154, 237,130, 46,
  203, 237,130, 46,
  204, 237,186,  1,
  252, 237,186,  1,
  255,   2,  1,  1};

// Gradient palette "vilani_20_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/gacruxa/vilani/tn/vilani-20.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 88 bytes of program space.

DEFINE_GRADIENT_PALETTE( vilani_20_gp ) {
    0,  90,105, 93,
    2, 247,248,247,
   27,  90,  2,123,
   28, 247,248,247,
   54,  90,  2,123,
   55, 247,248,247,
   83,  90,  2,123,
   84, 247,248,247,
  111,  90,  2,123,
  112, 247,248,247,
  140,  90,  2,123,
  140, 247,248,247,
  168,  90,  2,123,
  169, 247,248,247,
  178, 173, 99,192,
  193,  90,  2,123,
  195, 247,248,247,
  221,  90,  2,123,
  224, 247,248,247,
  249,  90,  2,123,
  252, 247,248,247,
  255,  45, 58, 48};

  // Gradient palette "higuey_17_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/gacruxa/higuey/tn/higuey-17.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 48 bytes of program space.

DEFINE_GRADIENT_PALETTE( higuey_17_gp ) {
    0, 220, 85, 47,
   88, 220, 85, 47,
   89, 194,225,255,
  100, 194,225,255,
  101,   1,  1,  1,
  114,   1,  1,  1,
  141,   1,  1,  1,
  152,   1,  1,  1,
  152, 194,225,255,
  166, 194,225,255,
  166,  60, 57,221,
  255,  60, 57,221};

// Gradient palette "higuey_22_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/gacruxa/higuey/tn/higuey-22.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 48 bytes of program space.

DEFINE_GRADIENT_PALETTE( higuey_22_gp ) {
    0, 167, 75,170,
   89, 167, 75,170,
   89, 194,225,255,
  100, 194,225,255,
  101,   1,  1,  1,
  114,   1,  1,  1,
  141,   1,  1,  1,
  152,   1,  1,  1,
  152, 194,225,255,
  166, 194,225,255,
  166,  60,178,115,
  255,  60,178,115};


  // Gradient palette "Pastel_Rainbow_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/ggr/tn/Pastel_Rainbow.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 408 bytes of program space.

DEFINE_GRADIENT_PALETTE( Pastel_Rainbow_gp ) {
    0, 255,135,123,
    2, 255,141,123,
    5, 255,147,123,
    7, 255,152,123,
   10, 255,159,123,
   12, 255,166,123,
   15, 255,173,123,
   17, 255,180,123,
   20, 255,186,123,
   22, 255,193,123,
   25, 255,201,123,
   27, 255,209,123,
   30, 255,217,123,
   32, 255,223,123,
   35, 255,231,123,
   37, 255,239,123,
   40, 255,248,123,
   42, 252,255,123,
   45, 244,255,123,
   47, 234,255,123,
   50, 224,255,123,
   53, 215,255,123,
   55, 206,255,123,
   58, 199,255,123,
   60, 190,255,123,
   63, 182,255,123,
   65, 173,255,123,
   68, 165,255,123,
   70, 159,255,123,
   73, 152,255,123,
   75, 144,255,123,
   78, 137,255,123,
   80, 130,255,123,
   83, 125,255,123,
   85, 120,255,125,
   88, 120,255,132,
   31, 120,241,255,
  133, 120,233,255,
  136, 120,227,255,
  138, 120,219,255,
  141, 120,211,255,
  143, 120,203,255,
  146, 120,195,255,
  148, 120,189,255,
  151, 120,182,255,
  154, 120,175,255,
  156, 120,168,255,
  159, 120,161,255,
  161, 120,156,255,
  164, 120,149,255,
  166, 120,142,255,
  169, 120,136,255,
  171, 125,135,255,
  174, 130,135,255,
  176, 137,135,255,
  179, 144,135,255,
  181, 152,135,255,
  184, 159,135,255,
  186, 165,135,255,
  189, 173,135,255,
  191, 182,135,255,
  194, 190,135,255,
  196, 199,135,255,
  199, 206,135,255,
  201, 215,135,255,
  204, 224,135,255,
  207, 234,135,255,
  209, 244,135,255,
  212, 252,135,255,
  214, 255,135,247,
  217, 255,135,237,
  219, 255,135,228,
  222, 255,135,219,
  224, 255,135,212,
  227, 255,135,203,
  229, 255,135,194,
  232, 255,135,186,
  234, 255,135,178,
  237, 255,135,172,
  239, 255,135,164,
  242, 255,135,156,
  244, 255,135,149,
  247, 255,135,142,
  249, 255,135,137,
  252, 255,135,130,
  255, 255,135,123};

  // Gradient palette "Sunset_Real_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/nd/atmospheric/tn/Sunset_Real.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 28 bytes of program space.

DEFINE_GRADIENT_PALETTE( Sunset_Real_gp ) {
    0, 120,  0,  0,
   22, 179, 22,  0,
   51, 255,104,  0,
   85, 167, 22, 18,
  135, 100,  0,103,
  198,  16,  0,130,
  255,   0,  0,160};



const TProgmemRGBGradientPalettePtr gGradientPalettes[] = {
    Pastel_Rainbow_gp,
    es_seadreams_06_gp,
    higuey_22_gp,
    higuey_17_gp,
    es_rosa_68_gp,
    es_autumn_05_gp,
    knoza_16_gp,
    a_gp,
    Sunset_Real_gp,
    fire_gp,
    My_Heart_Is_Crippled_gp,
    rgi_03_gp,
    gr66_hult_gp,
    slyvana_gp,
    goddess_moon_gp,
    vilani_20_gp,
    white_gp,
    
};

// Count of how many gradients are defined:
const uint8_t gGradientPaletteCount =
    sizeof(gGradientPalettes) / sizeof(TProgmemRGBGradientPalettePtr);

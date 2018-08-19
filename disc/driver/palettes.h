#ifndef palettes_h
#define palettes_h

#include "FastLED.h"

DECLARE_GRADIENT_PALETTE(rainbow_gp);
DECLARE_GRADIENT_PALETTE(Coral_reef_gp);
DECLARE_GRADIENT_PALETTE(fire_gp);
DECLARE_GRADIENT_PALETTE(My_Heart_Is_Crippled_gp);
DECLARE_GRADIENT_PALETTE(white_gp);
DECLARE_GRADIENT_PALETTE(red_gp);
DECLARE_GRADIENT_PALETTE(Caribbean_gp);
DECLARE_GRADIENT_PALETTE(rgi_03_gp);
DECLARE_GRADIENT_PALETTE(Paired_05_gp);

extern const TProgmemRGBGradientPalettePtr gGradientPalettes[];
extern const uint8_t gGradientPaletteCount;

extern const uint8_t GammaCorrection[256];

#endif

#ifndef WC_NEOPICEL_h
#define WC_NEOPIXEL_h
#include <CubeCell_NeoPixel.h>
#include "MyConfig.h"
#if defined(PIN_WS2812) && defined(NUM_WS2812)
extern CubeCell_NeoPixel pixels;
#endif

typedef enum {
  LM_NONE    = 0,
  LM_FREE    = 1,
  LM_BUSY    = 2,
  LM_FREE2   = 11,
  LM_BUSY2   = 12,
  LM_BUTTON  = 100
} NEOPIXEL_MODE_t; 

void startNeopixel();
void nextAnimationNeopixel();
void setNeopixelLoop();
void setNeopixel( NEOPIXEL_MODE_t _mode);
#endif

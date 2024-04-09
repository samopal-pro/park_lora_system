#include "WC_neopixel.h"
#if defined(PIN_WS2812) && defined(NUM_WS2812)
CubeCell_NeoPixel pixels(NUM_WS2812, PIN_WS2812, NEO_GRB + NEO_KHZ800);
#endif

uint16_t countNeopixel   = 0;
NEOPIXEL_MODE_t modeNeopixel = LM_NONE, saveModeNeopixel = LM_NONE;


void startNeopixel(){
#if defined(PIN_WS2812) && defined(NUM_WS2812)
   pixels.begin();
   delay(2000);
   uint16_t _hue = 0;
   uint16_t _inc = 65536/NUM_WS2812;
   for( int i=0; i<40; i++){
      for( int j=0; j<NUM_WS2812; j++){
         uint16_t h = _hue + _inc*j;
         pixels.setPixelColor(j, pixels.ColorHSV(h, 255, 100));     
      }
      pixels.show();
      _hue += _inc;
//      Serial.println(_hue);
      delay(50);
   }
#else
   Serial.println(F("WS2812 LED is disable"));   
#endif  
}

void setNeopixel( NEOPIXEL_MODE_t _mode){
   if( modeNeopixel == _mode )return;
   saveModeNeopixel = modeNeopixel;  
   modeNeopixel     = _mode;
   countNeopixel    = 0;
#if defined(PIN_WS2812) && defined(NUM_WS2812)
   switch( _mode ){
      case LM_FREE:
         for( int i=0; i< NUM_WS2812; i++)pixels.setPixelColor(i, pixels.Color(0, 255, 0));           
         break;
      case LM_BUSY:
         for( int i=0; i< NUM_WS2812; i++)pixels.setPixelColor(i, pixels.Color(255, 0, 0));           
         break;
      case LM_BUTTON:
         for( int i=0; i< NUM_WS2812; i++)pixels.setPixelColor(i, pixels.Color(0, 0, 255));           
         break;
         
   }
   pixels.show();
   Serial.print(F("Set NeoPixel Mode: "));
   Serial.println(modeNeopixel);
   
#else
   Serial.println(F("WS2812 LED is disable"));   
#endif  
}

void setNeopixelLoop(){
#if defined(PIN_WS2812) && defined(NUM_WS2812)
/*
   uint32_t _ms = millis();
   if(_ms > endNeopixelTM || _ms < startNeopixelTM ){
        nextAnimationNeopixel();
        return;
   } 
*/   
//   Serial.print(F("Loop NeoPixel : "));
//   Serial.println(modeNeopixel);
  pixels.begin();
//  uint8_t _b ;   
   switch( modeNeopixel ){
      case LM_FREE:
         for( int i=0; i< NUM_WS2812; i++)pixels.setPixelColor(i, pixels.Color(128, 128, 128));
         pixels.show();
         delay(100);           
         for( int i=0; i< NUM_WS2812; i++)pixels.setPixelColor(i, pixels.Color(0, 255, 0));           
         pixels.show();
         break;
      case LM_BUSY:
         if( countNeopixel%3 == 0 ){
            for( int i=0; i< NUM_WS2812; i++)pixels.setPixelColor(i, 0);
            pixels.show();
            delay(100);
         }           
         for( int i=0; i< NUM_WS2812; i++)pixels.setPixelColor(i, pixels.Color(255, 0, 0));           
         pixels.show();
         break;
      case LM_BUTTON:
         for( int i=0; i< NUM_WS2812; i++)pixels.setPixelColor(i, 0);
         pixels.show();
         delay(100);           
         for( int i=0; i< NUM_WS2812; i++)pixels.setPixelColor(i, pixels.Color(0, 0, 255));           
         pixels.show();
         setNeopixel(saveModeNeopixel);
         break;
      default :   
         for( int i=0; i< NUM_WS2812; i++)pixels.setPixelColor(i, 0);           
   }
//  pixels.show();
  countNeopixel++;
#endif  
}





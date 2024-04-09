#include "MySensorSR04.h"

float L_sr04 = NAN, H_sr04 = NAN;
bool Busy_sr04 = false;

float distArray[SAMPLE_LEN];

bool getDistanceSR04HAL(){
  int x = random(0,2);
  if( x == 1 ){
      L_sr04    = L_Ground - 1000;
      H_sr04    = L_Ground - L_sr04;
      Busy_sr04 = true;
  }
  else {
      L_sr04    = L_Ground;
      H_sr04    = 0;
      Busy_sr04 = false;  
  }
 
#if defined(DEBUG_SENSORS)      
   Serial.print(F("HAL SR04 Distance(mm)="));
   Serial.print(L_sr04);
   Serial.print(F(" High(mm)="));
   Serial.print(H_sr04);
   Serial.print(F(" Busy="));
   Serial.println(Busy_sr04);
#endif    
   return true;
}


#include "MySensorSR04.h"

float L_sr04 = NAN, H_sr04 = NAN;

#if defined(NAN_VALUE_FREE)      
bool Busy_sr04 = false; 
#else   
bool Busy_sr04 = true; 
#endif

float distArray[SAMPLE_LEN];

bool getDistanceSR04HAL(){
  bool ret = false;

   pinMode(PIN_TRIG, OUTPUT); 
   pinMode(PIN_ECHO, INPUT); 
// Померили сразу выборку значений   
   float distAvg = 0, distMin = 99999, distMax = 0;
   for( int i=0; i<SAMPLE_LEN; i++){
      digitalWrite(PIN_TRIG, LOW);
      delayMicroseconds(2);
      digitalWrite(PIN_TRIG, HIGH);
      delayMicroseconds(10);
      digitalWrite(PIN_TRIG, LOW);
      uint32_t _dur = pulseIn(PIN_ECHO, HIGH);
      float   _dist = _dur/5.8;
      distArray[i]  = _dist;
      distAvg += _dist;
      if(  distMin > _dist)distMin = _dist;
      if(  distMax < _dist)distMax = _dist;
      delay(50);
   }
// Вычислем среднеквадратичное отклонение   
   distAvg /= SAMPLE_LEN;
   float distDiv = 0;
   for( int i=0; i<SAMPLE_LEN; i++){
      distDiv += (distArray[i]-distAvg)*(distArray[i]-distAvg);
   }
   distDiv /= SAMPLE_LEN;
   distDiv = sqrt(distDiv);
// Вычисляем достовернсть 
   if( distDiv/distAvg < RELIABILITY_PROC ){
      L_sr04 = distAvg;
      if( L_sr04  > L_Ground )H_sr04 = 0;
      else H_sr04 = L_Ground - L_sr04;
      if( L_sr04 > L_Ground + L_Busy )Busy_sr04 = true;
      else if( H_sr04 > L_Busy)Busy_sr04 = true;
      else Busy_sr04 = false;
      ret = true;
   }
   else {
      L_sr04    = NAN;
      H_sr04    = NAN;
#if defined(NAN_VALUE_FREE)   
      ret       = true;   
      Busy_sr04 = false; 
#elif defined(NAN_VALUE_BUSY) 
      ret = true; 
      Busy_sr04 = true; 
#elif defined(NAN_VALUE_IGNORE) 
      ret = false;    
#endif   
   }

#if defined(DEBUG_SENSORS)      
   Serial.print(F("HAL SR04 Distance(mm)="));
   Serial.print(L_sr04);
   Serial.print(F(" High(mm)="));
   Serial.print(H_sr04);
   Serial.print(F(" Busy="));
   Serial.print(Busy_sr04);
//   Serial.print(F(" Min="));
//   Serial.print(distMin,0);
//   Serial.print(F(" Max="));
//   Serial.print(distMax,0);
   Serial.print(F(" Avg="));
   Serial.print(distAvg,0);
   Serial.print(F(" Div="));
   Serial.println(distDiv,0);
//   for( int i=0; i<SAMPLE_LEN; i++){
//      Serial.print(F(" "));
//      Serial.print(distArray[i],0);
//   }
   Serial.println();
#endif    
   return ret;
}


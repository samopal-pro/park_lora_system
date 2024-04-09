#ifndef MY_SENSOR_SR04_h
#define MY_SENSOR_SR04_h
#include <Arduino.h>
#include "MyConfig.h"

#define SAMPLE_LEN 10
#define RELIABILITY_PROC 0.15

extern float L_Min, L_Max, L_Ground, L_Busy;
extern float L_sr04,H_sr04;
extern bool Busy_sr04;

bool getDistanceSR04HAL();

#endif


#ifndef WC_TASK_h
#define WC_TASK_h
#include "WC_EEPROM.h"
#include "WC_Lora.h"
#include "WC_Json.h"
#include "src/SButton.h"
#include "src/SLed.h"
#include "WC_HTTP.h"
#include "crmlogo.h"
#include "WC_Net.h"
#include "MyConfig.h"

#include "src/Neopixel/Adafruit_NeoPixel.h"


#include <HTTPClient.h>
#include <time.h>





extern NTPClient ntpClient;
extern StaticJsonDocument<JSON_MSG_SIZE> j_msg;
extern bool isMQTT, isAP;
extern SemaphoreHandle_t loraMutex;
extern uint32_t idCount32;

extern char _stat_setup[];



void tasksStart();
void taskWiFi(void *pvParameters);
void taskHttpd(void *pvParameters);
void httpSend();
void listDir(const char *dirname, uint8_t levels = 0);
void taskButton1(void *pvParameters);
void taskNeopixel(void *pvParameters);
void startNeopixel();



#endif

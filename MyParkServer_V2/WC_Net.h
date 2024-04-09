#ifndef WC_NET_h
#define WC_NET_h
#include <WiFi.h>
#include "src/Ethernet1/Ethernet1.h"
#include <NTPClient.h>    //https://github.com/arduino-libraries/NTPClient
#ifdef ETHERNET_W5500
//extern SPIClass *SPI1;

#include "src/Ethernet1/EthernetUdp.h"
//#include <EthernetUdp.h>
#else
#include <WiFiUDP.h>
#endif

#include "WC_EEPROM.h"  
#include "WC_Task.h"
#define TM_WIFI_CONNECT 30000
//extern SPIClass *SPI1;
typedef enum
{
  WS_WIFI_OFF        = 0,
  WS_WIFI_WAIT       = 1,
  WS_WIFI_ON         = 2,
  WS_WIFI_AP_MODE    = 3,
  WS_WIFI_NOT_CONFIG = -1,
} WIFI_STAT_t;

  
extern bool isAP, wifi_correct;
extern int wifiRssi;



void ethernetInit();
void ethernetReconnect();
bool ethernetCheck();

void wifiInit();
bool wifiCheck();
void spiDisable();



#endif

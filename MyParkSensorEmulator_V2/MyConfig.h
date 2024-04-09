#ifndef _CONFIG_h
#define _CONFIG_h

#define CURRENT_FIRMWARE_TITLE    F("MyLoRaNode")
#define CURRENT_FIRMWARE_VERSION  F("3.0rc5")
#define CURRENT_CONFIG_VERSION    "0.01"
#define CURRENT_NODE              0xffff
#define DEFAULT_NODE_NAME         "Sens1"
#define ROOT_NODE                 0

#define DEBUG_SENSORS
#define CONFIG1


// Конфигурация с датчиком SR04M в режиме TRIG/ECHO 
#if defined(CONFIG1)

#define IS_SR04M_NORNAL
#define PIN_TRIG    P4_5 //TX2
#define PIN_ECHO    P4_4 //RX2
#define PIN_WS2812  GPIO7 
#define NUM_WS2812  9
#define PIN_BTN     GPIO1
#define CONTROLLER_TYPE   F("MyLoRa_Park_SR04M")

#endif




#endif

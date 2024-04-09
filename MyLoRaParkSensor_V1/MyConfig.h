#ifndef _CONFIG_h
#define _CONFIG_h

#define CURRENT_FIRMWARE_TITLE    F("MyLoRaNode")
#define CURRENT_FIRMWARE_VERSION  F("3.0rc7")
#define CURRENT_CONFIG_VERSION    "0.01"
#define CURRENT_NODE              0xffff
#define DEFAULT_NODE_NAME         "Sens1"
#define ROOT_NODE                 0

#define DEBUG_SENSORS
#define CONFIG1
// Доработка 12.03.24]
// Три параметра на замену FAIL_SENSOR
// Активным может быть только один из трех
//#define NAN_VALUE_FREE //"Свободно" если значение дистанции NAN (посылают глючные сенсоры)
//#define NAN_VALUE_BUSY //"Занято" если значение дистанции NAN
#define NAN_VALUE_IGNORE //Если дистанция NAN оставить предыдущее значение

#define IS_SR04M_NORNAL
#define PIN_TRIG P4_5 //TX2
#define PIN_ECHO P4_4 //RX2
#define PIN_WS2812 GPIO7
#define NUM_WS2812 9
#define PIN_BTN GPIO1
#define CONTROLLER_TYPE F("MyLoRa_Park_SR04M")






#endif

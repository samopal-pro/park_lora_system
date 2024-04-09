#ifndef WC_EEPROM_h
#define WC_EEPROM_h
#include <arduino.h>
#include <ArduinoJson.h> //https://github.com/bblanchon/ArduinoJson
#include <SPIFFS.h>
#include <FS.h>
#include "MyConfig.h"


//extern JsonObject j_ap, j_wifi, j_mqtt;
extern DynamicJsonDocument j_config;
extern DynamicJsonDocument j_attributes;
extern DynamicJsonDocument j_nodes;
extern DynamicJsonDocument j_servers;
extern String chipID;
extern char strID[];

void eepromBegin();
void eepromDefault();
void eepromDefaultIP();
void eepromDefaultMQTT();
void eepromDefaultWIFI();
void eepromDefaultESP();

void eepromRead();
void eepromSave();

void attributesRead();
void attributesSave();
void attributesDefault();

void nodesRead();
void nodesSave();
bool nodesSynchronization();
void nodesPrint(char *mode);

#endif

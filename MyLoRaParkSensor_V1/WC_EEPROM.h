#ifndef WC_EEPROM_h
#define WC_EEPROM_h
#include <arduino.h>
#include <EEPROM.h>
#include <ArduinoJson.h>
//#include "WC_Sensors.h"
#include "MyConfig.h"

// Телеметрия
#define T_DIST_MM                 F("L")            //Дистанция с датчика расстояния, мм (измеряемое)
#define T_HIGH_MM                 F("H")            //Высота над уровнем земли, мм  (вычисляемое)
#define T_IS_BUSY                 F("Busy")         //Флаг "занято" (вычисляемое)
#define T_BAT_V                   F("Vbat")         //Напряжение на батарее
// Общие атрибуты (настраиваемые)
#define ATT_RPEAT_N               F("Repeat")       //Число повторений отпраыки пакета
#define ATT_TM_SENS_S             F("TMsens")       //Интервал опроса сенсоров, сек
#define ATT_TM_LORA_S             F("TMlora")       //Интервал отправки, сек
#define ATT_TM_LORA_ERR_S         F("TMloraErr")    //Интервал отправки интервал отправки если не получен ответ, сек
#define ATT_TM_ON_S               F("TMon")         //Интервал огрубления включения, сек
#define ATT_TM_OFF_S              F("TMoff")        //Интервал огрубления отключения, сек


#define ATT_L_GROUND_MM           F("Lground")      //Высота установки сенсора (дистанция до земли при отсутсвиее авто), мм
#define ATT_L_MAX_MM              F("Lmax")         //Максималнаая дистанция сенсора (выше ошибка), мм
#define ATT_L_MIN_MM              F("Lmin")         //Минимальная дистанция сенсора (ниже ошибка), мм
#define ATT_L_BUSY_MM             F("Lbusy")        //Дистанция, менее которой считается место занято, мм
#define ATT_NAME                  F("Name")         //Имя сенсора (человеческое)
// Атрибуты сенсора (не изменяемые)
#define ATT_VER                   F("VerFW")        //Версия прошивки
#define ATT_TYPE                  F("Type")         //Тип сенсора
#define ATT_ID                    F("ID")           //Сенсор ID


void      eepromBegin();
void      eepromDefault();
void      eepromRead();
void      eepromSave();
bool  setAttributeJson(const __FlashStringHelper* _name, JsonObject json, float _min, float _max);
float getAttributeJson(const __FlashStringHelper* _name, float _def);

//extern StaticJsonDocument<1024> j_attributes;
extern DynamicJsonDocument j_attributes;

#endif

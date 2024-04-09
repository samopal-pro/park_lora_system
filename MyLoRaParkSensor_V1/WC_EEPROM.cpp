#include "WC_EEPROM.h"
DynamicJsonDocument j_attributes(1024);

/**
   Инициализация EEPROM
*/
void eepromBegin() {
}

/**
   Читаем конфигурацию из EEPROM
*/
void eepromRead() {
// Считываем размер конфигурации  
  uint16_t _size;
  size_t _size1 = sizeof(_size);
  EEPROM.begin(_size1);
  for( int i=0; i<_size1;i++)*((uint8_t*)&_size + i)=EEPROM.read(i);
  EEPROM.end();
  if( _size > 1000 )_size = 0;
  if( _size == 0 ){
     eepromDefault();
     eepromSave();
     return;     
  }
// Считываем конфигурацию в буфер  
  char _data[_size+1];
  EEPROM.begin(_size + _size1); 
  for ( int i=0; i<_size; i++ )_data[i] = (char)EEPROM.read(i + _size1);
  _data[_size] = '\0';
  EEPROM.end();
// Парсим коефигурацию в JSON Объект
  DeserializationError _err = deserializeJson(j_attributes, (const char *)_data);  
  if( !j_attributes.containsKey("Ver") ||  
     j_attributes["Ver"] != CURRENT_CONFIG_VERSION ){
     eepromDefault();
     eepromSave();
     return;         
     }
  
  Serial.print(F("Read EEPROM size="));
  Serial.print(_size);
  Serial.print(F(" data="));
  Serial.println(_data);
//  Serial.print(F(" err="));
//  Serial.println(_err);
}

/**
*  Сохраняем конфигурацию в EEPROM
*/
void eepromSave() { 
  String _data;
  serializeJson(j_attributes, _data);
  uint16_t _size = _data.length(); 
  size_t _size1  = sizeof(_size);
  EEPROM.begin(_size + _size1); 
  for( int i=0; i<_size1; i++)EEPROM.write(i, *((uint8_t*)&_size + i));
  for( int i=0; i<_size;  i++)EEPROM.write(i + _size1, (uint8_t)_data[i]);
  EEPROM.commit();
  EEPROM.end();
  Serial.print(F("Save EEPROM size="));
  Serial.print(_size);
  Serial.print(F(" data="));
  Serial.println(_data);
}

/**
   Сброс конфигурации в значения "по умолчанию"
*/
void eepromDefault() {
   j_attributes.clear();
   j_attributes["Ver"]                   = CURRENT_CONFIG_VERSION;
   j_attributes[ATT_NAME]                = DEFAULT_NODE_NAME;
// Максимальное число повторений отправки пакета
   j_attributes["Shared"][ATT_RPEAT_N]       = 5;
// Интервал между измерениями расстояния , сек 
   j_attributes["Shared"][ATT_TM_SENS_S]     = 1;
// Интервал отправки если статус парковки не изменился ,сек  
   j_attributes["Shared"][ATT_TM_LORA_S]     = 300;
// Интервал отправки если статус парковки не изменился, но подтверждения приема на предыдущую отправку не было, сек
   j_attributes["Shared"][ATT_TM_LORA_ERR_S] = 30;
// Задержка перед сигналом "Занято", сек   
   j_attributes["Shared"][ATT_TM_ON_S]       = 5;
// Задержка перед сигналом "Свободно", сек   
   j_attributes["Shared"][ATT_TM_OFF_S]      = 1;
// Растояние до земли, мм   
   j_attributes["Shared"][ATT_L_GROUND_MM]   = 3700;
// Максимальное расстояние датчика (не используется)   
   j_attributes["Shared"][ATT_L_MAX_MM]      = 4500;
// Минимальное расстояние датчика (не используется)   
   j_attributes["Shared"][ATT_L_MIN_MM]      = 100;
// Длина на которую отличается "занято" от "свободно", мм   
   j_attributes["Shared"][ATT_L_BUSY_MM]     = 600;
}

/*
 * Сохраняем один атрибут в конфигурационной структуре
 * 
 * @param _name - имя атрибута
 * @param json - Json структура откуда брать аргумент
 * @patam _min - минимальное значение для проверки
 * @param _max - максимальное значение для проверки
 * @return true - атрибут успешно сохранился в структуре
 * @return false - атрибут не сохранился в структуре
 */
bool  setAttributeJson(const __FlashStringHelper* _name, JsonObject json, float _min, float _max){
   if( !j_attributes["Shared"].containsKey(_name) ){
       Serial.print(F("!Unknown attribute "));
       Serial.println(_name);
       return false;
   }
   if( !json[_name] ){
//       Serial.print(F("!Not found attribute "));
//       Serial.println(_name);
       return false;
   }
   JsonVariant _val = json[_name].as<JsonVariant>();
   if( !_val.is<float>() ){
       Serial.print(F("!Attribute not numeric "));
       Serial.println(_name);
       return false;   
   }
//   float _valf = _val.as<long>();
   float _f = _val.as<float>();
   if( ( _min >= 0 && _f < _min ) || ( _max >= 0  && _f > _max ) ){
       Serial.print(F("!Attribute error range "));
       Serial.print(_name);
       Serial.print(F(" : "));
//       Serial.print(_min);
//       Serial.print(F(" : "));
//       Serial.print(_max);
//       Serial.print(F(" : "));
       Serial.println(_f);
       return false;      
   }
   if( j_attributes["Shared"][_name] == json[_name] ){
//       Serial.print(F("!Attribute not changed"));
//       Serial.print(_name);
//       Serial.print(F(" : "));
//       Serial.println(_valf);
       return false;      
       
   }
   Serial.print(F("Set attribute "));
   Serial.print(_name);
   Serial.print(F(" : "));
   Serial.println(_f);
   j_attributes["Shared"][_name] = _val;
   return true;
}

/**
 * Читаем атрибут в конфигурационной структуре
 * 
 * @param _name - имя атрибута
 * @param _def - возвращаемое значение, если атрибут не найден 
 * @return - значение атрибута
 */
float getAttributeJson(const __FlashStringHelper* _name, float _def){

//float  getAttributeJson(const char *_name,  float _default){
   if( !j_attributes["Shared"][_name] )return _def;
   return j_attributes["Shared"][_name].as<float>(); 
}

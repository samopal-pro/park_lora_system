 #include "WC_EEPROM.h"
DynamicJsonDocument j_config(4096);
DynamicJsonDocument j_attributes(8192);
DynamicJsonDocument j_gates(20000);

String chipID;
char strID[16];


/**
   Инициализация EEPROM
*/
void eepromBegin() {
     chipID = strID;
//     chipID = "ESP" + String((uint32_t)ESP.getEfuseMac(),HEX);
//     Serial.print("ChipID1: ");
//     Serial.println(chipID);
}

/**
   Читаем конфигурацию из EEPROM
*/
void eepromRead() {
   File f = SPIFFS.open("/config.json", FILE_READ);
   deserializeJson(j_config, f);
   f.close();
//   Serial.print("MQTT="); 
//   Serial.println((const char *)j_config["mqtt"]["server"]);
   String s;
   serializeJson(j_config, s);   
   Serial.println(s);

}


/**
   Сохраняем конфигурацию в EEPROM
*/
void eepromSave() {
   File f = SPIFFS.open("/config.json", FILE_WRITE);
   serializeJson(j_config, f);
   f.close();
}


/**
   Установка общих парараметров "по умолчанию"
*/
void eepromDefaultESP(){
  j_config["esp"]["id"]        = strID;
  j_config["esp"]["name"]      = CURRENT_FIRMWARE_TITLE;
  j_config["esp"]["password"]  = ""; 
}

/**
   Установка парараметров TCP/IP "по умолчанию"
*/
void eepromDefaultIP(){
//  j_config["ip"]["type"]       = "dhcp";
#ifdef GATE_PARK
//  j_config["ip"]["type"]       = "dhcp";
  j_config["ip"]["type"]       = "static";
//  j_config["ip"]["address"]    = "10.0.48.204";
  j_config["ip"]["address"]    = "10.0.48.194";
  j_config["ip"]["netmask"]    = "255.255.252.0";
  j_config["ip"]["gateway"]    = "10.0.48.1";
  j_config["ip"]["dns"]        = "8.8.8.8";
#else
  j_config["ip"]["type"]       = "static";
  j_config["ip"]["address"]    = "192.168.1.150";
  j_config["ip"]["netmask"]    = "255.255.255.0";
  j_config["ip"]["gateway"]    = "192.168.1.1";
  j_config["ip"]["dns"]        = "8.8.8.8";
#endif  
}

/**
   Установка парараметров WiFi "по умолчанию"
*/
void eepromDefaultWIFI(){
  j_config["wifi"]["name"]     = WIFI_SSID;
  j_config["wifi"]["password"] = WIFI_PASS;
}

/**
   Установка парараметров MQTT "по умолчанию"
*/
void eepromDefaultHTTP() { 
  j_config["http"]["server"]                = HTTP_HOST;
  j_config["http"]["port"]                  = HTTP_PORT;
  j_config["http"]["path"]                  = "/api/1/sensor/99/";
}

/**
   Установка всех парараметров  "по умолчанию"
*/
void eepromDefault() { 
  j_config.clear();
  j_config["version"]          = CURRENT_CONFIG_VERSION;    
  eepromDefaultESP();
  eepromDefaultIP();
  eepromDefaultWIFI();
  eepromDefaultHTTP();
  String s;
  serializeJson(j_config, s);
  Serial.println(s);
  
}

/**
   Чтение атрибутов из файла
*/
void attributesRead() {
   File f = SPIFFS.open("/attributes.json", FILE_READ);
   if( f ){
//      Serial.println("Read attributes ...");
      deserializeJson(j_attributes, f);
      f.close();
      if( !j_attributes.isNull() ){
//         j_attributes["Type"]    = CURRENT_FIRMWARE_TITLE;
//         j_attributes["Version"] = CURRENT_FIRMWARE_VERSION;
         String s;
         serializeJson(j_attributes, s);   
         Serial.println(s);
         return;
      }
   }
   Serial.println("Create attributes ...");
   attributesDefault();
   attributesSave();

}

/**
 * Установка атрибутов ао умолчанию
 */
void attributesDefault(){
   j_attributes.clear();
   j_attributes["TZ"]      = TIMEZONE;
//   j_attributes["Type"]    = CURRENT_FIRMWARE_TITLE;
 //  j_attributes["currentFwVer"] = CURRENT_FIRMWARE_VERSION;
   j_attributes["Name"]    = DEFAULTS_NAME;
   j_attributes.createNestedArray("Nodes");  

   j_attributes["Nodes"].add("AA000000000");  
   j_attributes["Nodes"].add("AA000000001");  
   j_attributes["Nodes"].add("AA000000002");  
   j_attributes["Nodes"].add("AA000000003");  
   j_attributes["Nodes"].add("AA000000004");  
   j_attributes["Nodes"].add("AA000000005");  
   j_attributes["Nodes"].add("AA000000006");  
   j_attributes["Nodes"].add("AA000000007");  
   j_attributes["Nodes"].add("AA000000008");  
   j_attributes["Nodes"].add("AA000000009");  
   j_attributes["Nodes"].add("AA00000000A");  
   j_attributes["Nodes"].add("AA00000000B");  
   j_attributes["Nodes"].add("AA00000000C");  
   j_attributes["Nodes"].add("AA00000000D");
   j_attributes["Nodes"].add("AA00000000E");  
   j_attributes["Nodes"].add("AA00000000F");  

   j_attributes.createNestedArray("Servers"); 
   j_attributes["Servers"].add("192.168.1.155");  
   j_attributes["Servers"].add("192.168.1.156");  
//   j_attributes["Servers"].add("192.168.1.152");  
//   j_attributes["Servers"].add("192.168.1.153");  
//   j_attributes["Servers"].add("192.168.1.154");  

}

/**
   Сохранение атрибутов в файл
*/
void attributesSave() {
   Serial.println("Save attribute");
   File f = SPIFFS.open("/attributes.json", FILE_WRITE);
   serializeJson(j_attributes, f);
//   Serial.printf
   f.close();
}


/**
   Чтение данных по нодам из файла
*/
void gatesRead() {
   File f = SPIFFS.open("/gates.json", FILE_READ);
   if( f ){
//      Serial.println("Read attributes ...");
      deserializeJson(j_gates, f);
      f.close();
   }
   gatesPrint("JsonGatesRead");
 }

/**
   Сохранение данных по нодам в файл
*/
void gatesSave() {
   File f = SPIFFS.open("/gates.json", FILE_WRITE);
   serializeJson(j_gates, f);
   f.close();
   gatesPrint("JsonGatesSave");
}


/**
* Выдача информации по всем нодам на экран
*/
void gatesPrint( char *mode ) {
   String s;
   serializeJson(j_gates, s);   
   Serial.print(mode);
   Serial.print(": ");
   Serial.println(s);
}


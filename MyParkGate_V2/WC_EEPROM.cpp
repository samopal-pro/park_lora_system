 #include "WC_EEPROM.h"
DynamicJsonDocument j_config(4096);
DynamicJsonDocument j_attributes(8192);
DynamicJsonDocument j_nodes(20000);
DynamicJsonDocument j_servers(10000);
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

#if defined(GATE_PARK) 
  j_config["ip"]["type"]       = "dhcp";
//  j_config["ip"]["type"]       = "static";
//  j_config["ip"]["address"]    = "10.0.49.202";
//  j_config["ip"]["address"]    = "10.0.49.7";
//  j_config["ip"]["netmask"]    = "255.255.252.0";
//  j_config["ip"]["gateway"]    = "10.0.48.1";
//  j_config["ip"]["dns"]        = "8.8.8.8";
#else
  j_config["ip"]["type"]       = "dhcp";
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

/*

   j_attributes["Nodes"].add("13A752772715");
   j_attributes["Nodes"].add("33A752770A1a");
   j_attributes["Nodes"].add("13A75277283a");
   j_attributes["Nodes"].add("33A752770826");
   j_attributes["Nodes"].add("13A75277292c");
   j_attributes["Nodes"].add("33A752770835");
   j_attributes["Nodes"].add("33A752770B37");
   j_attributes["Nodes"].add("13A752772B06");
   j_attributes["Nodes"].add("13A75277263e");
   j_attributes["Nodes"].add("33A752772531");
   j_attributes["Nodes"].add("33A752770B20");
   j_attributes["Nodes"].add("13A752772A40");
   j_attributes["Nodes"].add("13A752772633");
   j_attributes["Nodes"].add("33A75277201c");
   j_attributes["Nodes"].add("33A752771F44");
   j_attributes["Nodes"].add("33A752770C27");
*/   
   j_attributes["Nodes"].add("33A752770D1a");
   j_attributes["Nodes"].add("33A752771D15");
   j_attributes["Nodes"].add("33A752770B32");
   j_attributes["Nodes"].add("13A752772A2a");
   j_attributes["Nodes"].add("13A752772844");
   j_attributes["Nodes"].add("73A752771B3d");
   j_attributes["Nodes"].add("33A752770C1a");
   j_attributes["Nodes"].add("13A752772A09");
   j_attributes["Nodes"].add("33A752771D2a");
   j_attributes["Nodes"].add("33A752771C22");
   j_attributes["Nodes"].add("13A752772643");
   j_attributes["Nodes"].add("13A75277272b");
   j_attributes["Nodes"].add("13A752772639");
   j_attributes["Nodes"].add("33A752771D24");
   j_attributes["Nodes"].add("33A75277192c");
   j_attributes["Nodes"].add("13A752772932");
   j_attributes["Nodes"].add("13A752772641");
   j_attributes["Nodes"].add("33A752772127");
            
   j_attributes.createNestedArray("Servers"); 
//   j_attributes["Servers"].add("10.0.50.3");  
//   j_attributes["Servers"].add("10.0.48.204");  
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
void nodesRead() {
   File f = SPIFFS.open("/nodes.json", FILE_READ);
   if( f ){
//      Serial.println("Read attributes ...");
      deserializeJson(j_nodes, f);
      f.close();
   }
   if( nodesSynchronization() ){
      nodesSave();  
   }
   else {
      nodesPrint("JsonNodesRead");
   }
}

/**
   Сохранение данных по нодам в файл
*/
void nodesSave() {
   File f = SPIFFS.open("/nodes.json", FILE_WRITE);
   serializeJson(j_nodes, f);
   f.close();
   nodesPrint("JsonNodesSave");
}


/**
* Выдача информации по всем нодам на экран
*/
void nodesPrint( char *mode ) {
   String s;
   serializeJson(j_nodes, s);   
   Serial.print(mode);
   Serial.print(": ");
   Serial.println(s);
}

/**
* Синхранизация данных по нодам со списком нод
*/
bool nodesSynchronization(){
   bool _change = false;
// Помечаем все ноды как удаленные
   JsonObject root = j_nodes.as<JsonObject>();
//   nodesPrint("JsonNodes1");
   for (JsonPair kv : root) {
      j_nodes[kv.key()]["delete"] = true;
      j_nodes[kv.key()]["ms"]     = 0;
   }
//   nodesPrint("JsonNodes2");
// Проверяем список нод из атрибутов
   JsonArray arr = j_attributes["Nodes"].as<JsonArray>();
   for (JsonArray::iterator _node=arr.begin(); _node!=arr.end(); ++_node){
      String _snode = _node->as<String>();
      if( !j_nodes.containsKey(_snode)){
         j_nodes.createNestedObject(_snode);
         j_nodes[_snode]["status"] = "off";
         _change = true;
      }
      j_nodes[_snode].remove("delete");
//      j_nodes[_snode]["delete"] = false;    
   }
//   nodesPrint("JsonNodes3");
// Удаляем ноды, которых не было в атрибутах
   root = j_nodes.as<JsonObject>();
   for (JsonPair kv : root) {
      if( j_nodes[kv.key()].containsKey("delete") ){
         j_nodes.remove(kv.key());
         _change = true;
      }
   }
//   nodesPrint("JsonNodes3");
   return _change;
}
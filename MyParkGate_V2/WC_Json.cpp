#include "WC_Json.h"
#include "WC_Task.h"

TSJsonNodes JNodes;
TSJsonHTTP  JHttp;


/**
 * Конструктор класса TSJsonNodes
 */
TSJsonNodes::TSJsonNodes(){
   j_nodes.clear(); 
   j_lora.clear();  
   countNodeAll      = j_attributes["Nodes"].size();   
   countNodeOnline   = 0;   
   countNodeFree     = MAX_NUM_PARKING_SPACE;   
   countNodeBusy     = 0;   
   countNodeFreeSave = MAX_NUM_PARKING_SPACE;

   currentNodeNum = -1;
   isCountNodeFreeChange = false;

   for( int i=0; i<NUM_NEOPIXEL; i++ ){
      nodesStatus[i] = NLS_NONE;
//      nodesRssi[i]   = -999;
//      nodesLastMS[i] = 0;     
   }
   nodesStatus[NUM_NEOPIXEL-1] = NLS_WIFI_OFF;
}

/**
 * Конвертирование адреса ноды в имя контроллера
 * @param _node - числовой адрес ноды
 * @return - строковое имя контроллера
 */
char *TSJsonNodes::strNode(uint16_t _node){
   sprintf(StrNode,"K%03u",_node);
   return StrNode;
}


/**
 * Установка текущей ноды в документе структуры
 * 
 * @param _node - текстовое имя ноды KXXXX
 */
void TSJsonNodes::setCurrentNode(char * _node){
   j_node = j_nodes[_node].as<JsonObject>();
   if( j_node.isNull() ){
      j_node = j_nodes.createNestedObject(_node);    
   }
   q_node = j_lora[_node].as<JsonObject>();
   if( q_node.isNull() ){
      q_node = j_lora.createNestedObject(_node);    
   }
 /* 
   j_cmd  = j_node["cmd"].as<JsonArray>();
   if( j_cmd.isNull () ){
      j_cmd = j_node.createNestedArray("cmd");
      Serial.println("Create array cmd");
   }
*/
}

/**
 * Обновление времени последней телеметрии и статуса активности ноды
 */
 void TSJsonNodes::updateNode(uint32_t _ttl, int _rssi, JsonObject _params){
    j_node["ts"]   = (uint32_t)ntpClient.getEpochTime();
    j_node["ms"]   = millis();
    j_node["ttl"]  = _ttl;    
    j_node["rssi"] = _rssi;
    for (JsonPair kv : _params) {
       j_node["params"][kv.key()] = kv.value();
    }
    nodesPrint("JsonNodeUpdate");

}

/**
 * Сохранение клиентского атрибута в структуру текущей ноду
 * 
 * @param _name - имя атрибута
 * @param _val - значение атрибута
 */
void TSJsonNodes::setAttribute( char *_name, char *_val){
   j_node["attributes"][_name]=_val;
}

/**
 * Сохранение значения телеметрии в структуру текущей ноду
 * 
 * @param _name - имя телеметрии
 * @param _val - значение телеметрии
 */
void TSJsonNodes::setTelemetry( char *_name, float _value){
   j_node["params"][_name] = _value;
}


/**
 * Поставить атрибут в очередь на отправку на LoRa устройство
 * 
 * @param _name - имя атрибута
 * @param _val - значение атрибута
 */
void TSJsonNodes::addAttributeToLora( char *_name, JsonVariant _val){
   q_node["attributes"][_name] = _val;
}

/**
 * Поставить атрибут в очередь на отправку на LoRa устройство
 * 
 * @param _cmd - имя команды
 * @param _id - идентификатор RPC сессии
 */
void TSJsonNodes::addRpcToLora( char *_cmd, int _id){
   q_node["methods"][_cmd] = _id;
}

/*
bool TSJsonNodes::SetStatusNode(NEOPIXEL_LED_STATUS_t _stat, int _rssi ){
   if( currentNodeNum >= NUM_NEOPIXEL ||currentNodeNum == 0  ) currentNodeNum = -1;
   if( currentNodeNum < 0 )return false;     
   nodesStatus[currentNodeNum] = _stat;
   nodesRssi[currentNodeNum]   = _rssi;
   nodesLastMS[currentNodeNum] = millis();
   Serial.printf("!!!! Node %d stat %d\n",currentNodeNum,_stat);
   StatusAll();
   return true;
}
*/

/**
 * Послать статус всех нод
 */
bool TSJsonNodes::StatusAll(){
   bool _ret = false;
   countNodeAll    = j_attributes["Nodes"].size();   
   if( countNodeAll == 0 )countNodeAll = MAX_NUM_PARKING_SPACE;

   countNodeOnline = 0;   
   countNodeFree   = 0;   
   countNodeBusy   = 0;   
   int i=1;
   JsonArray arr = j_attributes["Nodes"].as<JsonArray>();
   for (JsonArray::iterator _node=arr.begin(); _node!=arr.end(); ++_node,i++){
      String _snode = _node->as<String>();
      uint32_t _ms = millis()-j_nodes[_snode]["ms"].as<uint32_t>();
      if( !j_nodes[_snode].containsKey("ms")|| j_nodes[_snode]["ttl"].as<uint32_t>()*1000 < _ms ){
         j_nodes[_snode]["status"] = "off";
         nodesStatus[i] = NLS_OFFLINE;
      }       
      else {
         nodesStatus[i] = NLS_NONE;
         countNodeOnline++;
      }  
//      else {
         if( !j_nodes[_snode]["params"].containsKey("Busy") || j_nodes[_snode]["params"]["Busy"].as<int>() == 0 ){
            j_nodes[_snode]["status"] = "free";  
            countNodeFree++;
//            countNodeOnline++;
            nodesStatus[i] = NLS_FREE;
         }
         else {
            countNodeBusy++;
//            countNodeOnline++;
            j_nodes[_snode]["status"] = "busy";
            nodesStatus[i] = NLS_BUSY;
         }
//      }
   }

   countNodeFree = countNodeAll - countNodeBusy;
   if( countNodeFree != countNodeFreeSave ){
     isCountNodeFreeChange = true;
     _ret = true;  
   } 
   countNodeFreeSave = countNodeFree;     
   Serial.printf("Nodes: %u, Online: %u, Busy %u, Free: %u, Change %u\n",countNodeAll,countNodeOnline,countNodeBusy,countNodeFree,(uint16_t)_ret);
   return _ret;
}

/**
 * Проверка ноды на немедленную отправку для устройств класса С
 */
bool TSJsonNodes::checkClassC(char *_node){
   JsonObject obj = j_nodes.as<JsonObject>();
   for (JsonObject::iterator it=obj.begin(); it!=obj.end(); ++it) {

//    j_node["ttl"]
      if( it->value()["ttl"].as<uint32_t>()  == 0){
//      if( it->value()["attributes"].containsKey("Class") && it->value()["attributes"]["Class"] == "C"  ){
          if(  !j_lora[it->key()]["attributes"].isNull() && j_lora[it->key()]["attributes"].size() > 0 ){
              setCurrentNode((char *)it->key().c_str());
//              char s[16];
              strncpy(_node,it->key().c_str(),15);
              return true;
          }
      }
   }
   return false;  
}
/**
 * Выдача документа на экран
 */
void TSJsonNodes::debug(){
   String s;
   serializeJson(j_nodes, s);
   Serial.print("JsonNodes:: ");
   Serial.println(s); 
   s = "";
   serializeJson(j_lora, s);
   Serial.print("JsonLora:: ");
   Serial.println(s); 
}


void TSJsonNodes::parseNodes(){
   s_tel.clear();
   s_att.clear();
//   Serial.println("Parse nodes");
   JsonObject obj = j_nodes.as<JsonObject>();
   for (JsonObject::iterator it=obj.begin(); it!=obj.end(); ++it) {
       String s1 = it->key().c_str();
       String s2 = "";
       if( it->value().containsKey("telemetry") ){
           if( it->value()["telemetry"].containsKey("Level") ){
               s1 += " ";
               float x = it->value()["telemetry"]["Level"].as<float>();
               s1 += String(x,0);
               s1 += "%";
           }
           if( it->value()["telemetry"].containsKey("Vcc") ){
               s1 += " ";
               float x = it->value()["telemetry"]["Vcc"].as<float>();
               s1 += String(x,1);
               s1 += "V";
           }
           if( it->value()["telemetry"].containsKey("Rssi") ){
               s1 += " ";
               float x = it->value()["telemetry"]["Rssi"].as<float>();
               s1 += String(x,0);
               s1 += "dB";
           }
       }
       if( it->value().containsKey("ts") ){
           char s[32];
           uint32_t t1 = it->value()["ts"].as<unsigned long>();
//           uint32_t t2 = time(nullptr) - t1;
           uint32_t t2 = (uint32_t)ntpClient.getEpochTime() - t1;

           const time_t t = (const time_t )t1;
           struct tm *ti = localtime( &t ); 
           sprintf(s,"%02d.%02d.%02d %02d:%02d",ti->tm_mday,ti->tm_mon+1,ti->tm_year%100,ti->tm_hour,ti->tm_min);           
           s2 = s;
       }
       if( it->value().containsKey("attributes") ){
           if( it->value()["attributes"].containsKey("TM") ){
               s2 += " TM=";
               s2 += it->value()["attributes"]["TM"].as<String>();
//               s2 += "s";
           }
           if( it->value()["attributes"].containsKey("High") ){
               s2 += " H=";
               s2 += it->value()["attributes"]["High"].as<String>();
//               s2 += "mm";
           }
       }
       s_tel.push_back(s1);
       s_att.push_back(s2);
   }
   
  
}

/**
 * Поместить ID и 
 */
void TSJsonNodes::addRequest(uint32_t _id, char *_name){
   char s[16];
   ultoa(_id,s,10);
//   sprintf(s,"%u",_id);
   j_node["request"][s] = _name;
//   j_node["request"][_name] = _id;
}


TSJsonHTTP::TSJsonHTTP(void){
   j_msg.clear();   
}

/**
 * Формирование строки отправки телеметрии по HTTP
 */
void TSJsonHTTP::setParams(uint16_t cNodeAll,uint16_t cNodeOnline,uint16_t cNodeFree,uint16_t cNodeBusy){
   char s[1024];
   sprintf(s,"http://%s:%d%s?id=%s&all=%u&online=%u&free=%u&busy=%u&key=%u",
      j_config["http"]["server"].as<const char *>(),
      j_config["http"]["port"].as<int>(),
      j_config["http"]["path"].as<const char *>(),
      strID,cNodeAll,cNodeOnline,cNodeFree,cNodeBusy,keyGen(strID,cNodeAll,cNodeOnline,cNodeFree,cNodeBusy));
   Serial.printf("HTTP GET: %s\n",s);      
   pop("xxx",s);     
}


void TSJsonHTTP::setStatisticAll(uint16_t cNodeAll,uint16_t cNodeOnline,uint16_t cNodeFree,uint16_t cNodeBusy){
   if( j_attributes.containsKey("Servers") && j_attributes["Servers"].size()>0 ){
      JsonArray arr = j_attributes["Servers"].as<JsonArray>();
      int i=1;
      q_msg.clear();
      q_ip.clear();
      for (JsonArray::iterator it=arr.begin(); it!=arr.end(); ++it,i++) {  
        setStatistic((char *)it->as<const char *>(),cNodeAll,cNodeOnline,cNodeFree,cNodeBusy);
      }
   }
}

void TSJsonHTTP::setStatistic(char *_server,uint16_t cNodeAll,uint16_t cNodeOnline,uint16_t cNodeFree,uint16_t cNodeBusy){
   char s[1024];
   sprintf(s,"http://%s/api?id=%s&all=%u&online=%u&free=%u&busy=%u&key=%u",
      _server,strID,cNodeAll,cNodeOnline,cNodeFree,cNodeBusy,keyGen(strID,cNodeAll,cNodeOnline,cNodeFree,cNodeBusy));
   Serial.printf("HTTP GET: %s\n",s);      
   pop(_server,s);     
}

/**
 * Отправка JSON в очередь на отправку по MQTT
 * @param _topic - имя топика MQTT
 */
void TSJsonHTTP::pop(const char *_server,const char *_str){
 
// Проверка что такое сообщение уже есть в очереди   
   Serial.print("Queue ");
   Serial.print(q_msg.size());
   Serial.print(" ");
   Serial.println(_str);
   if( q_msg.size() < Q_MAX_SIZE ){
       q_msg.push_back(_str);
       q_ip.push_back(_server);
   }
   else {
       Serial.println("Limit Queue size");
   }  
}


/**
 * Генерация контрольной суммы 
 */
uint16_t TSJsonHTTP::keyGen(char *_node, int _dist, uint32_t _tm ){
   char s[128];
   sprintf(s,"%s;%ld;%d;%d;%d",_node,_tm,_dist,_tm,0);
   uint16_t crc = 0;
   for( int i=0; i< strlen(s); i++ ){
       crc += (int)s[i];
   }    
   crc = ( ~ crc )&0xfff;   
   return crc;
  
}

/**
 * Генерация контрольной суммы 
 */
uint16_t TSJsonHTTP::keyGen(char *_id,uint16_t cNodeAll,uint16_t cNodeOnline,uint16_t cNodeFree,uint16_t cNodeBusy ){
   char s[128];
   sprintf(s,"%s;%u;%u;%u;%u",_id,cNodeAll,cNodeOnline,cNodeFree,cNodeBusy);
   uint16_t crc = 0;   for( int i=0; i< strlen(s); i++ ){
       crc += (int)s[i];
   }    
   crc = ( ~ crc )&0xfff;   
   return crc;
  
}
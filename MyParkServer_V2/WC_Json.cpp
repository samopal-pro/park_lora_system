#include "WC_Json.h"
#include "WC_Task.h"

//TSJsonNodes JNodes;
TSJsonHTTP  JHttp;
TSJsonGates JGates;
NEOPIXEL_LED_STATUS_t nodesStatus[NUM_NEOPIXEL];
/**
 * Конструктор класса TSJsonNodes
 */



TSJsonHTTP::TSJsonHTTP(void){
   j_msg.clear();   
}

/**
 * Формирование строки отправки телеметрии по HTTP
 */
void TSJsonHTTP::setParams(String id,uint16_t cNodeAll,uint16_t cNodeOnline,uint16_t cNodeFree,uint16_t cNodeBusy){
   char s[1024];
   sprintf(s,"http://%s:%d%s?id=%s&all=%u&online=%u&free=%u&busy=%u&key=%u",
      j_config["http"]["server"].as<const char *>(),
      j_config["http"]["port"].as<int>(),
      j_config["http"]["path"].as<const char *>(),
      id.c_str(),cNodeAll,cNodeOnline,cNodeFree,cNodeBusy,keyGen(strID,cNodeAll,cNodeOnline,cNodeFree,cNodeBusy));
      Serial.printf("HTTP GET: %s\n",s);      
//      pop(s);     
}

void TSJsonHTTP::setStatisticAll(uint16_t cNodeAll,uint16_t cNodeOnline,uint16_t cNodeFree,uint16_t cNodeBusy){
   if( j_attributes.containsKey("Servers") && j_attributes["Servers"].size()>0 ){
      JsonArray arr = j_attributes["Servers"].as<JsonArray>();
      int i=1;
      for (JsonArray::iterator it=arr.begin(); it!=arr.end(); ++it,i++) {  
        setStatistic((char *)it->as<const char *>(),cNodeAll,cNodeOnline,cNodeFree,cNodeBusy);
      }
   }
}

void TSJsonHTTP::setStatistic(char *_server,uint16_t cNodeAll,uint16_t cNodeOnline,uint16_t cNodeFree,uint16_t cNodeBusy){
   char s[1024];
  sprintf(s,"http://%s/api?id=%s&all=%u&online=%u&free=%u&busy=%u&key=%u",
      _server,strID,cNodeAll,cNodeOnline,cNodeFree,cNodeBusy,keyGen(_server,cNodeAll,cNodeOnline,cNodeFree,cNodeBusy));
   Serial.printf("HTTP GET: %s\n",s);      
   pop(s);     
}

/**
 * Отправка JSON в очередь на отправку по MQTT
 * @param _topic - имя топика MQTT
 */
void TSJsonHTTP::pop(const char *_str){
 
// Проверка что такое сообщение уже есть в очереди   
   Serial.print("Queue ");
   Serial.print(q_msg.size());
   Serial.print(" ");
   Serial.println(_str);
   if( q_msg.size() < Q_MAX_SIZE ){
       q_msg.push_back(_str);
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

TSJsonGates::TSJsonGates(){
//   Json.clear();
   changeCountFree = true;
   countFreeSave = 0xffff;
}

void TSJsonGates::set(String _id, uint16_t _all, uint16_t _online, uint16_t _free, uint16_t _busy){
   j_gates[_id]["countAll"]    = _all;
   j_gates[_id]["countOnline"] = _online;
   j_gates[_id]["countFree"]   = _free;
   j_gates[_id]["countBusy"]   = _busy;
//   j_gates[_id]["ts"]          = (uint32_t)ntpClient.getEpochTime();
   print();
}

void TSJsonGates::calculate(){
   countAll    = 0;
   countOnline = 0;
   countFree   = 0;
   countBusy   = 0;
   countGate   = 0;
   JsonObject _gates = j_gates.as<JsonObject>();
   for (JsonPair _item : _gates) {
       JsonObject _obj = _item.value();
       countAll    += _obj["countAll"].as<uint16_t>();
       countOnline += _obj["countOnline"].as<uint16_t>();
       countFree   += _obj["countFree"].as<uint16_t>();
       countBusy   += _obj["countBusy"].as<uint16_t>();
       countGate++;
    }
    if( countFree != countFreeSave ){
       changeCountFree = true;
       countFreeSave   = countFree;
    }
    Serial.printf("!!!! Gates=%d All=%d Online=%d Free=%d Busy=%d\n",countGate, countAll, countOnline, countFree, countBusy);
}

void TSJsonGates::print(){
   gatesPrint("JsonGates");
}
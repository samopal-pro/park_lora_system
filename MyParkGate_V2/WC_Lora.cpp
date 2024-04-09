#include "WC_LoRa.h"

MyLoRaBaseClass myLora((uint16_t)LORA_NODE);
States_t state;
char curNode[24];

uint16_t saveCount = 0xffff, saveNode = 0;
bool is_rx_mode = true;
bool is_wait_request = false;
uint16_t idCount = 1;
uint32_t idCount32 = 1;




#if defined(SCAN_ALL_NODES)
StaticJsonDocument<JSON_NODES_SIZE> jAllNodes;
uint16_t allModesCount     = 0;
bool     isAllNodesChange  = true;
#endif
//bool isCountNodeFreeChange = false;

/**
 * Параллельная задача обработки LoRa сообщений
 * 
 * @param pvParameters - параметры для FreeRTOS задачи
 */
void taskLoRa( void *pvParameters ){
   Serial.printf("LoRa task start ");
   Serial.println(xPortGetCoreID());
   uint8_t mac[6];
   esp_efuse_mac_get_default(mac);
//   MyLoRaAddress::Copy(myLora.Addr,mac);
   MyLoRaAddress::Set(myLora.Addr,0,0,0,0,0,0);
   char s[16];
   Serial.print(F("MyLoRa MAC Addr: "));
   Serial.println(MyLoRaAddress::Get(s,myLora.Addr));
   initLora();

  
///#ifdef DISPLAY_LED   
///   pinMode(PIN_LED_GREEN,OUTPUT);
///   digitalWrite(PIN_LED_GREEN,LOW);
///#endif   
// Инициализация SPI и LoRa модема SX1276
   myLora.StateRX = NSRX_NONE;   
   myLora.StateTX = NSTX_NONE;
//   LoRa_rxMode(); 
   uint32_t _ms0 = 0;
   int _count1 = 1;
   char _node[16];
   Radio.Rx(0);
// Активация ИК передатчика
//   irsend.begin(); 
   SendIR(0);
   vTaskDelay(3000);
   JNodes.StatusAll();
   SendIR(JNodes.countNodeFree);
   JNodes.isCountNodeFreeChange = false;


   while( true ){
//Если режим точки доступа то отключить прием сообщений    
//      if( isAP ){ 
//           vTaskDelay(1000);
//           continue;
//       }
//       Serial.println("1");
// Если в приемном цикле есть входящий пакет      
//       Radio.Rx(0);
       Radio.IrqProcess();

       if( loopLora(_node) ){
          SetSemaphoreIR(true);
          myLora.PrintRX();
          switch( myLora.StateRX ){
             case NSRX_ERROR_CRC :  
                myLora.SetRequest(false);
                sendLora();
                break;   
             case NSRX_OK : 
                JNodes.setCurrentNode(_node);
                decodeLora(_node,myLora.isRX_V3);
             case NSRX_DUBLE : 
                if(is_wait_request){
                   if(  myLora.isRX_V3 &&  myLora.HeaderRX_V3.Type & PACKET_V3_TYPE_ACK || !myLora.isRX_V3 && myLora.HeaderRX.Type == PACKET_TYPE_ACK  ){
                     JNodes.q_node.remove("attributes");
                   }   
                   is_wait_request = false;
                }                   
                else is_wait_request = sendJsonToLora(_node,myLora.isRX_V3);
                break; 
          }
          SetSemaphoreIR(false);
       }
   
       uint32_t _ms = millis();
       if( _ms0 < _ms ){
          if( JNodes.isCountNodeFreeChange ){
               SendIR(JNodes.countNodeFree);
               JNodes.isCountNodeFreeChange = false;
//               Serial.println("!!!!! Count Free is change !!!");
          }
//          else {
//               Serial.println("!!!!! Count Free not change");
// 
//         }
        
//           char _name[64],_value[64],_node1[16];
//           if( JNodes.checkClassC(_node1) ){
//               sendJsonToLora(_node1,false);
//           }
           
           _ms0 = _ms + 4000;
       }
       
       if( is_rx_mode )vTaskDelay(5);
       myLora.StateRX = NSRX_NONE;
//       Radio.IrqProcess();
   }     
}

/**
 * Инициализация LoRa модема
 */ 
void initLora(){
    static RadioEvents_t RadioEvents;
    Mcu.begin();

    RadioEvents.TxDone    = OnTxDone;
    RadioEvents.TxTimeout = OnTxTimeout;
    RadioEvents.RxDone    = OnRxDone;
    RadioEvents.RxError   = OnRxError;

    Radio.Init( &RadioEvents );
    Radio.SetChannel( RF_FREQUENCY );
    Radio.SetTxConfig( MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                                   LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                                   LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   true, 0, 0, LORA_IQ_INVERSION_ON, 3000 );

    Radio.SetRxConfig( MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                                   LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                                   LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   0, true, 0, 0, LORA_IQ_INVERSION_OFF, true );

}

/*
 * Callback успешное завершение передачи
 */
void OnTxDone( void ){
   myLora.StateTX = NSTX_SEND;   
   Serial.println(">>>> TX");
   myLora.PrintTX();
   Radio.Rx(0);
   is_rx_mode = true;
}

/*
 * Callback функция события таймаута приема
 */
void OnRxTimeout( void ){
   myLora.StateRX = NSRX_ERROR_TM;   
}

/*
 * Callback функция события таймаута передачи
 */
void OnTxTimeout( void ){
   myLora.StateTX = NSTX_ERROR;   
   Serial.println(">>>> TM TX");
   myLora.PrintTX();
   Radio.Rx(0);
   is_rx_mode = true;
}


/*
 * Callback функция события ошибки приема
 */
void OnRxError( void ){
   myLora.StateRX = NSRX_ERROR;   
}

/*
 * Callback функция события приема пакета
 * 
 * @param _payload - приемный буфер
 * @param _size - размер принимаемого пакета
 * @param _rssi - уровень сигнала принимаемого пакета
 * @param _snr - отношение сигнал/шум (не используется)
 */
void OnRxDone( uint8_t *_payload, uint16_t _size, int16_t _rssi, int8_t _snr ){
   Serial.println("<<<< RX");
   myLora.RX(_payload, _size, _rssi);
}



/**
 * Переключение LoRa модема в режим приема
 */
void LoRa_rxMode(){
//  LoRa.receive();                       
  if( !is_rx_mode){
//     LoRa.enableInvertIQ();               
     is_rx_mode = true;
  }

}

/**
 * Переключение LoRa модема в режим передачи
 */
void LoRa_txMode(){
  if( is_rx_mode){
//     LoRa.disableInvertIQ();               
     is_rx_mode = false;
  }
}

/**
 * Начинаем отправку LoRa пакета
 */
void sendLora(){
   is_rx_mode = false;  
   myLora.StateTX = NSTX_SEND;
   LoRa_txMode();
   Radio.Send(myLora.BufferTX, myLora.LengthTX);
//   Radio.IrqProcess();

}

/**
 * Приемный цикл LoRa
 */
bool loopLora(char *_node){
//   LoRa_rxMode();
//   Radio.IrqProcess();
   if( myLora.StateRX != NSRX_NONE ){
//      Serial.printf("Lora receive size=%d\n",packetSize,(int)myLora.Rssi);
//      myLora.PrintRX();
      if( myLora.isRX_V3 ){
          MyLoRaAddress::Get(_node,myLora.HeaderRX_V3.AddrTX);
//          Serial.println("MyLora V3 detected");
      }
      else {
          strncpy(_node,JNodes.strNode(myLora.HeaderRX.NodeTX),15);
//          Serial.println("MyLora V2 detected");
      }

#ifdef DISPLAY_LED  
      LedGreen.SeriesBlink(1,30000); 
#endif   
      if( checkWhiteList(_node) )return true;
      else return false;  
   }
   return false;
}


/**
 * Декодировать и обработать LoRa пакет
 */
void decodeLora(char *_node, bool _isV3){
    char _name[64], _svalue[64];
    float _value;
    int n = 0;
    uint8_t _type;
    uint32_t _ttl;
    if(_isV3){
   //   JNodes.updateNode(myLora.decodeTTL(myLora.HeaderRX_V3.TTL));
      _type = myLora.HeaderRX_V3.Type&B00001111 ;
      _ttl  = myLora.decodeTTL(myLora.HeaderRX_V3.TTL);
    }
    else {
//      JNodes.updateNode(myLora.decodeTTL(myLora.HeaderRX.TTL));
      _type = myLora.HeaderRX.Type;
      _ttl  = myLora.decodeTTL(myLora.HeaderRX.TTL);
    }
    switch( _type ){
       case PACKET_TYPE_JSON_TELEMETRY: 
       case PACKET_V3_TYPE_JSON_TELEMETRY: 
          if( myLora.GetJson() ){
              if( myLora.Json.containsKey("Dist") && myLora.Json.containsKey("Height") && myLora.Json.containsKey("Busy")  ){
                 JNodes.updateNode(_ttl,myLora.Rssi,myLora.Json.as<JsonObject>());
                 if( JNodes.StatusAll() ){
                     Serial.printf("!!!! Count Free change is %d\n",(int)JNodes.isCountNodeFreeChange);
                     JHttp.setStatisticAll(JNodes.countNodeAll,JNodes.countNodeOnline,JNodes.countNodeFree,JNodes.countNodeBusy);
                     nodesSave();
                 }
              }
          }
          break;
     }
 

}


/**
 * Проверить очередь отправки по LoRa для заданного узла
 * 
 * @param _node - номер узла
 * @return true - сообщение отправлено для данной ноды
 */
bool sendJsonToLora( char *_node, bool _isV3){
// Проверка списка атрибутов 
   bool _ret  = false; 
// Обработка атрибутов для сенсора  
   if( JNodes.q_node.containsKey("attributes") ){
       if( JNodes.q_node["attributes"].size() > 0 ){
          if( _isV3 ){
             myLora.SetHeaderTX(PACKET_V3_TYPE_JSON_ATTRIBUTE,myLora.HeaderRX_V3.AddrTX);
             myLora.Json.set(JNodes.q_node["attributes"]);
             if( myLora.SetJsonBodyTX() )_ret = true;
          }
          else {
             myLora.StartTX(PACKET_TYPE_JSON_ATTRIBUTE,myLora.HeaderRX.NodeTX);
             myLora.Json.set(JNodes.q_node["attributes"]);
             if( myLora.SetJson() )_ret = true;
          }
       } 
   }
   else myLora.SetRequest(true);
   sendLora();
   return _ret;
}

bool checkWhiteList(char *_node){
#if defined(SCAN_ALL_NODES)
   uint32_t _ts = ntpClient.getEpochTime();
   jAllNodes[_node] = _ts;
   uint16_t _count        = jAllNodes.size();
   if( allModesCount != _count  ){
      allModesCount    = _count;
      isAllNodesChange = true;   
   }
/*

   Serial.print("Set node to list ");
   Serial.print(_node);
   Serial.print(" ");
   Serial.println(_ts);

   String sss;
   serializeJson(jAllNodes, sss);
   Serial.print(F("AllNodeList:: "));
   Serial.println(sss); 
*/
#endif


   if( j_attributes.containsKey("Nodes") && j_attributes["Nodes"].size()>0 ){
      bool _is_valid = false;
      JNodes.currentNodeNum = -1;
      Serial.print("Check Nodes List: ");
      Serial.print(_node);
      JsonArray arr = j_attributes["Nodes"].as<JsonArray>();
      int i=1;
      for (JsonArray::iterator it=arr.begin(); it!=arr.end(); ++it,i++) {
         if( strcmp(it->as<const char *>(),_node) == 0 ){
           _is_valid = true;
            Serial.printf("!!!! Current node num is %d\n",i);
           if( i < NUM_NEOPIXEL )JNodes.currentNodeNum = i;
         }
      } 
//      Serial.print(" ");
//      Serial.print(it->as<const char *>());
      if( _is_valid ){
         Serial.println(" OK");      
         return true;
      }
      else {
         Serial.println(" NOT");      
         return false;
      }
   }
   Serial.println("List Nodes is empty");
   return false;

}


#include "WC_LoRa.h"

uint16_t saveCount = 0, saveNode = 0;
RadioEvents_t RadioEvents;

/**
 * Инициализация LoRa модема
 */
void initLora( ){
   Serial.println(F("LoRa init "));
   RadioEvents.TxDone    = OnTxDone;
//   RadioEvents.TxTimeout = OnTxTimeout;
   RadioEvents.RxDone    = OnRxDone;
   RadioEvents.RxTimeout = OnRxTimeout;
   RadioEvents.RxError   = OnRxError;
   Radio.Init( &RadioEvents );
   Radio.SetChannel( LORA_RF_FREQUENCY );
   Radio.SetTxConfig( MODEM_LORA, LORA_TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                      LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                      LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                      true, 0, 0, LORA_IQ_INVERSION_ON, 3000 );

   Radio.SetRxConfig( MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                      LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                      LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                      0, true, 0, 0, LORA_IQ_INVERSION_OFF, true );
}

/**
 * Отправка сформированного пакета
 */
bool sendLora(uint8_t _attMax){
   bool ret = false;
   uint8_t _att     = 0;
//   int _attMax  = myLora.CountTX; 
   bool is_base = true;
   for( int i=0; i<20; i++){ // Ограничиваем максимальное количество отправляемых сообщений
// Отправляем пакет 
      Radio.Send(myLora.BufferTX,myLora.LengthTX);
      myLora.StateTX = NSTX_NONE;     
      for(uint32_t ms = millis(); millis()-ms < TIMEOUT_TX && myLora.StateTX == NSTX_NONE;){
          Radio.IrqProcess();  
      }      
      myLora.PrintTX();
          
// Если был отправлен базовый пакет базовый пакет
      if( is_base ){  
// Создаем коно приема        
         if( receiveLora(TIMEOUT_RX) ){   
//            Serial.println("!!! Receive 1");   
            if(  (myLora.HeaderRX_V3.Type&B000111) ==  PACKET_V3_TYPE_ACK ){ret = true;break;}
            else if( (myLora.HeaderRX_V3.Type&B000111) ==  PACKET_V3_TYPE_ERROR )_att++;
            else {
                ret = true;
                is_base = false;                 
                decodeLora();
                myLora.SetHeaderTX(PACKET_V3_TYPE_ACK,myLora.HeaderRX_V3.AddrTX);
                myLora.HeaderTX_V3.CRC = myLora.SetCRC_V3(myLora.BufferTX,myLora.LengthTX);
            }
         }
         else _att++; 
         if( _att >= _attMax )break;
      } 
// Если шлются пакет подтверждения       
      else {
         if( receiveLora(TIMEOUT_RX) ){      
//            Serial.println("!!! Receive 2");   
            if(  (myLora.HeaderRX_V3.Type&B000111) ==  PACKET_V3_TYPE_ACK ){ret = true;break;}
            else {
                ret = true;
                decodeLora();
                myLora.SetHeaderTX(PACKET_V3_TYPE_ACK,myLora.HeaderRX_V3.AddrTX);
                myLora.HeaderTX_V3.CRC = myLora.SetCRC_V3(myLora.BufferTX,myLora.LengthTX);
            }
         }
         else break;
      }
      delay(random(0,200));
   }
   return ret;
}



/**
 * Создаем окно приема
 * 
 * @param _tm - длительность окна приема в мс
 * @return true - если был принят пакет
 * @return false - если пакет не принят или не прошел верификацию
 */
bool receiveLora( uint32_t _tm ){
    myLora.StateRX = NSRX_NONE;
//    Serial.println("RX ... ");
    Radio.Rx(0);
    uint32_t _ms = millis();
    for( uint32_t _ms0 = _ms + _tm; _ms < _ms0 && myLora.StateRX == NSTX_NONE;_ms = millis()){       
        Radio.IrqProcess( );   
        delay(20);
    }
    if( myLora.StateRX == NSRX_NONE )myLora.StateRX = NSRX_ERROR_TM;
//    Serial.println("DONE");
#ifdef MYLORA_DEBUG
    myLora.PrintRX();
#endif     
    if( myLora.StateRX == NSRX_OK )return true;
    else if( myLora.StateRX == NSRX_DUBLE )return true;
    return false;
}

/*
 * Callback функция события успешной передачи пакета
 */
void OnTxDone( void ){
 #ifdef MYLORA_DEBUG
//   Serial.println(F("TX DONE"));
 #endif
   myLora.StateTX = NSTX_SEND;   
}

/*
 * Callback функция события таймаута приема
 */
void OnRxTimeout( void ){
#ifdef MYLORA_DEBUG
   Serial.println(F("RX TIMEOUT"));
#endif
   myLora.StateRX = NSRX_ERROR_TM;   
}

/*
 * Callback функция события ошибки приема
 */
void OnRxError( void ){
#ifdef MYLORA_DEBUG
   Serial.println(F("RX ERROR"));
#endif
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
   myLora.RX(_payload,_size,_rssi);
}

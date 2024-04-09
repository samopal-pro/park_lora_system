// Include the libraries we need
#include <MyLoRaBase.h>

#include <innerWdt.h>
#include "MyConfig.h"
#include "WC_EEPROM.h"
#include "MySensorSR04.h"
#include "WC_Lora.h"
#include "WC_neopixel.h"
#include "src/SButton.h"

TimerEvent_t sleepTimer;
bool sleepTimerExpired    = false;

uint64_t chipID;
char strID[16];
bool isReboot             = false;
bool isClean              = false;
bool isAttributesResp     = false;
bool isAttributesSend     = false;
bool isClearDone          = false;
bool isServiceModeChange  = false;
bool isServiceMode        = false;
bool isServiceModeAlways  = false;
bool isStarted            = true;

//bool isSetServiceMode     = false;

uint32_t msAttributesResp = 60000; 
uint32_t msServiceMode    = 0;
uint16_t Attempt          = 2;
//int      loopCount        = 0;

uint32_t TM_SENS           = 1000;
uint32_t TM_LORA           = 60000;
uint32_t TM_LORA_ERR       = 30000;
uint16_t CNT_ON            = 20;
uint16_t CNT_OFF           = 20;
float    L_Max             = 4500;
float    L_Min             = 100;
float    L_Ground          = 2000;
float    L_Busy            = 1000;

uint16_t countBusy        = 0;
bool     isBusy           = false;
#ifdef FAIL_SENSOR
bool     isBusySave       = true; 
#else
bool     isBusySave       = false;
#endif

bool     isBusyReal       = false;
uint32_t _tm_lora         = TM_LORA;
//uint32_t serviceModeS     = 0;
//uint16_t serviceLoopFirst = 5;
void setServiceMode(bool _sm);  
MyLoRaBaseClass myLora(getID(),false);

#if defined(PIN_BTN)
SButton btn(PIN_BTN);
#endif

/**
*  setup() функция Arduino IDE
*/
void setup(void){
//  innerWdtEnable(true);
 // start serial port
  Serial.begin(115200);
  chipID=getID();
  Serial.print(CURRENT_FIRMWARE_TITLE);
  Serial.print(" Ver ");
  Serial.print(CURRENT_FIRMWARE_VERSION);
  MyLoRaAddress::Get(strID,myLora.Addr);
  Serial.print(" ChipID ");
  Serial.println(strID);
  char s[16];
  Serial.print(F("MyLoRa MAC Addr: "));
  Serial.println(MyLoRaAddress::Get(s,myLora.Addr));
  int x = 0;
  for(int i=0; i<strlen(strID);i++)x+=(byte)strID[i];
  Serial.print(F("Random Seed "));
  Serial.print(x);
  randomSeed(x);
  x = random(1000,10000);
  Serial.print(F(" Delay "));
  Serial.println(x);
  delay(x);
   


  startNeopixel();
  
  eepromBegin();
//  eepromDefault();
//  eepromSave();
  eepromRead(); 
//  sensorsInit();  

//  Attempt = (uint16_t)getAttributeJson(ATT_ATT,2);
  setAttributes();
  myLora.setTTL(120);
  MyLoRaAddress::Set(myLora.AddrRX,0,0,0,0,0,0);
  initLora();
#if defined(PIN_BTN)
  btn.SetLongClick(1000);
#endif    
  sendStatus("STARTUP");
  setNeopixel(LM_FREE);
  _tm_lora = TM_LORA;
}

uint32_t _ms = 0, _ms0 = 0,  _ms1 = 0, _ms2 = 0, _ms3 = 0;

/**
*  loop() - главный цикл Arduino IDE
*/

void loop(void){
   uint32_t _ms_cur = millis();
   bool is_send_telemetry = false;
   
// Если переполнение переменной   
   if( _ms_cur < _ms ){
      _ms0 = 0;
      _ms1 = 0;
      _ms2 = 0;
      _ms3 = 0;
   }
   _ms = _ms_cur;

#if defined(PIN_BTN)
  if( _ms0 < _ms ){
     _ms0 = _ms + 200;
     switch( btn.Loop() ){
        case SB_LONG_CLICK :
           Serial.println("!!! Press btn");
           if( !isnan(L_sr04) && L_sr04 >= L_Min && L_sr04 <= L_Max ){
              j_attributes["Shared"][ATT_L_GROUND_MM] = (uint16_t)L_sr04;
              setNeopixel(LM_BUTTON);
              Serial.print(F("!!! Set ground at "));
              Serial.println((uint16_t)L_sr04);
              eepromSave();
              setAttributes();
           }
           sendStatus("BTN");
           break;
     }
  }
#endif  
  if( _ms1 < _ms ){
     if( getDistanceSR04HAL() ){
        _ms1 = _ms + TM_SENS;  
// Работа со счетчиком для исключения случайных срабатываний     
        if( Busy_sr04 != isBusySave ){
           countBusy++;
           if( Busy_sr04 && CNT_ON < countBusy ){
              Serial.println(F(">>> Park place is busy"));
              countBusy = 0;
              isBusySave = Busy_sr04;
              isBusyReal = true;           
              setNeopixel(LM_BUSY);
              is_send_telemetry = true;
           }
           if( !Busy_sr04 && CNT_OFF < countBusy ){
              Serial.println(F(">>> Park place is free"));
              countBusy = 0;
              isBusySave = Busy_sr04;
              isBusyReal = false;
              setNeopixel(LM_FREE);
              is_send_telemetry = true;
           }
        }
        else countBusy = 0;
     }
     setNeopixelLoop();
  }
  if( _ms2 < _ms && countBusy == 0 ){
      _ms2 = _ms + _tm_lora;  
      is_send_telemetry = true;
//     sendAttributes();
//     sensorSend();
  }
  if( is_send_telemetry ){
     is_send_telemetry = false;
     sendTelemetry();
  }
//  if( _ms3 < _ms ){
//      _ms3 = _ms + 150;  
//      setNeopixelLoop();
//  }

}

void setAttributes(){
   Serial.println(F("!!! Set attributes"));
   TM_SENS      = (uint32_t)getAttributeJson(ATT_TM_SENS_S,1)*1000;
   TM_LORA      = (uint32_t)getAttributeJson(ATT_TM_LORA_S,300)*1000;
   TM_LORA_ERR  = (uint32_t)getAttributeJson(ATT_TM_LORA_ERR_S,30)*1000;
   CNT_ON       = (uint16_t)getAttributeJson(ATT_TM_ON_S,5);
   CNT_OFF      = (uint16_t)getAttributeJson(ATT_TM_OFF_S,5);
   L_Max        = getAttributeJson(ATT_L_MAX_MM,4500);
   L_Min        = getAttributeJson(ATT_L_MIN_MM,100);
   L_Ground     = getAttributeJson(ATT_L_GROUND_MM,2000);
   L_Busy       = getAttributeJson(ATT_L_BUSY_MM,1000);
}

/**
* Послать статус
*/
void sendStatus(char *_txt){
   myLora.SetHeaderTX(PACKET_V3_TYPE_JSON_TELEMETRY, myLora.AddrRX, true);
   myLora.Json["Status"] = _txt;
   if (myLora.SetJsonBodyTX()) sendLora(Attempt);
}


/**
* Послать параметры
*/
void sendTelemetry(){
//#ifndef FAIL_SENSOR  
//  if( !isnan(L_sr04) ){
//#endif
     myLora.SetHeaderTX(PACKET_V3_TYPE_JSON_TELEMETRY, myLora.AddrRX, true);
     myLora.Json["Dist"]   = round(L_sr04);
     myLora.Json["Height"] = round(H_sr04);
     myLora.Json["Busy"]   = round(isBusyReal);

     if (myLora.SetJsonBodyTX()) {
        if( sendLora(Attempt) )_tm_lora = TM_LORA;
        _tm_lora = TM_LORA_ERR;
     }
//#ifndef FAIL_SENSOR     
//  } 
//#endif    
}

/**
 * Послать клиентские атрибуты на сервер
 */
void sendAttributes(){
   if( !isAttributesSend )return;
   myLora.SetHeaderTX(PACKET_V3_TYPE_JSON_ATTRIBUTE,myLora.AddrRX);
   myLora.Json[ATT_ID]   = strID;
   myLora.Json[ATT_VER]  = (char *)CURRENT_FIRMWARE_VERSION;
   myLora.Json[ATT_NAME] = j_attributes[ATT_NAME];
#if defined(CONTROLLER_TYPE) || defined(ATT_TYPE) 
   myLora.Json[ATT_TYPE] = (char *)CONTROLLER_TYPE;
#endif
   isAttributesSend = false;
   if( myLora.SetJsonBodyTX() )sendLora(Attempt);  
}

/**
 *  Послать запрос на получение общих атрибутов
 */
void sendAttributesReq(){
   if( isAttributesResp )return; 
   if( millis() - msAttributesResp < 60000 )return; 
   msAttributesResp = millis();    
   myLora.SetHeaderTX(PACKET_V3_TYPE_JSON_ATTRIBUTE_REQUEST,myLora.AddrRX);
   JsonObject obj = j_attributes["Shared"].as<JsonObject>();
   for (JsonObject::iterator it=obj.begin(); it!=obj.end(); ++it) {
       myLora.Json.add(it->key().c_str());    
   }
   if( myLora.SetJsonBodyTX() )sendLora(Attempt);     
}

/**
* Декодировать входящие лора сообщения
 */
void decodeLora(){
//   Serial.println("!!! Decode");

   bool flag;
   JsonObject obj;
   JsonVariant _val;
   switch( myLora.HeaderRX_V3.Type & B00001111 ){  
       case PACKET_V3_TYPE_JSON_ATTRIBUTE :
          myLora.GetJson();
          flag = false;
          obj = myLora.Json.as<JsonObject>();
          if(setAttributeJson(ATT_RPEAT_N,     obj, 1,   10  )  )flag = true;
          if(setAttributeJson(ATT_TM_SENS_S,   obj, 1,   10  )  )flag = true;
          if(setAttributeJson(ATT_TM_LORA_S,   obj, 10,  1200)   )flag = true;
          if(setAttributeJson(ATT_TM_LORA_ERR_S,   obj, 10,  300)   )flag = true;
          if(setAttributeJson(ATT_TM_ON_S,     obj, 1,   10)    )flag = true;
          if(setAttributeJson(ATT_TM_OFF_S,    obj, 1,   10)    )flag = true;
          if(setAttributeJson(ATT_L_GROUND_MM, obj, 500, 10000) )flag = true;
          if(setAttributeJson(ATT_L_MAX_MM,    obj, 500, 10000) )flag = true;
          if(setAttributeJson(ATT_L_MIN_MM,    obj, 1,   10000) )flag = true;
          if(setAttributeJson(ATT_L_BUSY_MM,   obj, 1,   10000) )flag = true;
          isAttributesResp = true;  
          if( flag ){
              eepromSave(); 
//             sensorsConfigure();
          }
          break;
   }

}

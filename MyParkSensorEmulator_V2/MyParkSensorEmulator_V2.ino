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

uint32_t TM_SENS           = 30000;
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
bool     isBusySave       = false;
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
  Serial.println(F("Start Emulation Mode"));  
  Serial.print(CURRENT_FIRMWARE_TITLE);
  Serial.print(F(" Ver "));
  Serial.print(CURRENT_FIRMWARE_VERSION);
  MyLoRaAddress::Get(strID,myLora.Addr);
  Serial.print(F(" ChipID "));
  Serial.println(strID);
  char s[16];
  Serial.print(F("MyLoRa MAC Addr: "));
  Serial.println(MyLoRaAddress::Get(s,myLora.Addr));
  int x = 0;
  for(int i=0; i<strlen(strID);i++)x+=(byte)strID[i];
  startNeopixel();

  eepromBegin();
  eepromDefault();
//  eepromSave();
//  eepromRead(); 
//  sensorsInit();  

//  Attempt = (uint16_t)getAttributeJson(ATT_ATT,2);
//  setAttributes();
  myLora.setTTL(300);
  MyLoRaAddress::Set(myLora.AddrRX,0,0,0,0,0,0);
  initLora();
#if defined(PIN_BTN)
  btn.SetLongClick(1000);
#endif    
  setNeopixel(LM_FREE);
  _tm_lora = TM_LORA;
}

uint32_t _ms0 = 0,  _ms1 = 0, _ms2 = 0, _ms3 = 0;

/**
*  loop() - главный цикл Arduino IDE
*/

uint8_t _emulator_num = 0;
uint8_t _emulator_max = 4;

void loop(void){
   uint32_t _ms = millis();
 


  if( _ms1 == 0 || _ms1 > _ms || _ms - _ms1 > TM_SENS ){
     _ms1 = _ms;  
     Serial.print(F("Emulate node "));
     Serial.println(_emulator_num);
     switch(_emulator_num){
        case 0 : MyLoRaAddress::Set(myLora.Addr,0x13,0xA7,0x52,0x77,0x27,0x15);break;
        case 1 : MyLoRaAddress::Set(myLora.Addr,0x33,0xA7,0x52,0x77,0x0A,0x1a);break;
        case 2 : MyLoRaAddress::Set(myLora.Addr,0x13,0xA7,0x52,0x77,0x28,0x3a);break;
        case 3 : MyLoRaAddress::Set(myLora.Addr,0x33,0xA7,0x52,0x77,0x08,0x26);break;
        default : MyLoRaAddress::Set(myLora.Addr,0xAA,0x00,0x00,0x00,0x00,0x00);break;

     }
//     MyLoRaAddress::Set(myLora.Addr,0xaa,0,0,0,0,_emulator_num++);
     if( _emulator_num >= _emulator_max )_emulator_num = 0;
     getDistanceSR04HAL();
     isBusyReal = Busy_sr04;
     if( Busy_sr04 ){
        Serial.println(F(">>> Park place is busy"));
        setNeopixel(LM_BUSY);
     }
     else {
        Serial.println(F(">>> Park place is free"));
        setNeopixel(LM_FREE);
     }
     sendTelemetry();  
     setNeopixelLoop();
  }
  


}

void setAttributes(){
   Serial.println(F("!!! Set attributes"));
   TM_SENS      = (uint32_t)getAttributeJson(ATT_TM_SENS_S,1)*10000;
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
  if( !isnan(L_sr04) ){
     myLora.SetHeaderTX(PACKET_V3_TYPE_JSON_TELEMETRY, myLora.AddrRX, true);
     myLora.Json["Dist"]   = round(L_sr04);
     myLora.Json["Height"] = round(H_sr04);
     myLora.Json["Busy"]   = round(isBusyReal);

     if (myLora.SetJsonBodyTX()) {
        if( sendLora(Attempt) )_tm_lora = TM_LORA;
        _tm_lora = TM_LORA_ERR;
     }
  }   
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

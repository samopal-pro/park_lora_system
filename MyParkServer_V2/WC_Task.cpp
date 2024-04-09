#include "WC_Task.h"

StaticJsonDocument<JSON_MSG_SIZE> j_msg;

uint16_t mqtt_error  = 0;
uint16_t wifi_error  = 0;
bool wifi_correct    = false;
bool isUpdate        = false;
bool isConnectStatus = false;
bool isNTP           = false;
bool isAP            = false;
//TaskHandle_t clockHandle, clockHandleEffect, weatherHandle, gifHandle;

char _stat_wifi[10], _stat_mqtt[10];
char _stat_setup[20];

int wifiRssi = 0;
Adafruit_NeoPixel pixels(NUM_NEOPIXEL, PIN_NEOPIXEL, NEO_GRB+NEO_KHZ800);
IRsend irsend(PIN_IR_SEND);
uint16_t saveNum = 0xffff;
uint32_t msBtn    = 0;
uint16_t countBtn = 0;
uint32_t msIR = 0;
uint16_t numIR = 0xffff;

#ifdef ETHERNET_W5500
EthernetUDP ntpUDP;
#else
WiFiUDP ntpUDP;
#endif
NTPClient ntpClient(ntpUDP);

SButton btn1( PIN_BTN );
SemaphoreHandle_t btn1Semaphore, displaySemaphore, loraMutex, SemaphoreIR;



/**
 * Старт всех параллельных задач
 */
void tasksStart(){
   SPIFFS.begin(true);
   listDir("/");
   eepromBegin();
   eepromRead();
#ifdef RESET_CONFIG
   bool is_config_change = false;
   if( !j_config.containsKey("version") )is_config_change = true;
   else if( j_config["version"] != CURRENT_CONFIG_VERSION )is_config_change = true;
   if( is_config_change ){
      Serial.println("Change config version");
      eepromDefault();
      eepromSave();  
      eepromRead();
//      attributesDefault();
//      attributesSave();           
   }
#endif   
   attributesRead();
   gatesRead();
   InitIR();   


   startNeopixel();
   xTaskCreateUniversal(taskNeopixel, "neopixel", 4096, NULL, 5, NULL,CORE);
   xTaskCreateUniversal(taskButton1, "btn1", 4096, NULL, 5, NULL,CORE);   
    delay(1000);
//    xTaskCreateUniversal(taskNeopixel, "neopixel", 4096, NULL, 5, NULL,CORE);
    xTaskCreateUniversal(taskWiFi, "wifi", 4096, NULL, 2, NULL, CORE);   
    vTaskDelay(5000);
    SendIR(0);
    vTaskDelay(3000);
    JGates.calculate();
    SendIR(JGates.countFree);
    JGates.changeCountFree = false;
    vTaskDelay(3000);
    xTaskCreateUniversal(taskHttpd, "httpd", 4096, NULL, 1, NULL,CORE);   

}

/**
 * Задача работы с WiFi или Ethernet 
 * @param pvParameters
 */
void taskWiFi( void *pvParameters ){
   Serial.printf("WiFi task start ");
   Serial.println(xPortGetCoreID());
   uint16_t count = 0;
#ifdef ETHERNET_W5500
   ethernetInit();
#else   
  WiFi_ScanNetworks();
  wifiInit();
#endif

   uint32_t _ms0 = 0, _ms1 = 0, _ms2 = 0, _ms3 = 0;
   while(true){
      uint32_t _ms = millis();
      if( countBtn > 0 && _ms - msBtn > 60000 ){
         Serial.println(F("!!!! Reset button timeout"));
         countBtn = 0;
      }
// Режим точки доступа   
#ifdef ETHERNET_W5500  
      if( !isConnectStatus )ethernetReconnect();
      isConnectStatus = ethernetCheck();
#else 
      isConnectStatus = wifiCheck();
#endif
      if( !isConnectStatus ){
         nodesStatus[0]    = NLS_WIFI_OFF;
         wifi_error++;
      }
      else {
         nodesStatus[0]    = NLS_WIFI_ON;
         wifi_error = 0;
      }
      if( isAP ){
          if( WiFi.getMode() == WIFI_STA || WiFi.getMode() == WIFI_OFF ){
               isConnectStatus = false;
               strcpy(_stat_mqtt,"");
//               WiFi.persistent(false);
//               WiFi.disconnect(); //  this alone is not enough to stop the autoconnecter
//               WiFi_ScanNetworks();

               WiFi.mode(WIFI_AP);
               nodesStatus[0]    = NLS_WIFI_AP;
               vTaskDelay(1000);
//               WiFi.persistent(true);
//               if (_apPassword != NULL)WiFi.softAP(j_config["esp"]["name"].as<const char *>(),j_config["esp"]["password"].as<const char *>());
//               else WiFi.softAP(_apName);
               WiFi.softAP(j_config["esp"]["name"].as<const char *>());
               Serial.print(F("AP: start AP mode "));
               Serial.print(j_config["esp"]["name"].as<const char *>());
               Serial.print(" ");
               Serial.println(WiFi.softAPIP());


//               displayStatusWiFi("AP",0B000010101);
               strcpy(_stat_setup,"Wait...");
//               displayGive();
               HTTP_TM       = 300000;
               while(1){
//                  if( HTTP_checkTM() ){
//                     Serial.println(F("HTTPD: timeout server"));
//                     break;
//                  }
                  if( is_http_abort ){
                     Serial.println(F("HTTPD: abort server"));
                    break;
                  }
                  vTaskDelay(1000);
               }               

//               HTTP_startConfig(j_config["esp"]["name"].as<const char *>(),j_config["esp"]["password"].as<const char *>(),300000); 
               WiFi.disconnect(); 
//               WiFi.mode(WIFI_OFF);
               WiFi.mode(WIFI_STA);
               nodesStatus[0]    = NLS_WIFI_OFF;
           } 
           else {
               WiFi.mode(WIFI_STA);
           }
           isAP = false;
           vTaskDelay(1000); 
       }
       
// Проверка очереди отправки на сервер
        if( isConnectStatus && !JHttp.q_msg.empty() ){
          SetSemaphoreIR(true);
          httpSend();
          SetSemaphoreIR(false);
        }      
        count++;

// Каждые 60 секунд проверяем время
       if( _ms2 == 0 || _ms2 > _ms || ( _ms - _ms2 ) > 10000 ){
           _ms2 = _ms;
           SetSemaphoreIR(true);
           if( isConnectStatus ){
               if( !isNTP ){
                   ntpClient.begin();
                   isNTP = true;
               }    
               ntpClient.update();      
//               Serial.print(F("!!! NTP time: "));
//               Serial.println(ntpClient.getEpochTime());
           }
           SetSemaphoreIR(false);
       }

       
       if( _ms0 == 0 || _ms0 > _ms || ( _ms - _ms0 ) > 5000 ){
          _ms0 = _ms;
           SendIR(JGates.countFree);
          if( wifi_correct ){
//             Serial.printf("ERROR: wifi=%d mqtt=%d\n",wifi_error,mqtt_error);
             if( wifi_error > 300 ){
                 Serial.println("Many Wifi error. Reboot ...");
                 ESP.restart();
             }
             if( mqtt_error > 120 ){
                 Serial.println("Many MQTT error. Reboot ...");
                 ESP.restart();
             }
          }
       }

       vTaskDelay(1000);
       
   }  
}


/**
 * Задача HTTPD сервера 
 * @param pvParameters
 */
void taskHttpd( void *pvParameters ){
   Serial.printf("HTTPD task start ");
   Serial.println(xPortGetCoreID());
//   while( !isConnectStatus )vTaskDelay(2000);

   HTTPD_start();    
   while(1){
       SetSemaphoreIR(true);
       HTTPD_loop();  
       SetSemaphoreIR(false);
       vTaskDelay(100);       
   }
   HTTPD_loop();    

/*
  webServer.on("/", handleRoot);

  webServer.on("/inline", []() {
    webServer.send(200, "text/plain", "this works as well");
  });

  webServer.onNotFound(handleNotFound);

  HTTP_begin();
  webServer.begin();
  Serial.println("HTTP server started");
  while(1){
     webServer.handleClient();
     vTaskDelay(100);//allow the cpu to switch to other tasks

  }
*/  

}


/**
 * HTTP send message
 */
void httpSend(){
   if( !isConnectStatus )return;
   for( int i=0; i<10 && !JHttp.q_msg.empty(); i++){
      HTTPClient http;  
      String _msg   = JHttp.q_msg[0];
      Serial.printf("HTTP GET %s\n",_msg.c_str());        
      http.begin( _msg.c_str() );
      int httpCode = http.GET();
      if( httpCode == HTTP_CODE_OK ) {
         Serial.println(http.getString());
         JHttp.q_msg.erase(JHttp.q_msg.begin());
      }
      else
         Serial.printf("Error HTTP %d\n",httpCode);
         JHttp.q_msg.erase(JHttp.q_msg.begin());
      http.end();
   }
}


/**
 * Обработчик прерывания по нажатию кнопки
 */
void IRAM_ATTR ISR_btn1(){
   portBASE_TYPE x = pdFALSE;
   xSemaphoreGiveFromISR( btn1Semaphore, &x );
}

/**
 * Задача работы с кнопками
 */
void taskButton1( void *pvParameters ){
   Serial.print("Start Button Task ");
   Serial.println(xPortGetCoreID());
   btn1Semaphore    = xSemaphoreCreateBinary ();
// Инициализация кнопок   
   pinMode(PIN_BTN,INPUT_PULLUP);
   btn1.SetLongClick(1000);   
//   btn1.SetMultiClick(500);
//   btn1.SetAutoClick(10000,500);   
// Инициализация прерывания по кнопке
   bool bt_stat = false;
   uint8_t isr1 = digitalPinToInterrupt(PIN_BTN);
   Serial.printf("Attach ISR1=%d GPIO1=%d\n",(int)isr1,PIN_BTN);
   attachInterrupt(isr1, ISR_btn1, FALLING); 
   while(true){
      SBUTTON_CLICK ret = SB_NONE;
// Если сработало прерывание кнопки
      if( bt_stat ){
         ret = btn1.Loop();
         if( ret != SB_NONE ){
             Serial.println("Attach ISR1");
             attachInterrupt(isr1, ISR_btn1, FALLING); 
             bt_stat = false;
         }
         vTaskDelay(100);
      }
      else {
// Ждем срабатывания прерывания по кнопке
         Serial.println("Wait click BTN1\n");
         xSemaphoreTake( btn1Semaphore, portMAX_DELAY );
         detachInterrupt(isr1);
         bt_stat = true;
         ret = SB_NONE;
         Serial.println("Detach ISR1");
      }
// Обрабатываем значение кнопки      
      switch( ret ){
         case SB_CLICK :
            Serial.println("BTN1 click");
            break;
         case SB_LONG_CLICK :
            Serial.println("BTN1 long click");
            countBtn++;
            msBtn = millis();
            switch(countBtn){
               case 0 :
                  break;
               case 1 :
                  j_gates.clear();
                  JGates.calculate();
                  SendIR(JGates.countFree);
                  JGates.changeCountFree = false;
                  gatesSave();
                  break;
               case 2 :
                  isAP = true;
                  break;
               default :              
                  delay(2000);   
                  ESP.restart();                  
            }
            break;
         case SB_AUTO_CLICK :
            Serial.println("BTN1 auto click");
            break;
         case SB_MULTI_CLICK :
//            BLed.SeriesBlink(btn1.Count,100);
            Serial.printf("BTN1 multi click %d\n",btn1.Count);
/*
            if( isAP )wifiManager.httpLoopBreak = true;
            else {
               switch(btn1.Count ){
                  case 1 : serviceMode = SM_INPUT;xSemaphoreGive( serviceSemaphore );break; 
                  case 2 : serviceMode = SM_POWER;xSemaphoreGive( serviceSemaphore );break; 
                  case 3 : serviceMode = SM_NETSTAT;xSemaphoreGive( serviceSemaphore );break; 
                  case 4 : serviceMode = SM_TEST;xSemaphoreGive( serviceSemaphore );break; 
                  case 5 :isAP = true; break; 
                  break; 
               }
            }
            */
            break;
      }
   } 
}


/**
 * Задача работы со светодиодами
 */
void taskNeopixel( void *pvParameters ){
   Serial.println("Start Led Task");
   while(true){
#if defined(PIN_NEOPIXEL) && defined(NUM_NEOPIXEL)
      SetSemaphoreIR(true);
      uint32_t _ms = millis();      
//      for( int i=1; i<=JNodes.countNodeAll && i<NUM_NEOPIXEL;i++){
//         if( JNodes.nodesStatus[i] == NLS_NONE ){
//            JNodes.nodesStatus[i] = NLS_EXIST;
//            JNodes.nodesLastMS[i] = 0;
//         }
/*
         else {

            if( JNodes.nodesLastMS[i] != 0 ){
               if( _ms < JNodes.nodesLastMS[i] )JNodes.nodesLastMS[i] = _ms;
               if( JNodes.nodesLastMS[i]+TM_SENSOR_OFFLINE < _ms){
                  if( JNodes.nodesStatus[i] == NLS_OFFLINE )JNodes.nodesStatus[i] = NLS_OFFLINE1;
                  else JNodes.nodesStatus[i] = NLS_OFFLINE;
               }
            }
         }
         */
//      }
    
//      for( int i=JNodes.countNodeAll+1; i<NUM_NEOPIXEL;i++)JNodes.nodesStatus[i] = NLS_NONE;
//      Serial.printf("stat = %d rssi = %d ms = %ul cur=%ul\n", statusNodes[0],rssiNodes[0], msNodes[0], _ms );
      for( int i=0; i<NUM_NEOPIXEL; i++){
         switch( nodesStatus[i]){
           case NLS_NONE    : pixels.setPixelColor(i, 0 );break;
           case NLS_EXIST    : pixels.setPixelColor(i, pixels.Color(127, 127, 127) );break;
           case NLS_OFFLINE  : pixels.setPixelColor(i, pixels.Color(160, 48, 0) );break;
           case NLS_OFFLINE1 : pixels.setPixelColor(i, pixels.Color(127, 127, 127) );break;
           case NLS_FREE     : pixels.setPixelColor(i, pixels.Color(0, 255, 0) );break;
           case NLS_BUSY     : pixels.setPixelColor(i, pixels.Color(255, 0, 0) );break;
           case NLS_WIFI_OFF : pixels.setPixelColor(i, pixels.Color(160, 48, 0) );break;
           case NLS_WIFI_ON  : pixels.setPixelColor(i, pixels.Color(0, 128, 255) );break;
           case NLS_WIFI_AP  : pixels.setPixelColor(i, pixels.Color(0, 0, 255) );break;
         }
      }
      
      pixels.show();
      SetSemaphoreIR(false);

      vTaskDelay(1000);
#else
      vTaskDelay(300000);
#endif       
   }
    
}



void startNeopixel(){
#if defined(PIN_NEOPIXEL) && defined(NUM_NEOPIXEL)
   pixels.begin();
   delay(200);
   uint16_t _hue = 0;
   uint16_t _inc = 65536/NUM_NEOPIXEL;
   for( int i=0; i<60; i++){
      for( int j=0; j<NUM_NEOPIXEL; j++){
         uint16_t h = _hue + _inc*j;
         pixels.setPixelColor(j, pixels.ColorHSV(h, 255, 100));     
      }
      pixels.show();
      _hue += _inc;
//      Serial.println(_hue);
      delay(50);
   }
   for(int i=1; i<NUM_NEOPIXEL; i++) pixels.setPixelColor(i, pixels.Color(127, 127, 127));
   for( int i=0; i<NUM_NEOPIXEL; i++ ){
      nodesStatus[i] = NLS_NONE;
   }
   nodesStatus[NUM_NEOPIXEL-1] = NLS_WIFI_OFF;

   pixels.show();   // Send the updated pixel colors to the hardware.

#else
   Serial.println(F("WS2812 LED is disable"));   
#endif  
}



void listDir(const char * dirname, uint8_t levels){
    Serial.printf("Listing directory: %s\r\n", dirname);

    File root = SPIFFS.open(dirname);
    if(!root){
        Serial.println("- failed to open directory");
        return;
    }
    if(!root.isDirectory()){
        Serial.println(" - not a directory");
        return;
    }

    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if(levels){
                listDir(file.name(), levels -1);
            }
        } else {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("\tSIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
} 

void InitIR(){
   Serial.println(F("!!!! Init IR"));
   irsend.begin();
   SemaphoreIR    = xSemaphoreCreateMutex();
//   xSemaphoreTake( SemaphoreIR, 100 );
}

void SendIR(uint16_t _num){
  uint32_t ms = millis();
   if( ms - msIR < 2000 )return;
   msIR = ms;
   if( _num == saveNum )return;
   SetSemaphoreIR(true);
   saveNum = _num;
   uint8_t _ir_cmd[] = {0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19};

   if( _num == 0 ){
      _num = MAX_NUM_PARKING_SPACE+1;
   }
   Serial.printf("!!!! Send IR %d\n",_num);
   numIR = _num;
   if( _num < 10 ){
       irsend.sendNEC(irsend.encodeNEC(0x00, _ir_cmd[_num]),32,0);
//       IrSender.sendNEC(0x00, _ir_cmd[_num], 0);
   }
   else if( _num < 100){
       irsend.sendNEC(irsend.encodeNEC(0x00, _ir_cmd[_num/10]),32,0);
       delay(100);
       irsend.sendNEC(irsend.encodeNEC(0x00, _ir_cmd[_num%10]),32,0);
   }      
   else  {
       irsend.sendNEC(irsend.encodeNEC(0x00, _ir_cmd[_num/100]),32,0);
       delay(100);
       irsend.sendNEC(irsend.encodeNEC(0x00, _ir_cmd[(_num/10)%10]),32,0);
       delay(100);
       irsend.sendNEC(irsend.encodeNEC(0x00, _ir_cmd[_num%10]),32,0);
     
   }       
   SetSemaphoreIR(false);
}

void SetSemaphoreIR(bool flag){
   if( flag ){
//      Serial.println(F("!!!! Wait IR Semaphore"));
      xSemaphoreTake( SemaphoreIR, portMAX_DELAY );
//      Serial.println(F("!!!! Set IR Semaphore ON"));  
   }
   else {
//      Serial.println(F("!!!! Set IR Semaphore OFF"));
      xSemaphoreGive( SemaphoreIR ); 
      
   }
   
}
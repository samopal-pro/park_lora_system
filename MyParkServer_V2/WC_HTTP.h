/**
* HTTP server
* 
* Copyright (C) 2016 Alexey Shikharbeev
* http://samopal.pro
*/
#ifndef WS_HTTP_h
#define WS_HTTP_h
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <ESPmDNS.h>
#include <Update.h>
#include "WC_EEPROM.h"
#include "WC_Task.h"
#include "WC_Json.h"
#include <String>

//extern WiFiManager wifiManager;
extern bool is_http_abort;
extern uint32_t HTTP_TM;
extern WebServer webServer;


typedef struct HTTP_page_t {
   String Name;
   String Link;
   String Description;
   void (*Handle)(void);  
};

typedef enum {
  HT_TEXT     = 0,
  HT_PASSWORD = 1,
  HT_NUMBER   = 2,
  HT_IP       = 3,
  
}HTTP_input_type_t;

void handleRoot();
void handleNotFound();

void HTTPD_start();
void HTTPD_loop();
void HTTPD_stop();

void HTTP_begin(void);
void HTTP_stop(void);
bool HTTP_checkTM();
bool HTTP_redirect();

void HTTP_handleRoot(void);
void HTTP_handleInline(void);
void HTTP_handleUpdate(void);
void HTTP_handleUpdateFinal(void);
void HTTP_handleUpdateBegin(void);
void HTTP_handleUpload(void);
void HTTP_handleNotFound(void);
void HTTP_handleConfigWIFI(void);
void HTTP_handleConfigHTTP(void);
void HTTP_handleAPI(void);
void HTTP_handleLogo(void);

void HTTP_printNodeStatus(const char *node,String &out);
void HTTP_printNodeAge(const char *node, String &out);

void HTTP_handleReboot(void);
void HTTP_handleReset(void);
void HTTP_handleDownload(void);
void HTTP_handleDefault(void);
void HTTP_handleTest(void);
void HTTP_logo1(String &out);

void HTTP_printInput(String &out,const char *label, const char *name, const char *value, int size, int len, HTTP_input_type_t htype=HT_TEXT, const char *style=NULL, const char *add_text=NULL);

void HTTP_printCSS(String &out);
void HTTP_printHeader(String &out,int num=0, uint16_t refresh=0);
void HTTP_printHeader(String &out,String name, uint16_t refresh=0);
int numPages( String name );

void HTTP_printTail(String &out);
//void SetPwm();
void HTTP_device(void);
//void HTTP_handleGraph(void);
//void HTTP_handleData(void);


void WiFi_ScanNetworks();
void printNetworks();
void HTTP_printNetworks(String &out, const char *name);
#endif

/**
* HTTP server
* 
* Copyright (C) 2016 Alexey Shikharbeev
* http://samopal.pro
*/
#include "WC_HTTP.h"
#include "crmlogo.h"


std::vector <HTTP_page_t> Pages;
DNSServer dnsServer;
WebServer webServer(80);

uint32_t msScan = 0;
std::vector <String> n_ssid;
std::vector <int> n_rssi;

uint32_t HTTP_TM=0, http_ms;
bool is_http_abort = false;
bool is_update     = false;


void handleNotFound() {
  String message = "Страница не найдена\n\n";
  message += "URI: ";
  message += webServer.uri();
  message += "\nMethod: ";
  message += (webServer.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += webServer.args();
  message += "\n";
  for (uint8_t i = 0; i < webServer.args(); i++) {
    message += " " + webServer.argName(i) + ": " + webServer.arg(i) + "\n";
  }
  webServer.send(404, "text/plain", message);
}

/**
 * Запуск WIFI и HTTP сервера
 * 
 * @param _apName - SSID точки доступа
 * @param _apPassword - пароль
 * @param _tm - таймаут завершения режима если ничего не происходит
 */
void HTTPD_start(){
// Стартуем DNS сервер   
   dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
   dnsServer.start(53, "*", WiFi.softAPIP());
// Стартуем WEBсервер
   HTTP_begin();
   webServer.begin();
// Запускаем цикл
}

uint32_t _http_cont = 0;

void HTTPD_loop(){
   dnsServer.processNextRequest();   
   webServer.handleClient(); 
//   if( _http_cont%50 == 0 )Serial.printf("Http loop %ul ...\n",_http_cont);
//   _http_cont++;   
/*   
   if( HTTP_checkTM() ){
       Serial.println(F("HTTPD: timeout server"));
       break;
   }
   if( is_http_abort ){
       Serial.println(F("HTTPD: abort server"));
       break;
   }
*/   
}

void HTTPD_stop(){
// Останавливаем DNS и WEB серверы   
   webServer.stop();
   dnsServer.stop();
//   WiFi.mode(WIFI_OFF);
   Serial.println(F("HTTPD: exit"));
}

/**
 * Проверка таймаута сервера
 * 
 * @return true - таймаут еще не завершен
 * @return false - таймаут завершен
 */
bool HTTP_checkTM(){
  if( HTTP_TM ==  0 )return false;
  uint32_t _ms = millis();
  if( _ms < http_ms || ( _ms - http_ms ) > HTTP_TM )return true; 
  return false;
}

/** 
* Редирект с любого домена на главную страницу сервера
* 
* @return - true если сработал редирект
* @return false - редирект не сработал
*/
bool HTTP_redirect() {
//  if( !isAP )return true;

  String serverLoc =  webServer.client().localIP().toString();
  
  if (serverLoc != webServer.hostHeader() ) {
    Serial.print(F("HTTPD: redirect "));
    Serial.print(webServer.hostHeader());
    Serial.print(F(" to "));
    Serial.println(serverLoc);  
    webServer.sendHeader(F("Location"), (String)F("http://") + serverLoc, true); // @HTTPHEAD send redirect
    webServer.send ( 302, "text/plain", ""); // Empty content inhibits Content-length header so we have to close the socket ourselves.
    webServer.client().stop(); // Stop is needed because we sent no content length
    return true;
  }
  return false;
}




/**
 * Старт WEB сервера
 */
void HTTP_begin(void){  
   Pages.clear();
   Pages.push_back({String("Состояние"),    String("/"),            String("Состояние шлюза"),             HTTP_handleRoot});
   Pages.push_back({String("Соединение"),   String("/config_wifi"), String("Настройка соединения"),        HTTP_handleConfigWIFI});
//   Pages.push_back({String("Сервер"),       String("/config_http"), String("Настройка доступа к серверу"), HTTP_handleConfigHTTP});
   Pages.push_back({String("Сенсоры"),       String("/config_list"), String("Список сенсоров"),            HTTP_handleConfigAdv});
   Pages.push_back({String("Сервера"),       String("/config_servers"), String("Список серверов"),         HTTP_handleConfigAdv1});
   Pages.push_back({String("Прошивка"),   String("/update"),        String("Обновление прошивки"),         HTTP_handleUpload});
   Pages.push_back({String("Перезагрузка"), String("/reboot"),      String("Перезагрузка шлюза"),          HTTP_handleReboot});
// Startup DNS redirect  
// Startup HTTP server  
   webServer.on ( "/logo.png", HTTP_handleLogo );

    for( int i=0; i<Pages.size(); i++ ){
       webServer.on(Pages[i].Link.c_str(),HTTP_GET,Pages[i].Handle);  
    }
    webServer.on("/update", HTTP_POST, HTTP_handleUpdateFinal, HTTP_handleUpdate);
    webServer.onNotFound ( HTTP_handleNotFound );
    const char * headerkeys[] = {"User-Agent","Cookie"} ;
    size_t headerkeyssize = sizeof(headerkeys)/sizeof(char*);
    webServer.collectHeaders(headerkeys, headerkeyssize ); 
    webServer.begin();
    Serial.printf( "HTTP server started ...\n" );
    http_ms = millis();
    is_http_abort = false;
  
}

/**
 * Не испольщуется
 */
void HTTP_stop(void){ 
}

/**
 * Формирование CSS стилей WEB-сервера
 * 
 * @param out - строковый буфер
 */
void HTTP_printCSS(String &out){
    out += "<style>\n";
    out += " body { background-color:#cccccc;padding:20px; }\n";
    out += " .v-menu {width:130px;float:left;font-size:20px;}\n";
    out += " .v-menu a {background:#cccccc;color:#000088;display:block;padding:12px;text-decoration:none;}\n";
    out += " .v-menu a:hover {background-color: #909090;}\n";
    out += " .v-menu a.active {background:#808080;color:#fbfbfb;}\n";
    out += " .main {width:500;height:100%;background:#808080;color:#fbfbfb;padding:10px;border-radius:16px;float:left;}\n";
    out += " .main label {display:block;float:left;width:150;}\n";
    out += " .main input {background:#cccccc;color:#000088;border:0;}\n";
    
//    out += " .main input[type=\"file\"] {display: none;}\n";
//    out += " .file-upload {cursor: pointer;padding: 6px 12px;display: inline-block;border: 1px solid #ccc;background:#fceade;color:#c55a11;width=100%}\n";
    out += " .main select {background:#cccccc;color:#000088;border:0;}\n";
    out += " .main fieldset {border-color:#fbfbfb;}\n";
    out += " .main legend {font-size:20px;font-weight:bold;}\n";
    out += " .col1 {width:50%;float:left;}\n";
    out += " .col2 {width:45%;float:left;}\n";
    out += " .text {width:100%;float:left;}\n";
    out += " .tail {position:absolute;bottom:20;}\n";
    out += " .imp2 {margin-top:3px;float:left;}\n";
    out += " hr {border-top:1px solid #fbfbfb;}\n";

    out += " input[type=file]::file-selector-button {border: 2px solid #000088;background:#fceade;color:#c55a11;width:30%;}\n";
    out += " input[type=file]::file-selector-button:hover {background:#000088;}\n";
    out += " input[type=submit] {border: 2px solid #000088;}\n";
//  border: 2px solid #6c5ce7;
//  padding: .2em .4em;
//  border-radius: .2em;
//  background-color: #a29bfe;
//  transition: 1s;
//}

//input[type=file]::file-selector-button:hover {
//  background-color: #81ecec;
//  border: 2px solid #00cec9;
//}
    
    out += "</style>\n";  
}

/**
 * Формирование постоянной заголовочной части каждой сираницы
 * 
 * @param out - строковый буфер
 * @param num - номер активной страницы для меню
 * @param refresh - таймаут перезашрузки страницы в сек. 0 - если не нужно
 */
void HTTP_printHeader(String &out,int num, uint16_t refresh){
  String name = "Не найдена";
  String desc = "Страница не найдена";
  if( num >= 0 && num < Pages.size() ){
      desc = Pages[num].Description;
      name = Pages[num].Name;
      strcpy(_stat_setup,Pages[num].Name.c_str());
  }
  
 
  out += "<html>\n<head>\n<meta charset=\"utf-8\" />\n";
  if( refresh ){
     char str[10];
     sprintf(str,"%d",refresh);
     out += "<meta http-equiv='refresh' content='";
     out +=str;
     out +="'/>\n"; 
  }
  out += "<title>";
  out += desc;
  out += "</title>\n";
  HTTP_printCSS(out);
  out += "<body>\n";
  

  out += " <div class=\"v-menu\">\n   <p>&nbsp;</p>\n";
  for( int i=0; i< Pages.size(); i++ ){
     out += "  <a href=\"";
     out += Pages[i].Link;
     out += "\"";
     if( i == num )out += " class=\"Active\"";
     out += ">";
     out += Pages[i].Name;
     out += "</a>\n";
  }
  out+= " </div>\n";

  out += " <div class=\"main\" id=\"main\">\n  ";
//  out += "<div class=\"Head\">\n";
  out += "<h1>LORA PARKING SYSTEM<br>";
  out += "<img src=/logo.png></h1>\n";
//  out += "</div>\n";


  out += "  <h2>";
  out += desc;
  out += "</h2>\n";
  Serial.println(desc);
    
  http_ms = millis();
}   

/**
 * Формирование постоянной заголовочной части каждой сираницы
 * 
 * @param out - строковый буфер
 * @param name - имя активной страницы для меню
 * @param refresh - таймаут перезашрузки страницы в сек. 0 - если не нужно
 */
void HTTP_printHeader(String &out, char *name, uint16_t refresh){
   HTTP_printHeader(out, numPages(name), refresh );  
}

/**
 * Поиск номера активной страницы по имени
 * 
 * @param name - имя страницы
 */
int numPages( String name ){
   for( int i=0; i< Pages.size(); i++){
      if( Pages[i].Name == name )return i;
   }
   return -1;  
}


/**
 * Формирование одного элемента INPUT в форме
 * 
 * @param out - строковый буфер
 * @param label - метка HTTP
 * @param name - имя отображаемое на форме
 * @param value - ткущее значение 
 * @param size - ширина текстового поля
 * @param len - максимальное количество символов
 * @param type - тип поля ввода: HT_TEXT,HT_PASSWORD,HT_NUMBER,HT_IP
 */
void HTTP_printInput(String &out,const char *label, const char *name, const char *value, int size, int len, HTTP_input_type_t htype, const char *style, const char *add_text){
   char str[10];
   if( style == NULL )out += "  <p><label>";
   else {
       out += "  <div class=\"";
       out += style;
       out += "\"><label>";
   }
   out += label;
   out += "</label><input name ='";
   out += name;
   out += "' value='";
   out += value;
   out += "' size=";
   sprintf(str,"%d",size);  
   out += str;
   out += " maxlength=";    
   sprintf(str,"%d",len);  
   out += str;
   if( htype == HT_PASSWORD )out += " type='password'";
   if( htype == HT_NUMBER )out += " type='number'";
   if( htype == HT_IP ){
      out += " placeholder=\"xxx.xxx.xxx.xxx\"";
      out += " pattern=\"^(([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])\.){3}([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])$\"";
   }
   out += ">";
   if( add_text != NULL )out += add_text;
   if( style == NULL )out += "</p>\n";
   else out += "</div>\n";
     
}

/**
 * Вывод логотипа
  */

void HTTP_handleLogo() {
  webServer.send_P(200, PSTR("image/png"), logo, sizeof(logo));
}

 
/**
 * Формирование "хвоста" страницы
 * 
 * @param out - текстовый буфер
 */
void HTTP_printTail(String &out){
  out += "<div class=\"tail\"><hr align=\"left\" width=\"500\">Copyright (C) Miller-Ti, A.Shikharbeev, 2023</div>\n";


  out += " </div>";
  out += "</body>\n</html>\n";
}

/**
 * Формирование страницы 404
 */
void HTTP_handleNotFound() {
  if( HTTP_redirect() )return;

  String message = "";
//  HTTP_printHeader(message,-1,0); 
  message += "Page not found\n\n";
  message += "URI: ";
  message += webServer.uri();
  message += "\nMethod: ";
  message += (webServer.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += webServer.args();
  message += "\n";
  for (uint8_t i = 0; i < webServer.args(); i++) {
    message += " " + webServer.argName(i) + ": " + webServer.arg(i) + "\n";
  }
//  HTTP_printTail(message);
  webServer.send(404, "text/plain", message);
}

/**
 * Формирование корневой страницы "Status"
 */
void HTTP_handleRoot() {
  if( HTTP_redirect() )return;
  
  int numP = numPages("Состояние");
  String content = "";
  HTTP_printHeader(content,numP,0);
  content += "<div class=\"col1\">Название:</div><div class=\"col2\">";
  content += j_config["esp"]["name"].as<String>();
  content += "</div><br><hr>\n";
  content += "<div class=\"col1\">Идентификатор:</div><div class=\"col2\">";
  content += j_config["esp"]["id"].as<String>();
  content += "</div><br><hr>\n";
  content += "<div class=\"col1\">Версия прошивки:</div><div class=\"col2\">";
  content += CURRENT_FIRMWARE_VERSION;
  content += "</div><br><hr>\n";
  content += "<div class=\"col1\">Версия конфига:</div><div class=\"col2\">";
  content += CURRENT_CONFIG_VERSION;
  content += "</div><br><hr>\n";
/*  
  content += "<div class=\"col1\">Local time:</div><div class=\"col2\">";
  char s[24];
  time_t t = (time_t)ntpClient.getEpochTime()+ j_attributes["TZ"].as<long>()*3600;
//  time_t t = time(nullptr);                   
  struct tm *ti = localtime( &t ); 
  sprintf(s,"%02d.%02d.%04d %02d:%02d\n",ti->tm_mday,ti->tm_mon+1,ti->tm_year+1900,ti->tm_hour,ti->tm_min);           
  content += s;
  content += "</div><br><hr>\n";
*/

 //  sprintf(s_tm,"%ld",time(nullptr));
  content += "<div class=\"col1\">MAC алрес:</div><div class=\"col2\">";
  content += WiFi.macAddress();
  content += "</div><br><hr>\n";
  content += "<div class=\"col1\">Паркомест всего:</div><div class=\"col2\">";
  content += String(j_attributes["Nodes"].size());
  content += "</div>\n<div class=\"col1\">Паркомест Online:</div><div class=\"col2\">";
  content += String(JNodes.countNodeOnline);
  content += "</div>\n<div class=\"col1\">Паркомест свободно:</div><div class=\"col2\">";
  content += String(JNodes.countNodeFree);
  content += "</div>\n<div class=\"col1\">Паркомест занято:</div><div class=\"col2\">";
  content += String(JNodes.countNodeBusy);
  content += "</div><br>\n";

 
  content +="<div class=\"text\"><hr><form action='/' method='GET'>\n<input type='submit' value='Обновить' name='R'></form></div>\n"; 
     
//  content += "</div>\n";

 
  HTTP_printTail(content);
  webServer.send(200, "text/html", content);
}


 

/*
 * Формирование страницы с настройувми WiFi "Connection"
 */
void HTTP_handleConfigWIFI(void) {
  if( HTTP_redirect() )return;
  
  for( int i=0; i<webServer.args(); i++ ){
     Serial.print("Argnum=");
     Serial.print(i);
     Serial.print(" Name=");
     Serial.print(webServer.argName(i));
     Serial.print(" Value=");
     Serial.println(webServer.arg(i));
  }

  if ( webServer.args()> 0 ){
     if( webServer.hasArg("ESP_NAME")    )j_config["esp"]["name"]      = webServer.arg("ESP_NAME");
     if( webServer.hasArg("WIFI_SSID")   )j_config["wifi"]["name"]     = webServer.arg("WIFI_SSID");
     if( webServer.hasArg("WIFI_PASS")   )j_config["wifi"]["password"] = webServer.arg("WIFI_PASS");
     if( webServer.hasArg("STATIC_IP")   )j_config["ip"]["type"]       = "static";
     else j_config["ip"]["type"]       = "dhcp";
     if( webServer.hasArg("IP_ADDR")     )j_config["ip"]["address"]    = webServer.arg("IP_ADDR");
     if( webServer.hasArg("IP_MASK")     )j_config["ip"]["netmask"]    = webServer.arg("IP_MASK");
     if( webServer.hasArg("IP_GATE")     )j_config["ip"]["gateway"]    = webServer.arg("IP_GATE");
     if( webServer.hasArg("IP_DNS")      )j_config["ip"]["dns"]        = webServer.arg("IP_DNS");
     eepromSave();    
     eepromRead();    
     String header = "HTTP/1.1 301 OK\r\nLocation: /config_wifi\r\nCache-Control: no-cache\r\n\r\n";
     webServer.sendContent(header);
     strcpy(_stat_setup,"Save...");
     return;
  }
  String content = "";
  int numP = numPages("Соединение");
  HTTP_printHeader(content,numP,0);
  content += "  <form action='/config_wifi' method='GET'>\n";
 
  content += "   <fieldset>\n";
  content += "    <legend>Настройка шлюза</legend>\n";
  HTTP_printInput(content,"Название:","ESP_NAME",(const char *)j_config["esp"]["name"],16,32,HT_TEXT);
  content += "   </fieldset>&nbsp;\n";

  content += "   <fieldset>\n";
  content += "    <legend>Настройка WiFi</legend>\n";
  HTTP_printNetworks(content,"WIFI_SSID");
  HTTP_printInput(content,"Пароль:","WIFI_PASS",(const char *)j_config["wifi"]["password"],16,32,HT_PASSWORD);
  content += "   </fieldset>&nbsp;\n";
  content += "   <fieldset>\n";
  content += "    <label>Статический IP:</label>\n";
  content += "    <input type=\"checkbox\" value=\"static\" name=\"STATIC_IP\" onclick=\"submit();\"";
  if( j_config["ip"]["type"].as<String>() == "static" ){
     content += " checked>\n";
     HTTP_printInput(content,"Адрес:","IP_ADDR",(const char *)j_config["ip"]["address"],16,32,HT_IP);
     HTTP_printInput(content,"Маска:","IP_MASK",(const char *)j_config["ip"]["netmask"],16,32,HT_IP);
     HTTP_printInput(content,"Шлюз:","IP_GATE",(const char *)j_config["ip"]["gateway"],16,32,HT_IP);
     HTTP_printInput(content,"DNS:",    "IP_DNS", (const char *)j_config["ip"]["dns"],16,32,HT_IP);
  }
  else content += ">\n";
  
  content += "   </fieldset>&nbsp;\n";
  content += "   <fieldset>\n";
  content +="<input type='submit' name='Save' value='Сохранить'>"; 
  content += "   </fieldset>\n";
    
  content += "  </form>\n ";
  HTTP_printTail(content);
  webServer.send ( 200, "text/html", content );
   

}        

/*
 * Формирование страницы с настройками MQTT "MQTT"
 */
void HTTP_handleConfigHTTP(void) {
  if( HTTP_redirect() )return;
  
  for( int i=0; i<webServer.args(); i++ ){
     Serial.print("Argnum=");
     Serial.print(i);
     Serial.print(" Name=");
     Serial.print(webServer.argName(i));
     Serial.print(" Value=");
     Serial.println(webServer.arg(i));
  }

  if ( webServer.args()> 0 ){
     if( webServer.hasArg("HTTP_SERVER")     )j_config["http"]["server"]                 = webServer.arg("HTTP_SERVER");
     if( webServer.hasArg("HTTP_PORT")       )j_config["http"]["port"]                   = webServer.arg("HTTP_PORT").toInt();
     if( webServer.hasArg("HTTP_PATH")       )j_config["http"]["path"]                   = webServer.arg("HTTP_PATH");
     eepromSave();    
     eepromRead();    
     String header = "HTTP/1.1 301 OK\r\nLocation: /config_http\r\nCache-Control: no-cache\r\n\r\n";
     webServer.sendContent(header);
     return;
  }
  String content = "";
  int numP = numPages("Сервер");
  char str[20];
  HTTP_printHeader(content,numP,0);
  content += "  <form action='/config_http' method='GET'>\n";
  content += "   <fieldset>\n";
  content += "    <legend>Настройка доступа к серверу</legend>\n";
  HTTP_printInput(content,"Сервер:","HTTP_SERVER",(const char *)j_config["http"]["server"],16,32);
  sprintf(str,"%d",j_config["http"]["port"].as<int>());
  HTTP_printInput(content,"Порт:","HTTP_PORT",str,16,32,HT_NUMBER);
  HTTP_printInput(content,"Путь:","HTTP_PATH",(const char *)j_config["http"]["path"],16,32);
  content += "   </fieldset>&nbsp;\n";
  content += "   <fieldset>\n";
  content +="<input type='submit' name='Save' value='Сохранить'>"; 
  content += "   </fieldset>\n";
  
  content += "  </form>\n";
  HTTP_printTail(content);
  webServer.send ( 200, "text/html", content ); 
}       

/*
 * Формирование страницы со списокм сенсоров
 */
void HTTP_handleConfigAdv(void) {
  if( HTTP_redirect() )return;
  
  for( int i=0; i<webServer.args(); i++ ){
     Serial.print("Argnum=");
     Serial.print(i);
     Serial.print(" Name=");
     Serial.print(webServer.argName(i));
     Serial.print(" Value=");
     Serial.println(webServer.arg(i));
  }

  if ( webServer.args()> 0 ){
     j_attributes["Nodes"].clear();  
     if( !webServer.hasArg("Clean") ){
        for( int i=0; i<webServer.args(); i++ ){
//           if( webServer.argName(i) == "TZ" )j_attributes["TZ"]     = webServer.arg(i).toInt();
           if( webServer.argName(i) == "NODE[]" && webServer.arg(i)!= ""  )
              j_attributes["Nodes"].add(webServer.arg(i));
        }
      
     }
     attributesSave();    
     attributesRead();    
     if( nodesSynchronization() )nodesSave();
     String header = "HTTP/1.1 301 OK\r\nLocation: /config_list\r\nCache-Control: no-cache\r\n\r\n";
     webServer.sendContent(header);
     return;
  }
  String content = "";
  int numP = numPages("Сенсоры");
  char str[20];
  String add_str;
  
  HTTP_printHeader(content,numP,0);
  content += "  <form action='/config_list' method='GET'>\n";
//  content += "   <fieldset>\n";
//  content += "    <legend>Timezone setup</legend>\n";
//  sprintf(str,"%d",j_attributes["TZ"].as<int>());
//  HTTP_printInput(content,"Timezone:","TZ",str,16,32,HT_NUMBER);
//  content += "   </fieldset>";
  content += "   <fieldset>\n";
  content += "    <legend>Список сенсоров</legend>\n";

  JsonArray arr = j_attributes["Nodes"].as<JsonArray>();
  int i=1;
  for (JsonArray::iterator it=arr.begin(); it!=arr.end(); ++it,i++){
//     sprintf(str,"%d",it->as<int>());
     sprintf(str,"Сенсор %d:",i);
     HTTP_printNodeStatus(it->as<const char *>(),add_str);    
     HTTP_printInput(content,str,"NODE[]",it->as<const char *>(),16,32,HT_TEXT,"imp2",add_str.c_str());
  } 
  HTTP_printInput(content,"Добавить:","NODE[]","",16,32,HT_TEXT,"imp2");
  content += "   </fieldset>&nbsp;\n";
  content += "   <fieldset>\n";
  content +="<input type='submit' name='Save' value='Сохранить'>"; 
  content +="&nbsp;&nbsp; <input type='submit' name='Clean' value='Очистить'>"; 
  content += "   </fieldset>&nbsp;\n";
    
  content += "  </form>\n";
  HTTP_printTail(content);
  webServer.send ( 200, "text/html", content );
   

}       

/*
 * Формирование страницы с списоком серверов
 */
void HTTP_handleConfigAdv1(void) {
  if( HTTP_redirect() )return;
  
  for( int i=0; i<webServer.args(); i++ ){
     Serial.print("Argnum=");
     Serial.print(i);
     Serial.print(" Name=");
     Serial.print(webServer.argName(i));
     Serial.print(" Value=");
     Serial.println(webServer.arg(i));
  }

  if ( webServer.args()> 0 ){
     j_attributes["Servers"].clear();  
     if( !webServer.hasArg("Clean") ){
        for( int i=0; i<webServer.args(); i++ ){
//           if( webServer.argName(i) == "TZ" )j_attributes["TZ"]     = webServer.arg(i).toInt();
           if( webServer.argName(i) == "SERVERS[]" && webServer.arg(i)!= ""  )
              j_attributes["Servers"].add(webServer.arg(i));
        }
      
     }
     attributesSave();    
     attributesRead();    
     String header = "HTTP/1.1 301 OK\r\nLocation: /config_servers\r\nCache-Control: no-cache\r\n\r\n";
     webServer.sendContent(header);
     return;
  }
  String content = "";
  int numP = numPages("Сервера");
  char str[20];
  String add_str;
  
  HTTP_printHeader(content,numP,0);
  content += "  <form action='/config_servers' method='GET'>\n";
  content += "   <fieldset>\n";
  content += "    <legend>Список серверов</legend>\n";

  JsonArray arr = j_attributes["Servers"].as<JsonArray>();
  int i=1;
  for (JsonArray::iterator it=arr.begin(); it!=arr.end(); ++it,i++){
//     sprintf(str,"%d",it->as<int>());
     sprintf(str,"Сервер %d:",i);
     HTTP_printServerAge(it->as<const char *>(),add_str);    
     HTTP_printInput(content,str,"SERVERS[]",it->as<const char *>(),16,32,HT_TEXT,"imp2",add_str.c_str());
  } 
  HTTP_printInput(content,"Добавить:","SERVERS[]","",16,32,HT_TEXT,"imp2");
  content += "   </fieldset>&nbsp;\n";
  content += "   <fieldset>\n";
  content +="<input type='submit' name='Save' value='Сохранить'>"; 
  content +="&nbsp;&nbsp; <input type='submit' name='Clean' value='Очистить'>"; 
  content += "   </fieldset>&nbsp;\n";
    
  content += "  </form>\n";
  HTTP_printTail(content);
  webServer.send ( 200, "text/html", content );
   

}       


void HTTP_printNodeStatus(const char *_node,String &out){
   out = "";
   int _rssi = 0;
//   Serial.println(_node);
   if( j_nodes[_node].containsKey("rssi") )_rssi = j_nodes[_node]["rssi"].as<int>();
   if( !j_nodes[_node].containsKey("status") ){
      out = " Нет данных";      
//   Serial.println("!!! 1");
   }
   else if( j_nodes[_node]["status"].as<String>() == "off" ){
      out = " Нет связи";
//      HTTP_printNodeAge(_node, out);
//   Serial.println("!!! 2");
   }
   else if( j_nodes[_node]["status"].as<String>() == "free" ){
      out =" Свободно, ";
      out += String(_rssi);
      out += "дБ";       
      HTTP_printNodeAge(_node, out);      
//   Serial.println("!!! 3");
   }
   else if( j_nodes[_node]["status"].as<String>() == "busy" ){
      out =" Занято, ";
      out += String(_rssi);
      out += "дБ"; 
      HTTP_printNodeAge(_node, out);      
//   Serial.println("!!! 4");
   }
   else {
      out = " Нет данных";
//   Serial.println("!!! 5");
   }

}

void HTTP_printNodeAge(const char *_node, String &out){
   if( !j_nodes[_node].containsKey("ms") )return;
   uint32_t ms = millis() - j_nodes[_node]["ms"].as<uint32_t>();
   char s[20];
   if( ms == 0 )return;
   ms /= 1000;
   out += ", ";
   if( ms < 60 ){
      out += String((int)ms);
      out += " сек";
      return;
   }
   ms /= 60;
   if( ms < 60 ){
      out += String((int)ms);
      out += " мин";
      return;
   }
   ms /= 60;
   if( ms < 24 ){
      out += String((int)ms);
      out += " час";
      return;
   }
   ms /= 24;
   out += String((int)ms);
   out += " дн";

}

void HTTP_printServerAge(const char *_server, String &out){
   out = "";
   if( !j_servers[_server].containsKey("ms_ok") )return;
   uint32_t ms = millis() - j_servers[_server]["ms_ok"].as<uint32_t>();
   char s[20];
   if( ms == 0 )return;
   ms /= 1000;
   if( ms < 60 ){
      out += String((int)ms);
      out += " сек";
      return;
   }
   ms /= 60;
   if( ms < 60 ){
      out += String((int)ms);
      out += " мин";
      return;
   }
   ms /= 60;
   if( ms < 24 ){
      out += String((int)ms);
      out += " час";
      return;
   }
   ms /= 24;
   out += String((int)ms);
   out += " дн";
//   Serial.print("!!!! ServerAge: ");
//   Serial.println(_server);

}


/*
 * Формирование страницы перезагрузки шлюза "Reboot"
 */
void HTTP_handleReboot(void) {
  if( HTTP_redirect() )return;
  
  String content="";
  HTTP_printHeader(content,"Перезагрузка",0);
  content += "  Перезагрузка устройства через 3 сек\n";
  HTTP_printTail(content);
  webServer.send(200, "text/html", content);
  delay(2000);
  ESP.restart();    
}


/*
 * Процесс обновления прошивки через WEB
 */
void HTTP_handleUpdate() {
   if( is_update ){
//      String content;
//      HTTP_printHeader(content,"Update",0);
//      content += "Update firmware. Please wait about 30 sec\n";   
 //     HTTP_printTail(content);
//      webServer.send(200, "text/html", content);
      is_update = false;   
   }
//  Serial.println("Update Page");
//  strcpy(_stat_setup,"Update FW...");
//  displayGive();
  HTTPUpload& upload = webServer.upload();
  if (upload.status == UPLOAD_FILE_START) {
     Serial.setDebugOutput(true);
     Serial.printf("Update: %s\n", upload.filename.c_str());
     if (!Update.begin()) { //start with max available size
        Update.printError(Serial);
     }
  } else if (upload.status == UPLOAD_FILE_WRITE) {
     if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
       Update.printError(Serial);
     }
  } else if (upload.status == UPLOAD_FILE_END) {
     if (Update.end(true)) { //true to set the size to the current progress
       Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
     } else {
       Update.printError(Serial);
     }
     Serial.setDebugOutput(false);
   } else {
        Serial.printf("Update Failed Unexpectedly (likely broken connection): status=%d\n", upload.status);
   }
}

/**
 * Выдача страницы после окончания обновления прошивки
 */
void HTTP_handleUpdateFinal(void){
   is_update = false;
   String content;
   HTTP_printHeader(content,"Прошивка",0);
   if( Update.hasError() ){
       content += "<p>Ошибка обновления прошивки\n<p>";
       content += Update.errorString();
   }
   else {
       content += "Прошивка успешно обновленаю Перезагрузка через 2 сек.\n";   
   }
   HTTP_printTail(content);
   webServer.send(200, "text/html", content);
   if( !Update.hasError() ){
     delay(2000);
     ESP.restart();          
   }
}

/**
 * Выдаче страницы обновления прошивки "Update"
 */
void HTTP_handleUpdateBegin(void){
   String content;
   HTTP_printHeader(content,"Прошивка",0);
   content += "Загрузка прошивки займет примерно 30\n";  
   content += "<script type=\"text/javascript\">\n";
   content += " setTimeout(function(){location=\"https://update\";}, 5000);\n"; 
   content += " </script>\n"; 
   HTTP_printTail(content);
   webServer.send(200, "text/html", content);
}

/**
 * Выдача страницы с формой загрузки файла прошивки "Update"
 */
void HTTP_handleUpload() {
  if( HTTP_redirect() )return;
  
   is_update = true;
  String content;
  HTTP_printHeader(content,"Прошивка",0);
  content += "<script>\n";
//  content += "function F1(){$('#main').html(\"Begin firmware update. Please wait 30 sec\");}\n";
  content += "</script>\n";
  content += " <form method='POST' action='/update' enctype='multipart/form-data'>\n";
  content += "  <fieldset>\n";
// content += "    <legend>Upload firmware</legend>\n";
//  content += "    <p><label class=\"file-upload\">";
  content += "    <p><input type='file' name='update' id=\"userfile\" size=\"30\">\n";
//  content += "    <span id=\"userfile-name\">Not file firmware</span>\n";
//  content += "    <script type=\"text/javascript\">\n";
//  content += "      $('#userfile').change(function(e) {\n";
//  content += "        $('#userfile-name').text(this.files[0].name);\n";
//  content += "      })\n";
//  content += "    </script>\n";
  
//  content += "    </label>\n";
//  content += "    <label for=\"file_upload\">Select file</label>\n";    
  content += "    <p><input type='submit' name='Update' value='Прошивка'>\n";  content += "  </fieldset>\n";
  content += "</form>\n";
  HTTP_printTail(content);
  webServer.send(200, "text/html", content);
}


/**
 * Сканирование доступных WiFi сетей
 */
void WiFi_ScanNetworks(){
   Serial.println("Scan networks...");
   WiFi.mode(WIFI_OFF);
   n_ssid.clear();
   n_rssi.clear();
   int n = WiFi.scanNetworks();
   int indices[n];
   for (int i = 0; i < n; i++)indices[i] = i;
 
      // RSSI SORT
   for (int i = 0; i < n; i++) {
      for (int j = i + 1; j < n; j++) {
         if (WiFi.RSSI(indices[j]) > WiFi.RSSI(indices[i])) {
            std::swap(indices[i], indices[j]);
         }
      }
   }
     
   for (int i = 0; i < n; ++i) {
      n_ssid.push_back(WiFi.SSID(indices[i]));
      n_rssi.push_back(WiFi.RSSI(indices[i]));
   }
   printNetworks();
}

/**
 * Выдача отладочного сообщения доступных WiFi сетей
 */
void printNetworks(){
   for( int i=0; i< n_ssid.size();i++ ){
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(n_ssid[i]);
      Serial.print(" (");
      Serial.print(n_rssi[i]);
      Serial.println(")");
   }
}

/**
 * Выдача доступных WiFi сете в форму
 * 
 * @param out - текстовый буфер
 * @name name - имя метки HTTP поля
 */
void HTTP_printNetworks(String &out, const char *name){
   int n = n_ssid.size();
   if( n <= 0 ){
      HTTP_printInput(out,"SSID:",name,(const char *)j_config["wifi"]["name"],16,32);
   }
   else {
      out += "  <label>Сеть:</label>\n";
      out += "<select name=\"";
      out += name;
      out += "\">\n";
      for (int i=0; i<n; i++){
         out += "    <option value=\"";
         out += n_ssid[i];
         out += "\"";
 //        if( strcmp(n_ssid[i].c_str(),EA_Config.AP_SSID) == 0 )out+=" selected";
         if( n_ssid[i] == j_config["wifi"]["name"] )out+=" selected";
         out += ">";
         out += n_ssid[i];
         out += " [";
         out += n_rssi[i];
         out += "dB] ";
         out += "</option>\n";
      }     
      out += "</select>\n\n";   
   }

}

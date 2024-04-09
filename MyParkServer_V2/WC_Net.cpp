#include "WC_Net.h"
uint8_t eth_mac[6];
bool isEthernetHW   = false;
bool isEthernetLink = false;

WIFI_STAT_t wifiStatus = WS_WIFI_OFF;
bool is_connectWIFI     = false;
uint32_t msWIFI = 0;

//SPIClass SPI_ETH(HSPI);
//SPIClass *SPI1 = NULL;

void ethernetInit(){
   esp_efuse_mac_get_default(eth_mac);
   Serial.printf("MAC: %02x:%02x:%02x:%02x:%02x:%02x:\n",
       eth_mac[0],eth_mac[1],eth_mac[2],eth_mac[3],eth_mac[4],eth_mac[5]);

   static SPIClass SPI1(HSPI);
//   SPI1 = new SPIClass(HSPI);
Serial.println(1);
   SPI1.begin(ETH_SCLK, ETH_MISO, ETH_MOSI);
   pinMode(ETH_RST, OUTPUT);
   digitalWrite(ETH_RST, HIGH);
   delay(250);
   digitalWrite(ETH_RST, LOW);
   delay(50);
   digitalWrite(ETH_RST, HIGH);
   delay(350);
Serial.println(2);
   Ethernet.setSPI(&SPI1);
Serial.println(3);
   Ethernet.init(ETH_CS);
Serial.println(4);

/*
   Serial.println(1);
   Serial.println(HSPI);
   SPI1 = new SPIClass(HSPI);
   Serial.println(2);
   Serial.println((int)SPI1);
   delay(3000);
   SPI1->begin(ETH_SCLK, ETH_MISO, ETH_MOSI);
   esp_efuse_mac_get_default(eth_mac);
   Serial.printf("MAC: %02x:%02x:%02x:%02x:%02x:%02x:\n",
       eth_mac[0],eth_mac[1],eth_mac[2],eth_mac[3],eth_mac[4],eth_mac[5]);
   pinMode(ETH_RST, OUTPUT);
   digitalWrite(ETH_RST, HIGH);
   delay(250);
   digitalWrite(ETH_RST, LOW);
   delay(50);
   digitalWrite(ETH_RST, HIGH);
   delay(350);
   Serial.println(3);

   Ethernet.init(ETH_CS);
  */ 
}

void ethernetReconnect(){
//  Ethernet.end();
   int n = 0;
   Serial.println("Starting ETH connection...");
//   Serial.println((long)SPI1);   
   IPAddress ip_addr,ip_mask,ip_gate,ip_dns;
   if( j_config["ip"]["type"] == "static" ){
      if( ip_addr.fromString( j_config["ip"]["address"].as<String>() ) &&
          ip_mask.fromString( j_config["ip"]["netmask"].as<String>() ) &&
          ip_gate.fromString( j_config["ip"]["gateway"].as<String>() ) &&
          ip_dns.fromString(  j_config["ip"]["dns"].as<String>()     ) ){
             Serial.print("Config static IP address: ");
             Serial.print(ip_addr);
             Serial.print(" mask: ");
             Serial.print(ip_mask);
             Serial.print(" gate: ");
             Serial.print(ip_gate);
             Serial.print(" dns: ");
             Serial.println(ip_dns);
             n = 1;
             Ethernet.begin(eth_mac, ip_addr, ip_dns, ip_gate, ip_mask);  
      }
      else {
        n = Ethernet.begin(eth_mac);
        if( ip_dns.fromString(  j_config["ip"]["dns"].as<String>() ) )Ethernet.setDnsServerIP(ip_dns);
      }
   }
   else {
      n = Ethernet.begin(eth_mac);
      if( ip_dns.fromString(  j_config["ip"]["dns"].as<String>() ) )Ethernet.setDnsServerIP(ip_dns);
   }
  
   if ( n == 0) {
       Serial.println("Failed to configure Eth using DHCP");
       if (Ethernet.hardwareStatus() == EthernetNoHardware) {
           Serial.println("Eth shield was not found.");
       } else if (Ethernet.linkStatus() == LinkOFF) {
           Serial.println("Eth cable is not connected.");
       }
   } 
   else {
      switch( Ethernet.hardwareStatus() ){
         case EthernetNoHardware : Serial.println("Not Ethernet hardware\n");isEthernetHW = false;break;
         case EthernetW5100      : Serial.println("Ethernet W5100 found\n");isEthernetHW = true;break;
         case EthernetW5200      : Serial.println("Ethernet W5200 found\n");isEthernetHW = true;break;
         case EthernetW5500      : Serial.println("Ethernet W5500 found\n");isEthernetHW = true;break;
         default: Serial.println("Ethernet uncnown found\n");isEthernetHW = true;break;  
      }
      Serial.print("Ethernet IP is: ");
      Serial.println(Ethernet.localIP());
    
   }
   
}

bool ethernetCheck(){
  if( Ethernet.linkStatus() == LinkON )return true;
  else return false;
}

void wifiInit(){
/*  
//   WiFi.mode(WIFI_STA);
   if( j_config["ip"]["type"] == "static" ){
      IPAddress ip_addr,ip_mask,ip_gate,ip_dns;
      if( ip_addr.fromString( j_config["ip"]["address"].as<String>() ) &&
          ip_mask.fromString( j_config["ip"]["netmask"].as<String>() ) &&
          ip_gate.fromString( j_config["ip"]["gateway"].as<String>() ) &&
          ip_dns.fromString(  j_config["ip"]["dns"].as<String>()     ) ){
             Serial.println("Config static IP address");
             WiFi.config(ip_addr, ip_dns, ip_gate, ip_mask);    
      }
   }
*/   
   WiFi.persistent(false);
   WiFi.disconnect(); 
//   WiFi.mode(WIFI_OFF);
//   WiFi.persistent(false);
   is_connectWIFI  = false;
   vTaskDelay(1000);
   msWIFI          = 0;
}

bool wifiCheck(){
   if( isAP )return false;
   wifiRssi = WiFi.RSSI();
//   Serial.printf("Check WiFi %d %d\n",(int)WiFi.status(),(int)is_connectWIFI);
   if( WiFi.status() == WL_CONNECTED ){
//       Serial.println("!!! WiFi check ...");
       if( !is_connectWIFI ){
          wifi_correct = true;
//          displayStatusWiFi("ON",0B00001111);
          Serial.println(F("WiFi: connect ")); 
          Serial.print("IP: ");
          Serial.print(WiFi.localIP());
          Serial.print(" MASK: ");
          Serial.print(WiFi.subnetMask());
          Serial.print(" GW: ");
          Serial.print(WiFi.gatewayIP());
          Serial.print(" DNS: ");
          Serial.println(WiFi.dnsIP());
          is_connectWIFI = true;
          msWIFI = millis();
 
       }
       return true;
   }
   else {
       if( is_connectWIFI ){   
//          displayStatusWiFi("OFF",0B00000001);
          Serial.printf("WiFi: connection lost after %ld sec\n",millis() - msWIFI);  
          is_connectWIFI = false;
          msWIFI         = 0;
       }
       if( msWIFI == 0 ){
          Serial.println(F("WiFi: wait connect ..."));  
//          displayStatusWiFi("...",0B00110011);
          WiFi.persistent(false);
          WiFi.mode(WIFI_STA);
          vTaskDelay(1000);
          if( j_config["ip"]["type"] == "static" ){
              IPAddress ip_addr,ip_mask,ip_gate,ip_dns;
              if( ip_addr.fromString( j_config["ip"]["address"].as<String>() ) &&
                 ip_mask.fromString( j_config["ip"]["netmask"].as<String>() ) &&
                 ip_gate.fromString( j_config["ip"]["gateway"].as<String>() ) &&
                 ip_dns.fromString(  j_config["ip"]["dns"].as<String>()     ) ){
                 Serial.println("Config static IP address");
                 WiFi.config(ip_addr,ip_gate,ip_mask,ip_dns);    
              }
          }


          WiFi.begin(j_config["wifi"]["name"].as<const char *>(),j_config["wifi"]["password"].as<const char *>());
//          WiFi.begin("ASUS_58_2G","sav59vas");
          msWIFI = millis(); 
          
       }
       if(  msWIFI > millis() || ( millis() - msWIFI ) > TM_WIFI_CONNECT  ){  
//          displayStatusWiFi("OFF",0B00000001);
          Serial.println(F("WiFi: not connection. Reset wifi ..."));  
          WiFi.persistent(false);
          WiFi.disconnect(); 
//          WiFi.mode(WIFI_OFF);
          WiFi.persistent(false);
          msWIFI         = 0;
       }
       return false;
   }
   
}


void spiDisable(){
   Serial.println("SPI PIN disable");
   ethernetInit();   
//   pinMode(ETH_RST,INPUT);
   pinMode(ETH_CS,INPUT);
   pinMode(ETH_SCLK,INPUT);
   pinMode(ETH_MISO,INPUT);
   pinMode(ETH_MOSI,INPUT);
//   digitalWrite(ETH_RST,LOW);
//   digitalWrite(ETH_CS,LOW);
//   digitalWrite(ETH_SCLK,LOW);
//   digitalWrite(ETH_MISO,LOW);
//   digitalWrite(ETH_MOSI,LOW);

}

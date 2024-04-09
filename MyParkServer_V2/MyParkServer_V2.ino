#include "WC_Task.h"


void setup() {
//    delay(1000);
    Serial.begin(115200);
    while (!Serial);
    Serial.println("LoRa2Http gateway...");
    char s[13];
    uint8_t mac[6];
    esp_efuse_mac_get_default(mac);
    sprintf(strID,"ESP%02x%02x%02x%02x%02x%02x",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
    Serial.print(" ChipID ");
    Serial.println(strID);  
    Serial.printf("Main loop task ");
    Serial.println(xPortGetCoreID());
    
    tasksStart();  
}



void loop(){
}

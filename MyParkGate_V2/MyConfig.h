#ifndef WC_MYCONFIG_h
#define WC_MYCONFIG_h

//#define CURRENT_FIRMWARE_TITLE    "134-148_245_SVETOFORBOX_RU_INFO"
#define CURRENT_FIRMWARE_TITLE    "228-244_SVETOFORBOX_RU_INFO"

#define CURRENT_FIRMWARE_VERSION  "2.0rc3"
#define CURRENT_CONFIG_VERSION    "0.0.2"
#define DEFAULTS_NAME             "ParkGate_V1"

#define TM_SENSOR_OFFLINE         300000

//#define RESET_CONFIG

// Светодиоды на плате
//#define DISPLAY_LED
// Ethernet шлюз
//#define ETHERNET_W5500
// Сканирование всех нод
//#define SCAN_ALL_NODES
// Максимальное количество парковочных мест на стоянке
// MAX_NUM_PARKING_SPACE+1 - выдает программу "занято все" или "0"
#define MAX_NUM_PARKING_SPACE 87

#define TIMEZONE     5

// Конфигурация на разные сервера TB для облегчения отладки
//#define GATE_SAV
#define GATE_PARK
//#define GATE_ELLIPS

#if defined(GATE_PARK)   
#define WIFI_SSID    "SVETOFORBOXRU_SERVER"
#define WIFI_PASS    "100500600800SVETOFORBOx13"
#elif defined(GATE_SAV)   
#define WIFI_SSID    "ASUS_58_2G"
#define WIFI_PASS    "sav59vas"
#elif defined(GATE_ELLIPS)
#define WIFI_SSID    "ellips"
#define WIFI_PASS    "ell123ips"
#else
#define WIFI_SSID    "none"
#define WIFI_PASS    "none"
#endif

               
#if defined(GATE_SAV)
#define HTTP_HOST   "crm.moscow"
#define HTTP_PORT   8001
#else
#define HTTP_HOST   "crm.moscow"
#define HTTP_PORT   8001
#endif


//#define PIN_BTN       0    
#define PIN_BTN       4    
#define PIN_SDA       40
#define PIN_SCL       42
#define CORE          1
#define PIN_SCL       12
#define CORE          1

#define ETH_RST       16
#define ETH_CS        15
#define ETH_SCLK      18
#define ETH_MISO      47
#define ETH_MOSI      48

#define PIN_IR_SEND   5
#define PIN_NEOPIXEL  6
#define NUM_NEOPIXEL  MAX_NUM_PARKING_SPACE+1

#endif

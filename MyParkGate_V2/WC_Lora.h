#ifndef WC_LORA_h
#define WC_LORA_h
#include <SPI.h>
#include "WC_EEPROM.h"   
#include "WC_Task.h" 
#include "WC_Json.h"
#include "src/SLed.h"
//#include <LoRa.h>           //https://github.com/sandeepmistry/arduino-LoRa set via library mamager 
//#include <heltec.h>         //https://github.com/HelTecAutomation/Heltec_ESP32
#include <LoRaWan_APP.h>


#include <MyLoRaBase.h>
#include <time.h>

extern bool isAP;
extern SLed LedGreen;
extern char _lora_node[];
extern float _lora_vbat, _lora_dist, _lora_rssi;
extern uint32_t _lora_tm;
#if defined(SCAN_ALL_NODES)
extern StaticJsonDocument<JSON_NODES_SIZE> jAllNodes;
extern bool isAllNodesChange;
#endif
#define RF_FREQUENCY                                868000000 // Hz
#define TX_OUTPUT_POWER                             22        // dBm
#define LORA_BANDWIDTH                              0         // [0: 125 kHz,
                                                              //  1: 250 kHz,
                                                              //  2: 500 kHz,
#define LORA_SPREADING_FACTOR                       7         // [SF7..SF12]
#define LORA_CODINGRATE                             1         // [1: 4/5,
#define LORA_PREAMBLE_LENGTH                        8         // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT                         0         // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON                  false
#define LORA_IQ_INVERSION_ON                        false
#define LORA_IQ_INVERSION_OFF                       true


#define MYLORA_DEBUG
#define LORA_NODE   0


typedef enum
{
    STATUS_LOWPOWER,
    STATUS_RX,
    STATUS_TX
}States_t;

void taskLoRa( void *pvParameters );
void initLora();
void sendLora();
bool loopLora(char *_node);
void decodeLora(char *_node, bool _isV3 );
bool checkWhiteList(char *_node);

bool sendJsonToLora( char *_node, bool _isV3 );

void OnTxDone( void );
void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr );
void OnRxTimeout( void );
void OnTxTimeout( void );
void OnRxError( void );



#endif

#ifndef WC_LORA_h
#define WC_LORA_h
#include <SPI.h>
#include <LoRaWan_APP.h>
#include <MyLoRaBase.h>



#define LORA_RF_FREQUENCY     868000000 // Hz

#define LORA_TX_OUTPUT_POWER  22        // dBm

#define LORA_BANDWIDTH         0        // [0: 125 kHz,
                                        //  1: 250 kHz,
                                        //  2: 500 kHz,
                                        //  3: Reserved]
#define LORA_SPREADING_FACTOR   7       // [SF7..SF12]
#define LORA_CODINGRATE         1       // [1: 4/5,
                                        //  2: 4/6,
                                        //  3: 4/7,
                                        //  4: 4/8]
#define LORA_PREAMBLE_LENGTH    8       // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT     0       // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON      false
#define LORA_IQ_INVERSION_ON            false
#define LORA_IQ_INVERSION_OFF           true

extern MyLoRaBaseClass myLora;
extern RadioEvents_t RadioEvents;
void decodeLora();

void initLora();
bool sendLora(uint8_t _attMax);
bool receiveLora( uint32_t _tm );
void printLoraTX();
void printLoraRX();
void OnTxDone( void );
void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr );
void OnRxTimeout( void );
void OnRxError( void );

     


#define MYLORA_DEBUG


#endif

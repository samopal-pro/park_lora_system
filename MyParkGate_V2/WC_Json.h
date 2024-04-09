#ifndef WC_JSON_h
#define WC_JSON_h
#include <ArduinoJson.h>    ////https://github.com/bblanchon/ArduinoJson
#include <time.h>
#include <NTPClient.h>      //https://github.com/arduino-libraries/NTPClient
#include "MyConfig.h"
#include <vector>
extern NTPClient ntpClient;
#define JSON_NODES_SIZE    20000
#define JSON_MSG_SIZE      400
#define Q_MAX_SIZE         100


typedef enum {
    NLS_NONE      = 0,
    NLS_EXIST     = 1,
    NLS_OFFLINE   = 2,
    NLS_OFFLINE1  = 12,
    NLS_FREE      = 3,
    NLS_BUSY      = 4,
    NLS_WIFI_OFF  = 100,
    NLS_WIFI_ON   = 101,
    NLS_WIFI_AP   = 102, 
} NEOPIXEL_LED_STATUS_t;


class TSJsonNodes {
   private:
      JsonArray j_cmd;
//      StaticJsonDocument<JSON_NODES_SIZE> j_nodes;
      StaticJsonDocument<JSON_NODES_SIZE> j_lora;
      char StrNode[16];
   public:
// Статистика
      uint16_t countNodeAll;   
      uint16_t countNodeOnline;   
      uint16_t countNodeFree;   
      uint16_t countNodeFreeSave;   
      uint16_t countNodeBusy;   
      bool isCountNodeFreeChange;

      int currentNodeNum;

      NEOPIXEL_LED_STATUS_t nodesStatus[NUM_NEOPIXEL];
//      int nodesRssi[NUM_NEOPIXEL];
//      uint32_t nodesLastMS[NUM_NEOPIXEL];

      JsonObject j_node;
      JsonObject q_node;
      TSJsonNodes();
      std::vector <String> s_tel;
      std::vector <String> s_att;
      char *strNode(uint16_t _node); 
      void setCurrentNode(char *_node);
      void updateNode(uint32_t _ttl);
      void updateNode(uint32_t _ttl, int _rssi, JsonObject _params);
      void setAttribute( char *_name, char *_val );
      void setTelemetry( char *_name, float _value );
      void addAttributeToLora( char *_name, JsonVariant _val );
      void addRpcToLora( char *_cmd, int _id );
//      char *getCmd();
//      void pushCmd();
      void debug();
      void parseNodes();
      bool StatusAll();
      bool SetStatusNode(NEOPIXEL_LED_STATUS_t _stat, int _rssi = -999);
      bool checkClassC(char *_node);
      void addRequest(uint32_t _id, char *_name);
//      void calcNodes();

};

 

class TSJsonHTTP {
   private:
   public:
      JsonObject j_node;
      StaticJsonDocument<JSON_MSG_SIZE> j_msg;
      std::vector <String> q_msg;
      std::vector <String> q_ip;
      TSJsonHTTP(void);
      void setParams(uint16_t cNodeAll,uint16_t cNodeOnline,uint16_t cNodeFree,uint16_t cNodeBusy);
      void setStatisticAll(uint16_t cNodeAll,uint16_t cNodeOnline,uint16_t cNodeFree,uint16_t cNodeBusy);
      void setStatistic(char *_server,uint16_t cNodeAll,uint16_t cNodeOnline,uint16_t cNodeFree,uint16_t cNodeBusy);
//      void setStatus(char *_node, int _dist, uint32_t _uptime);
      uint16_t keyGen(char *_node, int _dist, uint32_t _tm );
      uint16_t keyGen(char *_id,uint16_t cNodeAll,uint16_t cNodeOnline,uint16_t cNodeFree,uint16_t cNodeBusy);
//      void initTelemetry(uint16_t _node);
//      void initTelemetry(char *_name);
//      void addTelemetry(char *_name, float _value);
//      void setMyTelemetry(char *_name, float _value);
//      void setMyAttribute(char *_name, char *_value);
      void pop(const char *_server,const char *_str);
};

extern TSJsonNodes JNodes;
extern TSJsonHTTP  JHttp;


#endif

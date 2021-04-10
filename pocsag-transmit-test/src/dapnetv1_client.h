#if !defined(_DAPNETV1_CLIENT_H)
#define _DAPNETV1_CLIENT_H
#include <WiFiClient.h>
#include <ezTime.h>

#define DAPNET_QUEUE_MAX 200

#define DAPNET_DEBUG
#define DAPNET_DEBUG_PORT   Serial
#if defined(DAPNET_DEBUG)
  #define DAPNET_DEBUG_PRINT(...) { DAPNET_DEBUG_PORT.print(__VA_ARGS__); }
  #define DAPNET_DEBUG_PRINTLN(...) { DAPNET_DEBUG_PORT.println(__VA_ARGS__); }
#else
  #define DAPNET_DEBUG_PRINT(...) {}
  #define DAPNET_DEBUG_PRINTLN(...) {}
#endif


/*!
* DAPNET v1 Protocol Parser
- catSIXe, encoder is partly transformed from RPITX, with my old contributions(batch-encoding etc.) from the old github
*/
class DAPNETV1Client {
  public:
    WiFiClient* _client;
    bool timeSlot[16];
    explicit DAPNETV1Client(WiFiClient* client);
    void begin();
    void test();
    void loop();
  private:
    uint8_t lastMsgAck;
    void sendAck();
};
uint8_t nibble2c(char c);
uint8_t hex2c(char c1, char c2);

typedef struct {
  uint32_t ric;
  uint8_t function;
  uint8_t length;
  char message[80];
} DAPNETMessage;


#endif
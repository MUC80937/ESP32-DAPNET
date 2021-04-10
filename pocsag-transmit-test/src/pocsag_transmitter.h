#if !defined(_POCSAG_TRANSMITTER_H)
#define _POCSAG_TRANSMITTER_H
#include <RadioLib.h>

#define POCSAG_MAX_ENCODING_BUFFER_SIZE 70 // 70 QBYTEs should be enough for max 80char Messages
#define POCSAG_MAX_BUFFER_SIZE 2000 // 2000 QBYTEs
#define POCSAG_PIN_DIO1 33 // DCLK
#define POCSAG_PIN_DIO2 32 // DATA

// #define POCSAG_DEBUG
#define POCSAG_DEBUG_PORT   Serial
#if defined(POCSAG_DEBUG)
  #define POCSAG_DEBUG_PRINT(...) { POCSAG_DEBUG_PORT.print(__VA_ARGS__); }
  #define POCSAG_DEBUG_PRINTLN(...) { POCSAG_DEBUG_PORT.println(__VA_ARGS__); }
#else
  #define POCSAG_DEBUG_PRINT(...) {}
  #define POCSAG_DEBUG_PRINTLN(...) {}
#endif

/*!
* POCSAG Transmitter
- catSIXe, encoder is partly transformed from RPITX, with my old contributions(batch-encoding etc.) from the old github
*/

static SX1278* _currentRadio; // we need to put this pointer static for the ISR
static uint32_t transmitterData[ POCSAG_MAX_BUFFER_SIZE ]; // complete transmission
static uint32_t encodingBuffer[ POCSAG_MAX_ENCODING_BUFFER_SIZE ]; // small static allocated memory, for storing a encoded chunk

static uint16_t transmitterLength;
static uint16_t transmitterOffset;
static uint8_t transmitterBitOffset;
static bool transmitterState;
static void IRAM_ATTR onTransmitterClock();

class POCSAGTransmitter {
    public:
        SX1278* _radio;
        explicit POCSAGTransmitter();
        void begin(SX1278 *radio);
        void clearQueue();
        int8_t getQueueCount();
        bool isActive();
        void queuePage(uint32_t ric, uint8_t functionBit, char* text);
        bool transmitBatch();
        void test();
    private:
        int8_t queueCount = 0;
};

#endif
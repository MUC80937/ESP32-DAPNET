#if !defined(_POCSAG_TRANSMITTER_H)
#define _POCSAG_TRANSMITTER_H
#include <RadioLib.h>
// #include <SX1278.h>

/*!
* POCSAG Transmitter
- catSIXe
*/
class POCSAGTransmitter {
    public:
        uint32_t *transmitterData;
        uint32_t *encodingBuffer;
        explicit POCSAGTransmitter();
        void begin(SX1278 *radio);
        void queuePage(uint32_t ric, char* text);
        void transmitBatch();
        void test();
        void writeFIFOChunk(uint8_t* data, size_t len);
    SX1278* _radio;
};

#endif
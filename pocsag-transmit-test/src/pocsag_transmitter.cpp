#include "pocsag_transmitter.h"
#include "pocsag_encoder.h"


POCSAGTransmitter::POCSAGTransmitter() {
    POCSAG_DEBUG_PRINTLN("malloc'ating and prefilling encodingBuffer");
    for (int i=0;i<POCSAG_MAX_BUFFER_SIZE;i++) transmitterData[i] = 0xFFFFFFFF;
    for (int i=0;i<POCSAG_MAX_ENCODING_BUFFER_SIZE;i++) encodingBuffer[i] = 0xFFFFFFFF;
}

void POCSAGTransmitter::begin(SX1278 *radio) {
    _radio = radio;
    _currentRadio = _radio;

    _radio->setFrequencyDeviation(4.5);
    // 512 baud testing
    //uint16_t bitRate = 62500; //62500; //(SX127X_CRYSTAL_FREQ * 1000.0) / 0.512;
    //_radio->_mod->SPIsetRegValue(SX127X_REG_BITRATE_MSB, (bitRate & 0xFF00) >> 8, 7, 0);
    //_radio->_mod->SPIsetRegValue(SX127X_REG_BITRATE_LSB, bitRate & 0x00FF, 7, 0);

    _radio->setCRC(false);
    _radio->setEncoding(RADIOLIB_ENCODING_NRZ);
    _radio->setDataShaping(RADIOLIB_SHAPING_NONE);
    _radio->setPreambleLength(0);
    _radio->setBitRate(1.2);    
    // pinz
    pinMode(POCSAG_PIN_DIO1, INPUT);
    pinMode(POCSAG_PIN_DIO2, OUTPUT);
}

void test2();

void POCSAGTransmitter::clearQueue() {
    queueCount = 0;
    transmitterLength = 0;
}
int8_t POCSAGTransmitter::getQueueCount() {
    return queueCount;
}
bool POCSAGTransmitter::isActive() {
    return transmitterState;
}
void POCSAGTransmitter::queuePage(uint32_t ric, uint8_t functionBit, char *text) {
    size_t beforeLength = transmitterLength + 0;
    size_t messageLength = textMessageLength(queueCount, ric, strlen(text));
    //uint32_t *transmission = (uint32_t *)malloc(sizeof(uint32_t) * messageLength);
    uint32_t *encodingBufferPointer = encodingBuffer;
    encodeTransmission(queueCount, ric, functionBit, text, encodingBufferPointer);
    transmitterLength += messageLength;

    #ifdef POCSAG_DEBUG
    POCSAG_DEBUG_PRINTLN();
    POCSAG_DEBUG_PRINT("RIC: "); POCSAG_DEBUG_PRINT(ric, DEC);
    POCSAG_DEBUG_PRINT(" F: "); POCSAG_DEBUG_PRINTLN(functionBit, DEC);
    POCSAG_DEBUG_PRINT("TEXT: "); POCSAG_DEBUG_PRINTLN(text);
    POCSAG_DEBUG_PRINTLN();
    POCSAG_DEBUG_PRINT("messageLength="); POCSAG_DEBUG_PRINTLN(messageLength, DEC);
    POCSAG_DEBUG_PRINTLN("MESSAGE encoded:");
    for (size_t i = 0; i < messageLength; i++)
        POCSAG_DEBUG_PRINT(encodingBuffer[i], HEX);
    POCSAG_DEBUG_PRINTLN();
    #endif

    // copy it to the correct place lol
    for (size_t i = 0; i < messageLength; i++) {
        transmitterData[ beforeLength + i ] = encodingBuffer[ i ];
    }
    //free(transmission);
    queueCount++;
}
bool POCSAGTransmitter::transmitBatch() {
    if (transmitterState > 0) return false; // we already started
    if (transmitterLength < 16) return false; // dont start if we have less than 16 bytes

    _radio->standby();
    #ifdef POCSAG_DEBUG
    POCSAG_DEBUG_PRINT(F("transmitterData: ")); POCSAG_DEBUG_PRINTLN ((unsigned long int)transmitterData, HEX);

    POCSAG_DEBUG_PRINT(F("transmitterData: ")); POCSAG_DEBUG_PRINTLN((unsigned long int)transmitterData, HEX);
    POCSAG_DEBUG_PRINTLN();
    POCSAG_DEBUG_PRINTLN("TX encoded in big buffer:");
    for (size_t i = 0; i < transmitterLength; i++)
        POCSAG_DEBUG_PRINT(transmitterData[i], HEX);
    POCSAG_DEBUG_PRINTLN();
    POCSAG_DEBUG_PRINT("transmitterLength="); POCSAG_DEBUG_PRINTLN(transmitterLength, DEC);

    POCSAG_DEBUG_PRINT(F("transmitterData: ")); POCSAG_DEBUG_PRINTLN((unsigned long int)transmitterData, HEX);

    POCSAG_DEBUG_PRINTLN("starting tx");
    #endif
    transmitterState = true;
    transmitterBitOffset = 32;
    transmitterOffset = 0;
    // attach DCLK interrupt
    attachInterrupt(digitalPinToInterrupt(POCSAG_PIN_DIO1), onTransmitterClock, RISING);
    delay(10); //giving it some time
    _currentRadio->transmitDirect();
    return true;
}

static void IRAM_ATTR onTransmitterClock() {
    transmitterBitOffset--;
    digitalWrite(POCSAG_PIN_DIO2, ((transmitterData[ transmitterOffset ] >> transmitterBitOffset) & 1) == 0);
    if (transmitterBitOffset == 0) { // lets shift to next byte
        transmitterBitOffset = 32;
        transmitterOffset++;
    }
    if (transmitterOffset > transmitterLength) {
        _currentRadio->standby();
        detachInterrupt(POCSAG_PIN_DIO1);
        transmitterState = false;
    }
}
#include "pocsag_transmitter.h"
#include "pocsag_encoder.h"


#define POCSAG_MAX_BUFFER_SIZE 2000 // 2000 QBYTEs



POCSAGTransmitter::POCSAGTransmitter() {
    Serial.println("malloc");
    transmitterData = (uint32_t *)malloc(sizeof(uint32_t) * POCSAG_MAX_BUFFER_SIZE); //TODO: make this static allocated
    for (int z=0;z<POCSAG_MAX_BUFFER_SIZE;z++) transmitterData[z] = 0xFFFFFFFF;
    encodingBuffer = (uint32_t *)malloc(sizeof(uint32_t) * POCSAG_MAX_BUFFER_SIZE); //TODO: make this static allocated
    for (int z=0;z<POCSAG_MAX_BUFFER_SIZE;z++) encodingBuffer[z] = 0xFFFFFFFF;
    
}

void POCSAGTransmitter::begin(SX1278 *radio) {
    _radio = radio;
    _radio->setFrequencyDeviation(4.5);
    /*_radio->_mod->SPIsetRegValue(SX127X_REG_FDEV_MSB, 0x00);
    _radio->_mod->SPIsetRegValue(SX127X_REG_FDEV_LSB, 75);*/
    // _radio->setEncoding(RADIOLIB_ENCODING_NRZ);
    _radio->_mod->SPIsetRegValue(SX127X_REG_PAYLOAD_LENGTH, 0x00);
    _radio->_mod->SPIsetRegValue(SX127X_REG_FIFO_THRESH, SX127X_TX_START_FIFO_NOT_EMPTY | 0x0F);
    // _radio->pubmod->SPIsetRegValue(SX127X_REG_FIFO_THRESH,  SX127X_TX_START_FIFO_NOT_EMPTY | SX127X_FIFO_THRESH);
    //_radio->pubmod->SPIsetRegValue(SX127X_REG_PREAMBLE_LSB, 0x00);
    //_radio->setPreambleLength(0);
    uint16_t bitRate = 62500; //62500; //(SX127X_CRYSTAL_FREQ * 1000.0) / 0.512;
    _radio->_mod->SPIsetRegValue(SX127X_REG_BITRATE_MSB, (bitRate & 0xFF00) >> 8, 7, 0);
    _radio->_mod->SPIsetRegValue(SX127X_REG_BITRATE_LSB, bitRate & 0x00FF, 7, 0);

    _radio->_mod->SPIsetRegValue(SX127X_REG_PA_RAMP, SX1278_NO_SHAPING);
    _radio->_mod->SPIsetRegValue(SX127X_REG_PACKET_CONFIG_1, SX127X_PACKET_FIXED | SX127X_DC_FREE_NONE | SX127X_CRC_OFF | SX127X_CRC_AUTOCLEAR_ON | SX127X_ADDRESS_FILTERING_OFF, 7, 0);
    _radio->_mod->SPIsetRegValue(SX127X_REG_PACKET_CONFIG_2, SX127X_DATA_MODE_PACKET, 6, 6);
    _radio->setCRC(false);
    _radio->setPreambleLength(0);
}
void POCSAGTransmitter::test() {
    _radio->standby();
    _radio->_mod->SPIsetRegValue(SX127X_REG_DIO_MAPPING_1, SX127X_DIO0_PACK_PACKET_SENT);

    size_t completeLength = 0;

    uint8_t * transmitterByteDataPointer = (uint8_t *)transmitterData;
    
    Serial.print(F("transmitterData: ")); Serial.println ((unsigned long int)transmitterData, HEX);
    Serial.print(F("transmitterByteDataPointer: ")); Serial.println ((unsigned long int)transmitterByteDataPointer, HEX);
    /*
    for (int y = 0; y < 13; y++) {
        size_t beforeLength = completeLength + 0;
        size_t messageLength = textMessageLength(y, 133701, 80);
        uint32_t *transmission = (uint32_t *)malloc(sizeof(uint32_t) * messageLength);
        encodeTransmission(y, 133701 + y, 3, "01234567890123456789012345678901234567890123456789012345678901234567890123456789", transmission);
        completeLength += messageLength;
        Serial.println();
        Serial.print("messageLength="); Serial.println(messageLength, DEC);
        Serial.println("MESSAGE encoded:");
        for (size_t i = 0; i < messageLength; i++)
            Serial.print(transmission[i], HEX);
        Serial.println();

        for (size_t i = 0; i < messageLength; i++) {
            size_t baseOffset = (beforeLength * sizeof(uint32_t)) + (i * sizeof(uint32_t));
            transmitterByteDataPointer[ baseOffset + 00 ] = ((transmission[ i ] >> 24) & 0xFF);
            transmitterByteDataPointer[ baseOffset + 01 ] = ((transmission[ i ] >> 16) & 0xFF);
            transmitterByteDataPointer[ baseOffset + 02 ] = ((transmission[ i ] >> 8)  & 0xFF);
            transmitterByteDataPointer[ baseOffset + 03 ] =  (transmission[ i ]        & 0xFF);
        }
        free(transmission);
        /*Serial.println("TX encoded after free:");
        for (size_t i = 0; i < messageLength; i++)
            Serial.print(transmission[i], HEX);/
    }
    */
    uint32_t testTransmission[ 18 + 13 ] = {
        // 18 words, 576 bits of preamble
		0xAAAAAAAA, 0xAAAAAAAA,
		0xAAAAAAAA, 0xAAAAAAAA, 0xAAAAAAAA, 0xAAAAAAAA, 0xAAAAAAAA, 0xAAAAAAAA, 0xAAAAAAAA, 0xAAAAAAAA,
		0xAAAAAAAA, 0xAAAAAAAA, 0xAAAAAAAA, 0xAAAAAAAA, 0xAAAAAAAA, 0xAAAAAAAA, 0xAAAAAAAA, 0xAAAAAAAA,
		// The real data starts here
		0x7CD215D8, 0x7A89C197, 0x7A89C197, 0x7A89C197, 0x7A89C197, 0x7A89C197, 0x7A89C197, 0x7A89C197,
		0x7A89C197, 0x4F5A0109, 0xC2619CE1, 0x7A89C197, 0x7A89C197,
    };
    completeLength = 18+13;
    for (size_t i = 0; i < completeLength; i++) {
        size_t baseOffset = (0 * sizeof(uint32_t)) + (i * sizeof(uint32_t));
        transmitterByteDataPointer[ baseOffset + 00 ] = ((testTransmission[ i ] >> 24) & 0xFF);
        transmitterByteDataPointer[ baseOffset + 01 ] = ((testTransmission[ i ] >> 16) & 0xFF);
        transmitterByteDataPointer[ baseOffset + 02 ] = ((testTransmission[ i ] >> 8)  & 0xFF);
        transmitterByteDataPointer[ baseOffset + 03 ] =  (testTransmission[ i ]        & 0xFF);
    }
    Serial.print(F("transmitterData: ")); Serial.println ((unsigned long int)transmitterData, HEX);
    Serial.print(F("transmitterByteDataPointer: ")); Serial.println ((unsigned long int)transmitterByteDataPointer, HEX);

    Serial.println();
    Serial.println("TX encoded in big buffer:");
    for (size_t i = 0; i < completeLength*4; i++)
        Serial.print(transmitterByteDataPointer[i], HEX);
    Serial.println();
    Serial.print("completeLength="); Serial.println(completeLength, DEC);

    Serial.print(F("transmitterData: ")); Serial.println ((unsigned long int)transmitterData, HEX);
    Serial.print(F("transmitterByteDataPointer: ")); Serial.println ((unsigned long int)transmitterByteDataPointer, HEX);

    uint8_t * transmissionChunk = (uint8_t *)malloc(16);
    for (int i=0;i<16;i++) transmissionChunk[i] = i%2==0?0x55 : 0xAA; // setting it all to 0xFF to see where memcpy works
    // do encoding

    byte irq2 = 0x00;
    //memcpy(transmissionChunk, transmitterByteDataPointer, 16);
    writeFIFOChunk(transmitterByteDataPointer, 16);
    transmitterByteDataPointer+=16;
    /*for (int i=0;i<16;i++) {
        transmissionChunk[ i ] = *transmitterByteDataPointer;
        transmitterByteDataPointer++;
    }*/

    //writeFIFOChunk(transmissionChunk, 16);
    Serial.println("starting tx");
    _radio->setMode(SX127X_TX);
    /*for (int i=4; i < completeLength; i+=4) {
        //memcpy(transmissionChunk, transmitterByteDataPointer, 16);
        transmitterByteDataPointer+=16;
        writeFIFOChunk(transmissionChunk, 16);
        delay(20);
        do {
            irq2 = _radio->_mod->SPIgetRegValue(SX127X_REG_IRQ_FLAGS_2);
            Serial.print(irq2 >> 4 & 1, HEX);
        } while ((irq2 >> 4 & 1) > 0);
    }*/
    //Serial.println("free'd transmissionChunk, waiting for PacketSent");
    do {
        irq2 = _radio->_mod->SPIgetRegValue(SX127X_REG_IRQ_FLAGS_2);
        // Serial.print(irq2 >> 3 & 1, HEX);
    } while (((irq2 >> 3) & 1) == 0);
    _radio->standby();
    free(transmissionChunk);
    Serial.println("done");

    Serial.print(F("transmitterData: ")); Serial.println ((unsigned long int)transmitterData, HEX);
    Serial.print(F("transmitterByteDataPointer: ")); Serial.println ((unsigned long int)transmitterByteDataPointer, HEX);

}

void POCSAGTransmitter::writeFIFOChunk(uint8_t* data, size_t len) {
    for (int z=0;z<len;z++) Serial.print(data[ z ], HEX);
    Serial.println();
    _radio->_mod->SPItransfer(0x80, 0x00, data, NULL, len);
}
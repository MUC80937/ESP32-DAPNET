#include "dapnetv1_client.h"

char _dapnetRICSubstring[10];
uint16_t dapnetQueueLength;
static DAPNETMessage dapnetQueue[DAPNET_QUEUE_MAX];
DAPNETV1Client::DAPNETV1Client(WiFiClient* client) {
    _client = client;
    lastMsgAck = 0x00;
    dapnetQueueLength = 0;
    for (int i=0;i<16;i++) timeSlot[i] = false;
}

void DAPNETV1Client::begin() {
    if (_client->connect("10.13.0.180", 43434)) {
        Serial.println("Connected to DAPNET");
        _client->println("[ESP32LoraPager v1.0.0 xx1337 123]");
    }
    
}
void DAPNETV1Client::sendAck() {
    _client->println(F("+"));
    _client->flush();
}
void DAPNETV1Client::loop() {
    if (_client->connected()) {
        if (_client->available()) {
            String line = _client->readStringUntil('\n');
            DAPNET_DEBUG_PRINT(F("DAPNET>")); DAPNET_DEBUG_PRINTLN(line);
            DAPNET_DEBUG_PRINTLN(line[0], HEX);
            switch (line[0]) {
                case 0x32: // "timesync"
                    DAPNET_DEBUG_PRINTLN("timesync");
                    DAPNET_DEBUG_PRINTLN(String("      <") + line + F(":0000"));
                    _client->println(line + F(":0000"));
                    sendAck();
                break;
                case 0x33:// "timeset"
                    sendAck();
                break;
                case 0x34: // "timeslot"
                    for (int i = 2; i < line.length(); i++) {
                        uint8_t timeSlotIndex = line.charAt(i);
                        // very rudimentary single hex char to number parser
                        timeSlotIndex = timeSlotIndex > 57 ? timeSlotIndex - 55 : timeSlotIndex - 48;
                        timeSlot[ timeSlotIndex ] = true;
                    }
                    #ifdef DAPNET_DEBUG
                        DAPNET_DEBUG_PRINT("TimeslotConfig: "); for (int i=0;i<16;i++) DAPNET_DEBUG_PRINT(timeSlot[i] ? 49 : 48, 0); DAPNET_DEBUG_PRINTLN();
                    #endif
                    sendAck();
                break;
                case 0x23: // Message
                    if (line[4] == 0x36 && line[6] == 0x31) { // type msg &speed is 1200baud
                        //DAPNET_DEBUG_PRINTLN(F("going to indexOf")); 
                        uint8_t endOfRICLoc = line.indexOf(0x3A, 8);
                        uint8_t beforeStringLoc = line.lastIndexOf(0x3A) + 1;
                        uint8_t lineLength = line.length();
                        dapnetQueue[ dapnetQueueLength ].length = lineLength - beforeStringLoc;

                        //DAPNET_DEBUG_PRINT(F("endOfRICLoc:")); DAPNET_DEBUG_PRINTLN(endOfRICLoc);
                        for (int j = 0; j < 10; j++) _dapnetRICSubstring[j] = j + 8 < endOfRICLoc ? line[ j + 8 ] : 0x00;
                        //DAPNET_DEBUG_PRINT(F("_dapnetRICSubstring:")); DAPNET_DEBUG_PRINTLN(_dapnetRICSubstring);
                        dapnetQueue[ dapnetQueueLength ].ric = strtol(_dapnetRICSubstring, 0, 16);
                        dapnetQueue[ dapnetQueueLength ].function = line[ endOfRICLoc + 1 ] - 0x30;
                        for (int j = 0; j < 80; j++) dapnetQueue[ dapnetQueueLength ].message[ j ] = 0x00;
                        for (int j = 0; j < dapnetQueue[ dapnetQueueLength ].length; j++) dapnetQueue[ dapnetQueueLength ].message[ j ] = line[ beforeStringLoc + j ];
                        
                        DAPNET_DEBUG_PRINT(F("RIC:")); DAPNET_DEBUG_PRINTLN(dapnetQueue[ dapnetQueueLength ].ric);
                        DAPNET_DEBUG_PRINT(F("FNC:")); DAPNET_DEBUG_PRINTLN(dapnetQueue[ dapnetQueueLength ].function);
                        DAPNET_DEBUG_PRINT(F("LEN:")); DAPNET_DEBUG_PRINTLN(dapnetQueue[ dapnetQueueLength ].length);
                        DAPNET_DEBUG_PRINT(F("TXT:")); DAPNET_DEBUG_PRINTLN(dapnetQueue[ dapnetQueueLength ].message);

                        dapnetQueueLength++;
                    }
                    //msgAck
                    _client->print(0x23, 0);
                    DAPNET_DEBUG_PRINT(0x23, 0);
                    lastMsgAck = hex2c(line[1], line[2]);
                    lastMsgAck++;
                    if (lastMsgAck < 0x10) {
                        _client->print(0x30, 0);
                        DAPNET_DEBUG_PRINT(0x30, 0);
                    }
                    _client->print(lastMsgAck, HEX);
                    DAPNET_DEBUG_PRINT(lastMsgAck, HEX);
                    _client->print(0x20, 0);
                    DAPNET_DEBUG_PRINT(0x20, 0);
                    _client->println(0x2B, 0);
                    DAPNET_DEBUG_PRINTLN(0x2B, 0);
                    _client->flush();
                break;
            }
        }
    }
}

uint8_t nibble2c(char c) {
    if ((c>='0') && (c<='9'))
        return c-'0';
    if ((c>='A') && (c<='F'))
        return c+10-'A';
    if ((c>='a') && (c<='a'))
        return c+10-'a';
    return -1;
}
uint8_t hex2c(char c1, char c2) {
    if(nibble2c(c2) >= 0)
        return nibble2c(c1) * 16 + nibble2c(c2);
    return nibble2c(c1);
}
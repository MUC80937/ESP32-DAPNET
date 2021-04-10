#include <Arduino.h>
#define RADIOLIB_LOW_LEVEL
#define RADIOLIB_GODMODE

#include <RadioLib.h>
#include <pocsag_transmitter.h>
//Better
#define LORA_SCK        5
#define LORA_MISO       19
#define LORA_MOSI       27
#define LORA_SS         18
#define LORA_DIO0       26
#define LORA_DIO1       33
#define LORA_DIO2       32
#define LORA_RST        23

SX1276 radio = new Module(LORA_SS, LORA_DIO0, LORA_RST, LORA_DIO1);
POCSAGTransmitter transmitter;

void setup() {
  Serial.begin(115200);
  int state = radio.beginFSK(868.23,1.2,4.5);
  if (state == ERR_NONE) {
    Serial.println(F("success!"));
  } else {
    Serial.print(F("failed, code "));
    Serial.println(state);
    while (true);
  }
  Serial.println("initialized modem");
  transmitter.begin(&radio);
  Serial.println("initialized transmitter");
  delay(2e2);
  transmitter.queuePage(133701, 3, "Testbericht 0"); // should never be sent
  transmitter.clearQueue();
  transmitter.queuePage(133701, 3, "Testbericht 1");
  transmitter.queuePage(133703, 3, "Testbericht -2");
  transmitter.queuePage(133706, 3, "Testbericht --3");
  transmitter.queuePage(133707, 3, "Testbericht ---4");
  transmitter.transmitBatch();
}

void loop() {
  //Serial.print(".");
  delay(50);
  // put your main code here, to run repeatedly:
}
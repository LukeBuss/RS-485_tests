// src/slave1/main.cpp
#include <Arduino.h>
#include <RS485ModbusRTU.h>
#include "config.h"
#include "pinmap.h"
#include "Slave1Functions.h"

#ifdef TARGET_NANO
  RS485ModbusRTU bus(DE_RE_PIN);
#elif TARGET_NRF52840
  RS485ModbusRTU bus(Serial1, DE_RE_PIN);
  #include <Adafruit_TinyUSB.h>
#endif

const uint8_t SLAVE_ID = 0x01;

void setup() {
  Serial.begin(SERIAL_SPEED);
#ifdef TARGET_NRF52840
  // while (!Serial) { delay(10); } // let USB enumerate
#endif

#ifdef TARGET_NRF52840
  Serial1.begin(RS485_BAUD); // For nRF52840, Serial1 is used
  while (!Serial1) { delay(10); } // let USB enumerate
#endif

  bus.begin(RS485_BAUD);
  bus.setDebug(&Serial);
  bus.enableDebug(true);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_GREEN, LOW); // Start with LED on

  Serial.println("Slave 1 ready: waiting for ADD commands over RS485 Modbus RTU");
}

void loop() {
  uint8_t buffer[64];
  size_t len = bus.receiveResponse(buffer, sizeof(buffer));

  // if (len >= 3 && buffer[0] == SLAVE_ID && buffer[1] == 0x03) {
  //   handleFunction03_Add(bus, buffer, len);
  //   //Serial.println();
  // }
  if (len >= 2 && buffer[0] == SLAVE_ID && buffer[1] == 0x04) {
    handleFunction04_Location(bus, buffer, len);
  }
}

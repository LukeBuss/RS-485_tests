// src/slave1/main.cpp
#include <Arduino.h>
#include <RS485ModbusRTU.h>
#include "config.h"
#include "pinmap.h"
#include "Slave1Functions.h"

#ifdef TARGET_NANO
RS485ModbusRTU bus(DE_RE_PIN);
#else
RS485ModbusRTU bus(Serial2, DE_RE_PIN);
#endif

const uint8_t SLAVE_ID = 0x01;

void setup() {
  Serial.begin(SERIAL_SPEED);
  bus.begin();
  bus.setDebug(&Serial);

  Serial.println("Slave 1 ready: waiting for ADD commands over RS485 Modbus RTU");
}

void loop() {
  uint8_t buffer[64];
  size_t len = bus.receiveResponse(buffer, sizeof(buffer));

  if (len >= 3 && buffer[0] == SLAVE_ID && buffer[1] == 0x03) {
    handleFunction03_Add(bus, buffer, len);
      Serial.println();
  }
}

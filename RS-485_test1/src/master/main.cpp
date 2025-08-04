// src/master/main.cpp
#include <Arduino.h>
#include <RS485ModbusRTU.h>
#include "config.h"
#include "pinmap.h"
#include "MasterFunctions.h"

#ifdef TARGET_NANO
RS485ModbusRTU bus(RS485_RX, RS485_TX, DE_RE_PIN);
#else
RS485ModbusRTU bus(Serial2, DE_RE_PIN);
#endif

unsigned long lastSendTime = 0;

void setup() {
  Serial.begin(SERIAL_SPEED);
  bus.begin();
  bus.setDebug(&Serial);

  Serial.println("Master ready: sending ADD commands over RS485 Modbus RTU");
}

void loop() {
  if (millis() - lastSendTime > 1000) {
    lastSendTime = millis();
    uint8_t values[] = { 4, 5, 6 }; // expected sum = 15
    sendAddCommand(bus, 0x01, values, sizeof(values));
  }

  uint8_t response[16];
  size_t len = bus.receiveResponse(response, sizeof(response));

  if (len > 0) {
    if (len >= 5 && response[0] == 0x01 && response[1] == 0x03 && response[2] == 0x02) {
      uint16_t sum = (response[3] << 8) | response[4];
      Serial.print("Received sum from slave: ");
      Serial.println(sum);
    } else {
      Serial.println("Invalid response: ");
      for (size_t i = 0; i < len; i++) {
        Serial.print("0x");
        Serial.print(response[i], HEX);
        Serial.print(" ");
      }
    }
    Serial.println();
  }

  delay(100);
}
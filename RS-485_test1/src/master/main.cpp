// src/master/main.cpp
#include <Arduino.h>
#include <RS485ModbusRTU.h>
#include "config.h"
#include "pinmap.h"
#include "MasterFunctions.h"

#ifdef TARGET_NANO
RS485ModbusRTU bus(DE_RE_PIN);
#else
RS485ModbusRTU bus(Serial2, DE_RE_PIN);
#endif

unsigned long lastSendTime = 0;
unsigned long start = micros();
unsigned long totalTime = micros();

void setup() {
  Serial.begin(SERIAL_SPEED);
  bus.begin();
  bus.setDebug(&Serial);
  bus.enableDebug(false);

  Serial.println("Master ready: sending ADD commands over RS485 Modbus RTU");
}

void loop() {
  if (millis() - lastSendTime > 1000) {
    lastSendTime = millis();

    // Step timing variables
    unsigned long t1 = micros();
    
    uint8_t values[] = { 4, 5, 6 }; // expected sum = 15
    sendAddCommand(bus, 0x01, values, sizeof(values));

    unsigned long t2 = micros();  // after send

    uint8_t response[16];
    size_t len = bus.receiveResponse(response, sizeof(response));

    unsigned long t3 = micros();  // after receive

    if (len > 0) {
      if (len >= 5 && response[0] == 0x01 && response[1] == 0x03 && response[2] == 0x02) {
        uint16_t sum = (response[3] << 8) | response[4];

        Serial.print("Received sum from slave: ");
        Serial.println(sum);
        Serial.print("Send time: ");
        Serial.print(t2 - t1);
        Serial.println(" µs");
        Serial.print("Receive time: ");
        Serial.print(t3 - t2);
        Serial.println(" µs");
        Serial.print("Total time: ");
        Serial.print(t3 - t1);
        Serial.println(" µs");
      } else {
        Serial.println("Invalid response:");
        bus.printBytes(response, len);
      }
    } else {
      Serial.println("No response or timeout");
    }

    Serial.println();
  }

  delay(1);
}

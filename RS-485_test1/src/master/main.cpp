// src/master/main.cpp
#include <Arduino.h>
#include "config.h"
#include "pinmap.h"
#include "RS485ModbusRTU.h"

#if defined(TARGET_NANO)
  RS485ModbusRTU bus(DE_RE_PIN);
#else
  RS485ModbusRTU bus(RS485_PORT, DE_RE_PIN);
#endif

static unsigned long lastPing = 0;

 void sendLocationRequest(uint8_t slave) {
  uint8_t frame[2] = { slave, 0x04 }; // function 0x04: request location
  bus.sendRequest(frame, sizeof(frame));
}

void setup() {
  Serial.begin(115200);
  delay(50);

#if defined(ARDUINO_ARCH_ESP32) || defined(TARGET_ESP32)
  // Optional: set pins here if not default, e.g.:
  // RS485_PORT.begin(SERIAL_SPEED, SERIAL_8N1, RX_PIN, TX_PIN);
#endif

  bus.begin(SERIAL_SPEED);
#if RS485_DEBUG
  bus.setDebug(&Serial);
  bus.enableDebug(true);
#endif

#if defined(ARDUINO_ARCH_ESP32) || defined(TARGET_ESP32)
  // Optional: enable hardware RS485 so RTS drives DE automatically
  // bus.enableHardwareRS485(RS485_RTS);
#endif
}

void loop() {
  // Periodically ping the slave for its location
  if (millis() - lastPing >= 200) { // every 200ms
    sendLocationRequest(SLAVE_ID);
    lastPing = millis();
  }

  // Read any incoming response
  uint8_t rx[RS485_MAX_FRAME];
  size_t n = bus.receiveResponse(rx, sizeof(rx));
  if (n >= 8 && rx[0] == SLAVE_ID && rx[1] == 0x04) {
    // Expecting 3x 16-bit big-endian values: x, y, heading*100
    int16_t x = (int16_t)((rx[3] << 8) | rx[4]);
    int16_t y = (int16_t)((rx[5] << 8) | rx[6]);
    uint16_t h = (uint16_t)((rx[7] << 8) | rx[8]);
    float heading = h / 100.0f;
    Serial.print(F("LOC x=")); Serial.print(x);
    Serial.print(F(" y=")); Serial.print(y);
    Serial.print(F(" h=")); Serial.println(heading, 2);
  }
}

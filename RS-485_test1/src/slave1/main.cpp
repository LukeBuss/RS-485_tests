// src/slave1/main.cpp
#include <Arduino.h>
#include "config.h"
#include "pinmap.h"
#include "RS485ModbusRTU.h"

#if defined(TARGET_NANO)
  RS485ModbusRTU bus(DE_RE_PIN);
#else
  RS485ModbusRTU bus(RS485_PORT, DE_RE_PIN);
#endif

// Demo data source for location
static int16_t locX = -254;     // cm or arbitrary units
static int16_t locY = -16;
static uint16_t locH100 = 35999; // heading in deg * 100 (e.g., 359.99°)

static void handleLocationRequest(const uint8_t* req, size_t n) {
  if (n < 2) return;
  if (req[0] != SLAVE_ID) return;
  if (req[1] != 0x04) return; // function 0x04: location

  uint8_t reply[] = {
    req[0],      // Slave ID
    0x04,        // Function code
    0x06,        // byte count (3 values × 2 bytes)
    (uint8_t)(locX >> 8), (uint8_t)(locX & 0xFF),
    (uint8_t)(locY >> 8), (uint8_t)(locY & 0xFF),
    (uint8_t)(locH100 >> 8), (uint8_t)(locH100 & 0xFF)
  };
  bus.sendRequest(reply, sizeof(reply));
}

void setup() {
  Serial.begin(115200);
  delay(50);

#if defined(ARDUINO_ARCH_ESP32) || defined(TARGET_ESP32)
  // Optional: pin mux
  // RS485_PORT.begin(SERIAL_SPEED, SERIAL_8N1, RX_PIN, TX_PIN);
#endif

  bus.begin(SERIAL_SPEED);
#if RS485_DEBUG
  bus.setDebug(&Serial);
  bus.enableDebug(true);
#endif
}

void loop() {
  uint8_t rx[RS485_MAX_FRAME];
  size_t n = bus.receiveResponse(rx, sizeof(rx));
  if (n >= 2) {
    // Only 0x04 supported in this demo
    handleLocationRequest(rx, n);
  }

  // (Optional) animate the values so you can see change
  // locH100 = (locH100 + 5) % 36000; // +0.05 deg each loop
}

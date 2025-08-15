#include <Arduino.h>
#include <RS485ModbusRTU.h>
#include "config.h"
#include "pinmap.h"
#include "Slave1Functions.h"

#ifdef TARGET_ESP32S3
RS485ModbusRTU bus(Serial2, DE_RE_PIN);
#else
  #error "Select TARGET_ESP32S3 in build_flags"
#endif

void setup() {
  pinMode(DE_RE_PIN, OUTPUT);
  digitalWrite(DE_RE_PIN, LOW);    // receive default

  Serial.begin(SERIAL_SPEED);
  delay(200);

  bus.begin(RS485_BAUD);
  bus.setDebug(&Serial);
  bus.enableDebug(true);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);             // turn on LED

  Serial.println("\n[SLAVE_1] ESP32-S3 ready");
}

void loop() {
  uint8_t buffer[64];
  size_t len = bus.receiveResponse(buffer, sizeof(buffer), 50);

  if (len >= 2 && buffer[1] == 0x04) {
    handleFunction04_Location(bus, buffer, len);
  }
  else if (len >= 3 && buffer[1] == 0x03) {
    handleFunction03_Add(bus, buffer, len);
  }
  // else: ignore
}

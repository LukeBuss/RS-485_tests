#include <Arduino.h>

// XIAO ESP32-S3 — Sender
#define RX_PIN D2   // Board's RX pad (not used on sender, but required by begin)
#define TX_PIN D1   // Board's TX pad

void setup() {
  Serial.begin(115200);                      // USB CDC
  Serial1.begin(115200, SERIAL_8N1, RX_PIN, TX_PIN); // map pins to UART1
  Serial.println("Sender ready");

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
}

void loop() {
  static uint32_t i = 0;
  Serial1.printf("PING %lu\n", i++);
  delay(500);
}

#include <Arduino.h>

// XIAO ESP32-S3 — Receiver
#define RX_PIN D2
#define TX_PIN D1

void setup() {
  Serial.begin(115200);                      
  Serial1.begin(115200, SERIAL_8N1, RX_PIN, TX_PIN);
  Serial.println("Receiver ready");

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
}

void loop() {
  while (Serial1.available()) {
    int c = Serial1.read();
    Serial.write(c);   // show incoming on USB Serial Monitor
  }
}

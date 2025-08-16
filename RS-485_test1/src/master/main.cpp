#include <Arduino.h>

// XIAO ESP32-S3 — Sender
#define RX_PIN D2   // Board's RX pad (not used on sender, but required by begin)
#define TX_PIN D1   // Board's TX pad

const int LED_PIN = D9; // Indicator Light

void setup() {  
  pinMode(LED_PIN, OUTPUT); // Indicator Light
  digitalWrite(LED_PIN, HIGH); // LED ON

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW); // LED ON

  Serial.begin(115200);                      // USB CDC
  Serial1.begin(115200, SERIAL_8N1, RX_PIN, TX_PIN); // map pins to UART1
  Serial.println("Sender ready");
}

void loop() {
  static uint32_t i = 0;
  Serial1.printf("ON %lu\n", i++);

  Serial.printf("LED ON %lu\n", i);
  digitalWrite(LED_PIN, HIGH); // LED ON
  digitalWrite(LED_BUILTIN, HIGH); // LED OFF
  delay(500);

  // Toggle LED state
  Serial1.printf("OFF\n");

  Serial.printf("LED OFF\n");
  digitalWrite(LED_PIN, LOW); // LED OFF
  digitalWrite(LED_BUILTIN, LOW); // LED ON
  delay(500);
}

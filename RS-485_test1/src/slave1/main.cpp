#include <Arduino.h>

// XIAO ESP32-S3 — Receiver
#define RX_PIN D2   // Board's RX pad
#define TX_PIN D1   // Board's TX pad (not used on receiver, but required by begin)

const int LED_PIN = D9; // Indicator Light
unsigned long timeSinceLastBlink = 0;

void setup() {
  pinMode(LED_PIN, OUTPUT); // Indicator Light
  digitalWrite(LED_PIN, HIGH); // LED ON

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW); // LED ON

  Serial.begin(115200);                      
  Serial1.begin(115200, SERIAL_8N1, RX_PIN, TX_PIN);
  Serial.println("Receiver ready");
}

void loop() {
  while (Serial1.available()) {
    String input = "";
    while (Serial1.available()) {
      char c = Serial1.read();
      input += c;
      delay(2); // small delay to allow buffer to fill
    }
    // if the first 3 letters of input are "ON "
    if (input.startsWith("ON ")) {
      Serial.printf("ON  %lu\n", input.substring(3).toInt());
      digitalWrite(LED_PIN, HIGH);
    } else if (input.startsWith("OFF")) {
      Serial.println("OFF");
      Serial.println();
      digitalWrite(LED_PIN, LOW);
    } else if (input.length() > 0) {
      Serial.println(input);
    }
  }
  if (millis() - timeSinceLastBlink > 1000) {
    timeSinceLastBlink = millis();
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN)); // Toggle LED
  }
}

#include <Arduino.h>
#include "config.h"
#include "pinmap.h"

#ifdef TARGET_NANO
//SoftwareSerial rs485(RS485_RX, RS485_TX);
#endif

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(DE_RE_PIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH); // Turn on LED initially
  //digitalWrite(DE_RE_PIN, LOW);

  Serial.begin(SERIAL_SPEED);
  //rs485.begin(RS485_BAUD);
}

void loop() {
  Serial.println("Hello from " ROLE_NAME);
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN)); // Toggle LED state
  delay(1000); // Wait for a second

  // // Read incoming data if available
  // if (Serial.available()) {
  //   String incomingData = Serial.readStringUntil('\n');
  //   Serial.print("Received: ");
  //   Serial.println(incomingData);
  // }
}

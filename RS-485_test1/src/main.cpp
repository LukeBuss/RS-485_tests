#include <Arduino.h>
#include "pinmap.h"
#include "config.h"

#ifdef TARGET_NANO
#include <SoftwareSerial.h>
SoftwareSerial rs485(RS485_RX, RS485_TX);
#endif

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(DE_RE_PIN, OUTPUT);
  digitalWrite(DE_RE_PIN, LOW);  // Start in receive mode

  Serial.begin(SERIAL_SPEED);
  rs485.begin(RS485_BAUD);

  Serial.print("Booted as: ");
  Serial.println(ROLE_NAME);
}

void loop() {
#ifdef ROLE_MASTER
  digitalWrite(DE_RE_PIN, HIGH);   // Enable transmit
  delayMicroseconds(100);

  rs485.println("Hello from MASTER");
  rs485.flush();                   // Wait until sent
  delayMicroseconds(100);

  digitalWrite(DE_RE_PIN, LOW);    // Back to receive
  delay(2000);                     // Send every 2 seconds

  Serial.println("Sent message from MASTER");

#else
  if (rs485.available()) {
    String msg = rs485.readStringUntil('\n');
    Serial.print("Received: ");
    Serial.println(msg);
  }
  Serial.print("Listening as: ");
  Serial.println(ROLE_NAME);
#endif

  // Blink onboard LED to show life
  static bool led = false;
  digitalWrite(LED_BUILTIN, led ? HIGH : LOW);
  led = !led;
  delay(500);
}

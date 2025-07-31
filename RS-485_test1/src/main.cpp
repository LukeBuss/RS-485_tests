#include <Arduino.h>
#include "pinmap.h"
#include "config.h"

#ifdef TARGET_NANO
#include <SoftwareSerial.h>
SoftwareSerial rs485(RS485_RX, RS485_TX);
#endif

unsigned long lastSendTime = 0;
unsigned long heartbeatTime = 0;
int counter = 0;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(DE_RE_PIN, OUTPUT);
  digitalWrite(DE_RE_PIN, LOW);  // Start in receive mode

  Serial.begin(SERIAL_SPEED);
  rs485.begin(RS485_BAUD);

  Serial.print("Booted as: ");
  Serial.println(ROLE_NAME);
}

void enableTransmit() {
  digitalWrite(DE_RE_PIN, HIGH);
  delayMicroseconds(100);
}

void disableTransmit() {
  delayMicroseconds(100);
  digitalWrite(DE_RE_PIN, LOW);
}

void loop() {
#ifdef ROLE_MASTER
  if (millis() - lastSendTime > 2000) {
    lastSendTime = millis();
    counter++;

    // Send to slave
    enableTransmit();
    rs485.println(counter);
    rs485.flush();
    disableTransmit();

    Serial.print("Sent to slave: ");
    Serial.println(counter);

    // Wait and read reply
    delay(100); // Give slave time to respond
    if (rs485.available()) {
      String response = rs485.readStringUntil('\n');
      Serial.print("Received from slave: ");
      Serial.println(response);
    }
  }

#else  // SLAVE
  if (rs485.available()) {
    String incoming = rs485.readStringUntil('\n');
    int value = incoming.toInt();
    int reply = value + 1000;

    Serial.print("Received from master: ");
    Serial.println(value);

    delay(10); // Brief pause before responding

    enableTransmit();
    rs485.println(reply);
    rs485.flush();
    disableTransmit();

    Serial.print("Sent to master: ");
    Serial.println(reply);
  }
#endif

  // LED heartbeat
  if (millis() - heartbeatTime > 500) {
    heartbeatTime = millis();

    static bool led = false;
    digitalWrite(LED_BUILTIN, led ? HIGH : LOW);
    led = !led;

    //Serial.println("Heartbeat");
  }
  delay(100); // Prevent flooding the serial output
}

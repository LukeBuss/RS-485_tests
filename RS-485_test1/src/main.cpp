#include <Arduino.h>
#include "pinmap.h"
#include "config.h"

#ifdef TARGET_NANO
#include <SoftwareSerial.h>
SoftwareSerial rs485(RS485_RX, RS485_TX);
#endif

// === Global Timers ===
unsigned long lastSendTime = 0;
unsigned long heartbeatTime = 0;

// === RS-485 Control ===
void enableTransmit() {
  digitalWrite(DE_RE_PIN, HIGH);
  delayMicroseconds(100);
}

void disableTransmit() {
  delayMicroseconds(100);
  digitalWrite(DE_RE_PIN, LOW);
}

// === MASTER ===
#ifdef ROLE_MASTER
void sendCalibrateCommand(uint8_t addr) {
  uint8_t packet[] = { 0xFA, addr, 0x80, 0x00 };
  uint8_t crc = 0;
  for (int i = 0; i < 4; i++) crc += packet[i];

  enableTransmit();
  rs485.write(packet, 4);
  rs485.write(crc & 0xFF);
  rs485.flush();
  disableTransmit();

  Serial.print("Sent Calibrate Command to 0x");
  Serial.println(addr, HEX);
}

// void readReply() {
//   if (rs485.available()) {
//     uint8_t buffer[10];
//     int i = 0;
//     while (rs485.available() && i < 10) {
//       buffer[i++] = rs485.read();
//     }

//     if (i >= 5 && buffer[0] == 0xFB) {
//       Serial.print("Received: ");
//       for (int j = 0; j < i; j++) {
//         Serial.print(buffer[j], HEX);
//         Serial.print(" ");
//       }
//       Serial.println();
//     }
//   }
// }
#endif

// === SLAVE ===
// #ifdef ROLE_SLAVE1
// void handleRequest() {
//   static uint8_t buffer[10];
//   static int index = 0;

//   while (rs485.available()) {
//     uint8_t b = rs485.read();
//     buffer[index++] = b;

//     if (index >= 5) {
//       if (buffer[0] == 0xFA && buffer[1] == 0x01 && buffer[2] == 0x80) {
//         uint8_t crc_calc = (buffer[0] + buffer[1] + buffer[2] + buffer[3]) & 0xFF;
//         if (buffer[4] == crc_calc) {
//           Serial.println("Received valid calibrate command");

//           // Response: FB 01 80 01 CRC
//           uint8_t reply[] = { 0xFB, 0x01, 0x80, 0x01 };
//           uint8_t crc = 0;
//           for (int i = 0; i < 4; i++) crc += reply[i];

//           enableTransmit();
//           rs485.write(reply, 4);
//           rs485.write(crc & 0xFF);
//           rs485.flush();
//           disableTransmit();

//           Serial.println("Sent: FB 01 80 01 CRC");
//         } else {
//           Serial.println("Invalid CRC");
//         }
//         index = 0;
//       }
//     }

//     if (index >= 10) index = 0;
//   }
// }
// #endif

// === SETUP ===
void setup() {
  pinMode(DE_RE_PIN, OUTPUT);
  digitalWrite(DE_RE_PIN, LOW);  // Start in receive mode
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(SERIAL_SPEED);
  rs485.begin(RS485_BAUD);

  Serial.print("Booted as: ");
  Serial.println(ROLE_NAME);
  delay(2000);
}

// === LOOP ===
void loop() {
#ifdef ROLE_MASTER
  if (millis() - lastSendTime > 5000) {
    lastSendTime = millis();
    sendCalibrateCommand(0x01);  // Send to slave addr 0x01
    // readReply();
  }
#endif
// #elif defined(ROLE_SLAVE1)
//   handleRequest();
//   delay(10);  // Give time for loop
// #endif

  if (millis() - heartbeatTime > 500) {
    heartbeatTime = millis();
    static bool led = false;
    digitalWrite(LED_BUILTIN, led ? HIGH : LOW);
    led = !led;
  }
  delay(100);
}

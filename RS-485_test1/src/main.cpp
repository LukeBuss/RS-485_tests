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
bool tempDir = false;

void heartbeat(int interval) {
  if (millis() - heartbeatTime > static_cast<unsigned long>(interval)) {
    heartbeatTime = millis();
    static bool led = false;
    digitalWrite(LED_BUILTIN, led ? HIGH : LOW);
    led = !led;
  }
}

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

void sendDriveCommand(uint8_t addr, uint16_t rpm, bool clockwise = true) {
  uint8_t dir = clockwise ? 0x01 : 0x00;
  uint8_t spd_l = rpm & 0xFF;
  uint8_t spd_h = (rpm >> 8) & 0xFF;

  uint8_t packet[] = { 0xFA, addr, 0xF6, dir, spd_l, spd_h };
  uint8_t crc = 0;
  for (int i = 0; i < 6; i++) crc += packet[i];

  enableTransmit();
  rs485.write(packet, 6);
  rs485.write(crc & 0xFF);
  rs485.flush();
  disableTransmit();

  Serial.print("Sent Drive Command → Addr: 0x");
  Serial.print(addr, HEX);
  Serial.print(", RPM: ");
  Serial.print(rpm);
  Serial.print(", Dir: ");
  Serial.println(clockwise ? "CW" : "CCW");
}

void requestEncoderAngle(uint8_t addr) {
  uint8_t packet[] = { 0xFA, addr, 0x30 };
  uint8_t crc = packet[0] + packet[1] + packet[2];

  enableTransmit();
  rs485.write(packet, 3);
  rs485.write(crc & 0xFF);
  rs485.flush();
  disableTransmit();

  Serial.println("Sent encoder angle request (FA 01 30)");
}

void readEncoderAngleResponse() {
  static const uint8_t START_BYTE = 0xFB;
  static bool inFrame = false;
  static uint8_t buffer[10];
  static uint8_t index = 0;

  while (rs485.available()) {
    uint8_t byte = rs485.read();

    // Look for frame start
    if (!inFrame) {
      if (byte == START_BYTE) {
        inFrame = true;
        buffer[0] = byte;
        index = 1;
      }
    } else {
      // Inside frame
      if (byte == START_BYTE) {
        // New frame detected early — reset
        buffer[0] = byte;
        index = 1;
      } else {
        buffer[index++] = byte;

        if (index >= 10) {
          // Parse when we have enough
          inFrame = false;
          index = 0;

          // Basic CRC check
          uint8_t crc = 0;
          for (int i = 0; i < 9; i++) crc += buffer[i];

          if (crc != buffer[9]) {
            Serial.print("CRC error: ");
            for (int i = 0; i < 10; i++) {
              Serial.print(buffer[i], HEX); Serial.print(" ");
            }
            Serial.println();
            return;
          }

          // Valid packet structure?
          if (buffer[1] == 0x01 && buffer[2] == 0x30) {
            int32_t carry = buffer[3] | (buffer[4] << 8) | (buffer[5] << 16) | (buffer[6] << 24);
            uint16_t ticks = buffer[7] | (buffer[8] << 8);
            ticks &= 0x3FFF;

            float angle = ticks * 360.0f / 16384.0f;
            float totalAngle = angle + (carry * 360.0f);

            Serial.print("Ticks: "); Serial.print(ticks);
            Serial.print(" → Degrees: "); Serial.print(angle, 2);
            Serial.print("°, Turns: "); Serial.print(carry);
            Serial.print(" → Total: ");
            if (carry == 16777216 || carry == -16777216) {
              Serial.println("ovf°");
            } else {
              Serial.print(totalAngle, 2); Serial.println("°");
            }
          } else {
            Serial.println("Invalid function or address");
          }
        }
      }
    }
  }
}

#endif

// === SETUP ===
void setup() {
  pinMode(DE_RE_PIN, OUTPUT);
  digitalWrite(DE_RE_PIN, LOW);  // Start in receive mode
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(SERIAL_SPEED);
  rs485.begin(RS485_BAUD);

  Serial.print("Booted as: ");
  Serial.println(ROLE_NAME);
  sendCalibrateCommand(0x01);  // Send to slave addr 0x01
  delay(10000);
}

// === LOOP ===
void loop() {
#ifdef ROLE_MASTER
  if (millis() - lastSendTime > static_cast<unsigned long>(1000)) {
    lastSendTime = millis();
    // if (tempDir) {
    //   sendDriveCommand(0x01, 160, false); // Send to slave addr 0x01, 160 RPM, CCW
    //   tempDir = false;
    // } else {
    //   sendDriveCommand(0x01, 320, true);  // Send to slave addr 0x01, 320 RPM, CW
    //   tempDir = true;
    // }
    requestEncoderAngle(0x01);  // Request encoder position from slave addr 0x01
    delay(50);
    readEncoderAngleResponse();  // Read and process the encoder position response
  }

  heartbeat(500);

#else
  #error "Unknown Role: " ROLE_NAME
#endif
  delay(100);
}

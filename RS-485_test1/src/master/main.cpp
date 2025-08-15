#include <Arduino.h>
#include <RS485ModbusRTU.h>
#include "config.h"
#include "pinmap.h"
#include "MasterFunctions.h"

#ifdef TARGET_ESP32S3
RS485ModbusRTU bus(Serial2, DE_RE_PIN);
#else
  #error "Select TARGET_ESP32S3 in build_flags"
#endif

void setup() {
  pinMode(DE_RE_PIN, OUTPUT);
  digitalWrite(DE_RE_PIN, LOW);               // default to receive

  Serial.begin(SERIAL_SPEED);
  delay(200);

  bus.begin(RS485_BAUD);
  bus.setDebug(&Serial);
  bus.enableDebug(true);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);             // turn on LED

  Serial.println("\n[MASTER] ESP32-S3 ready");
  Serial.print("ROLE: "); Serial.println(ROLE_NAME);
}

static void printTiming(unsigned long t1, unsigned long t2, unsigned long t3) {
  Serial.print("TX time(us): "); Serial.print(t1);
  Serial.print("  RX wait(us): "); Serial.print(t2);
  Serial.print("  RX frame(us): "); Serial.println(t3);
}

void loop() {
  const uint8_t SLAVE_ID = 0x01;

  // Example: request location (function 0x04)
  uint8_t pkt[2] = { SLAVE_ID, 0x04 };

  unsigned long t0 = micros();
  bus.sendRequest(pkt, sizeof(pkt));
  unsigned long t1 = micros();

  uint8_t resp[64];
  size_t len = bus.receiveResponse(resp, sizeof(resp), 50);
  unsigned long t2 = micros();

  if (len >= 2 && resp[0] == SLAVE_ID && resp[1] == 0x04 && len == 2 + 1 + 6) {
    // [ID][FC][ByteCount=6][xH][xL][yH][yL][hH][hL]
    int16_t x = (int16_t)((resp[3] << 8) | resp[4]);
    int16_t y = (int16_t)((resp[5] << 8) | resp[6]);
    uint16_t heading = (uint16_t)((resp[7] << 8) | resp[8]);

    Serial.print("X: "); Serial.print(x);
    Serial.print("  Y: "); Serial.print(y);
    Serial.print("  H(deg*100): "); Serial.println(heading);

    printTiming(t1 - t0, (t2 - t1), 0UL);
  } else if (len > 0) {
    Serial.println("Invalid response:");
    bus.printBytes(resp, len);
  } else {
    Serial.println("No response / timeout");
  }

  delay(1000);
}

#include <Arduino.h>

#define RS485_TX  D7   // unused but required by begin()
#define RS485_RX  D8   // from MAX3485 RO
#define BAUD_RS485 115200

void setup() {
  Serial.begin(115200);
  delay(200);

  Serial2.begin(BAUD_RS485, SERIAL_8N1, RS485_RX, RS485_TX);
  Serial.println("\n[RX-ONLY] listening. EN is hard-tied LOW.");
}

void loop() {
  // Look for our 0x55 0xAA header and then read the rest
  static enum { SEEK55, SEEKAA, READ4 } state = SEEK55;
  static uint8_t buf[4];
  static int idx = 0;

  while (Serial2.available()) {
    uint8_t b = Serial2.read();

    switch (state) {
      case SEEK55: if (b == 0x55) state = SEEKAA; break;
      case SEEKAA: if (b == 0xAA) { state = READ4; idx = 0; } else state = SEEK55; break;
      case READ4:
        buf[idx++] = b;
        if (idx == 4) {
          uint16_t cnt = (uint16_t(buf[0]) << 8) | buf[1];
          char c1 = (char)buf[2], c2 = (char)buf[3];
          Serial.print("RX OK  counter="); Serial.print(cnt);
          Serial.print("  text="); Serial.print(c1); Serial.println(c2);
          state = SEEK55;
        }
        break;
    }
    Serial.println("Read, but error :" + String(b));
  }
}

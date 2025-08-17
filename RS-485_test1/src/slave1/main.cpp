#include <Arduino.h>

// ---- Pin map (XIAO ESP32-S3) ----
const int PIN_EN = D9;   // DE & !RE tied together (HIGH=TX, LOW=RX)
const int PIN_RX = D8;   // RO -> MCU RX
const int PIN_TX = D7;   // MCU TX -> DI

// ---- Timing ----
#define RS485_PRE_TX_US   40
#define RS485_POST_TX_US  120

static inline void rs485Begin() {
  pinMode(PIN_EN, OUTPUT);
  digitalWrite(PIN_EN, LOW);  // always listen unless sending
  Serial.begin(115200);
  Serial1.begin(115200, SERIAL_8N1, PIN_RX, PIN_TX);
  delay(20);
}

static inline void rs485Write(const uint8_t* data, size_t len) {
  digitalWrite(PIN_EN, HIGH);
  delayMicroseconds(RS485_PRE_TX_US);
  Serial1.write(data, len);
  Serial1.flush();
  delayMicroseconds(RS485_POST_TX_US);
  digitalWrite(PIN_EN, LOW);
}

bool readLine(char* buf, size_t buflen) {
  static size_t i = 0;
  while (Serial1.available()) {
    char c = (char)Serial1.read();
    if (c == '\n') {
      if (i < buflen) buf[i] = '\0';
      i = 0;
      return true;
    }
    if (i + 1 < buflen) buf[i++] = c; else i = 0; // reset on overflow
  }
  return false;
}

void setup() {
  rs485Begin();
  Serial.println("[SLAVE] RS-485 up");
}

void loop() {
  char line[64];
  if (readLine(line, sizeof(line))) {
    Serial.print("RX: "); Serial.println(line);

    char echo[96];
    int len = snprintf(echo, sizeof(echo), "ECHO: %s\n", line);
    rs485Write((uint8_t*)echo, len);
    Serial.print("TX: "); Serial.write(echo, len);
  }
  delay(1);  // yield
}

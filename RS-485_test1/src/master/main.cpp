#include <Arduino.h>

// ---- Pin map (XIAO ESP32-S3) ----
const int PIN_EN = D9;   // DE & !RE tied together (HIGH=TX, LOW=RX)
const int PIN_RX = D8;   // RO -> MCU RX
const int PIN_TX = D7;   // MCU TX -> DI

// ---- Timing (adjust if needed) ----
#define RS485_PRE_TX_US   40    // settle after EN HIGH before sending
#define RS485_POST_TX_US  120   // guard time after flush before EN LOW

// Helpers
static inline void rs485Begin() {
  pinMode(PIN_EN, OUTPUT);
  digitalWrite(PIN_EN, LOW);  // start in receive
  Serial.begin(115200);
  Serial1.begin(115200, SERIAL_8N1, PIN_RX, PIN_TX);
  delay(20);
}

static inline void rs485Write(const uint8_t* data, size_t len) {
  digitalWrite(PIN_EN, HIGH);                 // TX on, RX off
  delayMicroseconds(RS485_PRE_TX_US);
  Serial1.write(data, len);
  Serial1.flush();                            // wait until shifted out
  delayMicroseconds(RS485_POST_TX_US);
  digitalWrite(PIN_EN, LOW);                  // back to RX
}

// Read a '\n'-terminated line with timeout (ms)
// Returns true if a line was captured into buf (stripped of '\n')
bool readLine(char* buf, size_t buflen, uint32_t timeout_ms) {
  size_t i = 0;
  uint32_t t0 = millis();
  while (millis() - t0 < timeout_ms) {
    while (Serial1.available()) {
      char c = (char)Serial1.read();
      if (c == '\n') {
        if (i < buflen) buf[i] = '\0';
        return true;
      }
      if (i + 1 < buflen) buf[i++] = c;      // keep space for NUL
    }
    delay(1);
  }
  if (i < buflen) buf[i] = '\0';
  return false;
}

void setup() {
  rs485Begin();
  Serial.println("[MASTER] RS-485 up");
}

void loop() {
  static uint32_t n = 0;
  char out[32];
  int len = snprintf(out, sizeof(out), "PING %lu\n", (unsigned long)n++);

  rs485Write((uint8_t*)out, len);
  Serial.print("TX: "); Serial.write(out, len);

  char in[64];
  if (readLine(in, sizeof(in), 250)) {
    Serial.print("RX: "); Serial.println(in);
  } else {
    Serial.println("RX: (timeout)");
  }

  delay(500);
}

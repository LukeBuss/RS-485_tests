#include <HardwareSerial.h>

HardwareSerial mySerial(1); 

unsigned long blinkTimer = 0;

// XIAO ESP32-S3 + MAX3485 half-duplex
const int PIN_EN = D9;   // DE & !RE tied here (HIGH=TX, LOW=RX)
const int PIN_RX = D8;   // RO -> MCU RX
const int PIN_TX = D7;   // MCU TX -> DI

void rs485Begin() {
  pinMode(PIN_EN, OUTPUT);
  digitalWrite(PIN_EN, HIGH); // start in receive
  Serial.begin(115200);      // USB debug
  Serial1.begin(115200, SERIAL_8N1, PIN_RX, PIN_TX); // map UART to D8/D7
}

// transmit helper (toggles DE/!RE)
void rs485Write(const uint8_t* data, size_t len) {
  digitalWrite(PIN_EN, LOW);           // TX on, RX off
  delayMicroseconds(2);                 // enable settle (conservative)
  Serial1.write(data, len);
  Serial1.flush();                      // wait for bytes to leave
  delayMicroseconds(2);                 // turn-around guard
  digitalWrite(PIN_EN, HIGH);            // back to RX
}




void setup() {
  pinMode(LED_BUILTIN, OUTPUT); // Set the built-in LED pin as an output
  digitalWrite(LED_BUILTIN, LOW); // Turn the LED ON

  rs485Begin(); Serial.println("Master up");
}

void loop() {
  static uint32_t count = 0;
  static uint32_t n = 0;
  char buf[32];
  int len = snprintf(buf, sizeof(buf), "PING %lu\n", (unsigned long)n++);
  rs485Write((uint8_t*)buf, len);

  uint32_t t0 = millis();
  while (millis() - t0 < 200) {        // short listen window
    while (Serial1.available()) Serial.write(Serial1.read());
  }

  if (millis() - blinkTimer > 1000) {
    blinkTimer = millis();
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN)); // Toggle the built-in LED
    Serial.print("LED toggled at: ");
    Serial.println(++count);
  }
  delay(500);
}

#include <HardwareSerial.h>

HardwareSerial mySerial(1); // Use UART2
#define enable_pin D2 // Define the enable pin as D2

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

  rs485Begin(); Serial.println("Slave1 up");
}

void loop() {
  static uint32_t count = 0;
  static char inbuf[64];
  static int idx = 0;

  while (Serial1.available()) {
    char c = (char)Serial1.read();
    Serial.write(c);                    // show on USB
    if (idx < (int)sizeof(inbuf)-1) inbuf[idx++] = c;
    if (c == '\n') {                    // got a line -> echo it
      inbuf[idx] = '\0';
      rs485Write((uint8_t*)inbuf, idx);
      idx = 0;
    }
  }

  if (millis() - blinkTimer > 1000) {
    blinkTimer = millis();
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN)); // Toggle the built-in LED
    Serial.print("LED toggled at: ");
    Serial.println(++count);
  }
  delay(500);
}

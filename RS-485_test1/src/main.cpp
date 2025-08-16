#include <Arduino.h>

#ifdef nRF52_SERIES
  #include <Adafruit_TinyUSB.h> // Only needed for Seeed nRF52 Boards core
#endif

const int LED_PIN = D9; // D7 = 44, D8 = 7, D9 = 8

void setup() {
  Serial.begin(SERIAL_BAUD); // Uses value from platformio.ini
  //while (!Serial) delay(10);

  pinMode(LED_BUILTIN, OUTPUT);
  Serial.println("Starting blink test...");

  pinMode(LED_PIN, OUTPUT); // Indicator Light
  digitalWrite(LED_PIN, LOW); // Start OFF
}

void loop() {
  digitalWrite(LED_BUILTIN, LOW); // LED ON
  digitalWrite(LED_PIN, HIGH); // Indicator ON
  Serial.println("LED ON");
  delay(500);

  digitalWrite(LED_BUILTIN, HIGH); // LED OFF
  digitalWrite(LED_PIN, LOW); // Indicator OFF
  Serial.println("LED OFF");
  delay(500);
}

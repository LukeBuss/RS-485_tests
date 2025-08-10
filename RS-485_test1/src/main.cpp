#include <Arduino.h>

#ifdef nRF52_SERIES
  #include <Adafruit_TinyUSB.h> // Only needed for Seeed nRF52 Boards core
#endif

void setup() {
  Serial.begin(SERIAL_BAUD); // Uses value from platformio.ini
  while (!Serial) delay(10);

  pinMode(LED_BUILTIN, OUTPUT);
  Serial.println("Starting blink test...");
}

void loop() {
  digitalWrite(LED_BUILTIN, LOW); // LED ON
  Serial.println("LED ON");
  delay(500);

  digitalWrite(LED_BUILTIN, HIGH); // LED OFF
  Serial.println("LED OFF");
  delay(500);
}

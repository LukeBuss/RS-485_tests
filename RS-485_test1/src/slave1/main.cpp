#include <Arduino.h>

// -------- Pin map (XIAO ESP32-S3) --------
// D7 = TX (GPIO44), D8 = RX (GPIO7), D9 = external LED (GPIO8)
#ifndef LED_BUILTIN
  #define LED_BUILTIN 21   // XIAO S3 user LED
#endif
const bool LED_BUILTIN_ACTIVE_LOW = true;

const int PIN_LED_EXT = D9;     // active-HIGH LED
const int PIN_UART_TX = D1;     // -> other board RX
const int PIN_UART_RX = D2;     // <- other board TX

// Tasks
void taskBlinkBuiltin(void *);  // core 0: 1 Hz blink
void taskUartRx(void *);        // core 1: receive commands, drive D9

void setup() {
  Serial.begin(115200);
  delay(50);
  Serial.println("[SLAVE] start");

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(PIN_LED_EXT, OUTPUT);
  digitalWrite(PIN_LED_EXT, LOW);
  digitalWrite(LED_BUILTIN, LED_BUILTIN_ACTIVE_LOW ? HIGH : LOW); // off

  // Route UART1 to D8 (RX) / D7 (TX)
  Serial1.begin(115200, SERIAL_8N1, PIN_UART_RX, PIN_UART_TX);

  // Core 0: blink; Core 1: RX
  xTaskCreatePinnedToCore(taskBlinkBuiltin, "blink", 2048, nullptr, 1, nullptr, 0);
  xTaskCreatePinnedToCore(taskUartRx,       "uart_rx", 4096, nullptr, 1, nullptr, 1);
}

void loop() {
  vTaskDelay(pdMS_TO_TICKS(1000));
}

// -------- core0: blink builtin every 1000 ms --------
void taskBlinkBuiltin(void *) {
  bool on = false;
  for (;;) {
    on = !on;
    digitalWrite(LED_BUILTIN, LED_BUILTIN_ACTIVE_LOW ? (on ? LOW : HIGH)
                                                     : (on ? HIGH : LOW));
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

// -------- core1: parse "LED_ON"/"LED_OFF" and drive D9 --------
void taskUartRx(void *) {
  static char line[32];
  size_t idx = 0;

  for (;;) {
    while (Serial1.available()) {
      char c = (char)Serial1.read();

      if (c == '\n' || c == '\r') {
        if (idx > 0) {
          line[idx] = '\0';
          if (!strcmp(line, "LED_ON")) {
            digitalWrite(PIN_LED_EXT, HIGH);
            Serial.println("RX: LED_ON");
          } else if (!strcmp(line, "LED_OFF")) {
            digitalWrite(PIN_LED_EXT, LOW);
            Serial.println("RX: LED_OFF");
          } else {
            Serial.print("RX: unknown -> "); Serial.println(line);
          }
          idx = 0;
        }
      } else if (idx < sizeof(line) - 1) {
        line[idx++] = c;
      } else {
        idx = 0; // overflow, reset
      }
    }
    vTaskDelay(pdMS_TO_TICKS(5));
  }
}

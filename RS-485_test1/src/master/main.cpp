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

// Shared state (updated by core0, used by core1)
volatile bool g_ledCmdOn = false;

// Tasks
void taskBlinkBuiltin(void *);  // core 0: 1 Hz blink, sets g_ledCmdOn
void taskUartTx(void *);        // core 1: send LED_ON/OFF every 500 ms and drive D9

void setup() {
  Serial.begin(115200);
  delay(50);
  Serial.println("[MASTER] start");

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(PIN_LED_EXT, OUTPUT);
  digitalWrite(PIN_LED_EXT, LOW);
  digitalWrite(LED_BUILTIN, LED_BUILTIN_ACTIVE_LOW ? HIGH : LOW); // off

  // Route UART1 to D8 (RX) / D7 (TX)
  Serial1.begin(115200, SERIAL_8N1, PIN_UART_RX, PIN_UART_TX);

  // Core 0: blink; Core 1: TX
  xTaskCreatePinnedToCore(taskBlinkBuiltin, "blink", 2048, nullptr, 1, nullptr, 0);
  xTaskCreatePinnedToCore(taskUartTx,       "uart_tx", 4096, nullptr, 1, nullptr, 1);
}

void loop() {
  vTaskDelay(pdMS_TO_TICKS(1000));
}

// -------- core0: blink builtin every 1000 ms --------
void taskBlinkBuiltin(void *) {
  bool on = false;
  for (;;) {
    on = !on;
    // builtin LED is active-LOW on XIAO S3
    digitalWrite(LED_BUILTIN, LED_BUILTIN_ACTIVE_LOW ? (on ? LOW : HIGH)
                                                     : (on ? HIGH : LOW));
    g_ledCmdOn = on;  // publish state for TX task
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

// -------- core1: send command every 500 ms + mirror to D9 --------
void taskUartTx(void *) {
  for (;;) {
    // Mirror state locally
    digitalWrite(PIN_LED_EXT, g_ledCmdOn ? HIGH : LOW);

    if (g_ledCmdOn)  { Serial1.print("LED_ON\n");  Serial.println("TX: LED_ON");  }
    else             { Serial1.print("LED_OFF\n"); Serial.println("TX: LED_OFF"); }

    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

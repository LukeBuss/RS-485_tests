#include <Arduino.h>
#include <LogQueue.h>
#include <RS485.h>
extern "C" {
  #include "driver/uart.h"
  #include "esp_timer.h"
}

// Forward declarations for the IDF-style helpers defined in lib/RS485/RS485.cpp
extern void rs485_init(uint32_t baud);
extern void task_rx(void*);
extern void task_tx(void*);

void setup() {
  Serial.begin(921600);
  Log::init(Log::DROP_OLDEST, 1, 2048, 1); // init LogQueue with default settings
  rs485_init(10000000);
  xTaskCreatePinnedToCore(task_rx, "rx", 4096, nullptr, 3, nullptr, 0); // core 0
  xTaskCreatePinnedToCore(task_tx, "tx", 4096, nullptr, 2, nullptr, 0); // core 0
}

void loop() { vTaskDelay(pdMS_TO_TICKS(1000)); }

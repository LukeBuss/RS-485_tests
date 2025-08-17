#include "LogQueue.h"
#include <string.h>

namespace Log {

struct LogMsg {
  uint16_t len;
  char buf[LOG_MSG_MAX];
};

static QueueHandle_t sQueue = nullptr;
static DropPolicy    sPolicy = DROP_OLDEST;
static SemaphoreHandle_t sWriteMtx = nullptr;

static void loggerTask(void*) {
  LogMsg m;
  for (;;) {
    if (xQueueReceive(sQueue, &m, portMAX_DELAY) == pdTRUE) {
      if (sWriteMtx) xSemaphoreTake(sWriteMtx, portMAX_DELAY);
      Serial.write((const uint8_t*)m.buf, m.len);   // no interrupts masked
      if (sWriteMtx) xSemaphoreGive(sWriteMtx);
    }
  }
}

void init(DropPolicy policy, UBaseType_t taskPrio, uint32_t taskStack, BaseType_t core) {
  if (!sQueue) {
    sQueue   = xQueueCreate(LOG_QUEUE_LEN, sizeof(LogMsg));
    sPolicy  = policy;
    sWriteMtx = xSemaphoreCreateMutex();
    xTaskCreatePinnedToCore(loggerTask, "logger", taskStack, nullptr, taskPrio, nullptr, core);
  }
}

void printf(const char* fmt, ...) {
  if (!sQueue) return;
  LogMsg m;
  va_list ap; va_start(ap, fmt);
  int n = vsnprintf(m.buf, LOG_MSG_MAX, fmt, ap);
  va_end(ap);
  if (n < 0) return;
  if (n >= LOG_MSG_MAX) n = LOG_MSG_MAX - 1;
  m.len = (uint16_t)n;

  if (xQueueSend(sQueue, &m, 0) != pdTRUE) {
    if (sPolicy == DROP_OLDEST) {
      LogMsg drop;
      if (xQueueReceive(sQueue, &drop, 0) == pdTRUE) (void)xQueueSend(sQueue, &m, 0);
    }
    // else DROP_NEWEST: silently drop newest
  }
}

void flush(uint32_t timeoutMs) {
  uint32_t t0 = millis();
  while (uxQueueMessagesWaiting(sQueue) > 0 && (millis() - t0) < timeoutMs) {
    vTaskDelay(pdMS_TO_TICKS(5));
  }
}

} // namespace Log

#include <Arduino.h>
extern "C" {
  #include "driver/uart.h"
}

#define UART_PORT   UART_NUM_1
#define TXD_PIN     GPIO_NUM_44   // D7
#define RXD_PIN     GPIO_NUM_7    // D8
#define RTS_DE_PIN  GPIO_NUM_8    // D9 -> DE&!RE (hardware drives it)

#ifndef LED_BUILTIN
  #define LED_BUILTIN 21          // XIAO S3 user LED (active-LOW)
#endif
const bool LED_ACTIVE_LOW = true;

static QueueHandle_t uart_queue = nullptr;

static void rs485_init(uint32_t baud = 1000000) {
  uart_config_t cfg = {
    .baud_rate = (int)baud,
    .data_bits = UART_DATA_8_BITS,
    .parity    = UART_PARITY_DISABLE,
    .stop_bits = UART_STOP_BITS_1,
    .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    .source_clk = UART_SCLK_APB
  };
  ESP_ERROR_CHECK(uart_param_config(UART_PORT, &cfg));
  ESP_ERROR_CHECK(uart_set_pin(UART_PORT, TXD_PIN, RXD_PIN, RTS_DE_PIN, UART_PIN_NO_CHANGE));
  ESP_ERROR_CHECK(uart_driver_install(UART_PORT, 4096, 4096, 10, &uart_queue, 0));
  ESP_ERROR_CHECK(uart_set_mode(UART_PORT, UART_MODE_RS485_HALF_DUPLEX)); // auto-DE
}

// Minimal line parser (handles chunked arrivals)
static void apply_line(const char* s) {
  if (!strcmp(s, "LED_ON"))  digitalWrite(LED_BUILTIN, LED_ACTIVE_LOW ? LOW : HIGH);
  if (!strcmp(s, "LED_OFF")) digitalWrite(LED_BUILTIN, LED_ACTIVE_LOW ? HIGH : LOW);
}

void task_rx(void *){
  uart_event_t ev;
  static char line[64];
  size_t li = 0;

  for(;;){
    if (xQueueReceive(uart_queue, &ev, portMAX_DELAY)) {
      if (ev.type == UART_DATA && ev.size > 0) {
        uint8_t buf[256];
        const uint32_t to_read = (uint32_t)((ev.size < sizeof(buf)) ? ev.size : sizeof(buf));
        int n = uart_read_bytes(UART_PORT, buf, to_read, 0);
        for (int i=0; i<n; ++i) {
          char c = (char)buf[i];
          if (c == '\n' || c == '\r') {
            if (li) { line[li] = '\0'; apply_line(line); li = 0; }
          } else if (li < sizeof(line)-1) {
            line[li++] = c;
          } else {
            li = 0; // overflow -> reset
          }
        }
      }
      // (Optional) handle other events: UART_FIFO_OVF, UART_BUFFER_FULL, etc.
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LED_ACTIVE_LOW ? HIGH : LOW);
  rs485_init(1000000);
  xTaskCreatePinnedToCore(task_rx, "rx", 4096, nullptr, 3, nullptr, 0);
}

void loop(){ vTaskDelay(pdMS_TO_TICKS(1000)); }

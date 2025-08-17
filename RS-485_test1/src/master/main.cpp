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

static void rs485_init(uint32_t baud = 1000000) { // 1 Mbps default
  uart_config_t cfg = {
    .baud_rate = (int)baud,
    .data_bits = UART_DATA_8_BITS,
    .parity    = UART_PARITY_DISABLE,
    .stop_bits = UART_STOP_BITS_1,
    .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    .source_clk = UART_SCLK_APB
  };
  ESP_ERROR_CHECK(uart_param_config(UART_PORT, &cfg));
  // RTS is used by RS-485 mode as DE. No CTS used.
  ESP_ERROR_CHECK(uart_set_pin(UART_PORT, TXD_PIN, RXD_PIN, RTS_DE_PIN, UART_PIN_NO_CHANGE));
  // Bigger RX/TX buffers keep things smooth; queue gives you RX events if you need them later
  ESP_ERROR_CHECK(uart_driver_install(UART_PORT, 4096, 4096, 10, &uart_queue, 0));
  ESP_ERROR_CHECK(uart_set_mode(UART_PORT, UART_MODE_RS485_HALF_DUPLEX)); // auto-DE!
}

void task_tx(void *){
  bool on = false;
  for(;;){
    on = !on;
    digitalWrite(LED_BUILTIN, LED_ACTIVE_LOW ? (on ? LOW : HIGH)
                                             : (on ? HIGH : LOW));
    const char *msg = on ? "LED_ON\n" : "LED_OFF\n";
    uart_write_bytes(UART_PORT, msg, strlen(msg));
    // No manual DE toggling; auto-DE handles turn-around cleanly
    uart_wait_tx_done(UART_PORT, pdMS_TO_TICKS(5));
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LED_ACTIVE_LOW ? HIGH : LOW);
  rs485_init(1000000);   // try 2'000'000 after it’s stable
  xTaskCreatePinnedToCore(task_tx, "tx", 4096, nullptr, 3, nullptr, 0);
}

void loop(){ vTaskDelay(pdMS_TO_TICKS(1000)); }

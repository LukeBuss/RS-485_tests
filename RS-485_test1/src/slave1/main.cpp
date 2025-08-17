#include <Arduino.h>
extern "C" {
  #include "driver/uart.h"
}

// ==== RS-485 / UART pins (XIAO ESP32-S3) ====
#define UART_PORT   UART_NUM_1
#define TXD_PIN     GPIO_NUM_44   // D7
#define RXD_PIN     GPIO_NUM_7    // D8
#define RTS_DE_PIN  GPIO_NUM_8    // D9

// ==== Protocol ====
static const uint8_t STX = 0xFA;
static const uint8_t ADDR_ME = 0x01;     // this slave's address
static const uint8_t CMD_LOC_REQ = 0x04;
static const uint8_t CMD_LOC_RSP = 0x84; // reply = cmd | 0x80

static QueueHandle_t uart_queue = nullptr;

// ------------ CRC16 Modbus ------------
static uint16_t crc16_modbus(const uint8_t* data, size_t len) {
  uint16_t crc = 0xFFFF;
  for (size_t i = 0; i < len; ++i) {
    crc ^= data[i];
    for (int b = 0; b < 8; ++b) {
      crc = (crc & 1) ? (crc >> 1) ^ 0xA001 : (crc >> 1);
    }
  }
  return crc;
}

static void sendFrame(uint8_t addr, uint8_t cmd, const uint8_t* payload, uint8_t plen) {
  uint8_t buf[64]; size_t i = 0;
  buf[i++] = STX; buf[i++] = addr; buf[i++] = cmd; buf[i++] = plen;
  if (plen && payload) { memcpy(&buf[i], payload, plen); i += plen; }
  uint16_t crc = crc16_modbus(buf, i);
  buf[i++] = (uint8_t)(crc & 0xFF);
  buf[i++] = (uint8_t)(crc >> 8);

  uart_write_bytes(UART_PORT, (const char*)buf, i);
  uart_wait_tx_done(UART_PORT, pdMS_TO_TICKS(20));
}

static bool parseOneFrame(uint8_t* in, size_t n, size_t& used,
                          uint8_t& addr, uint8_t& cmd, uint8_t* payload, uint8_t& plen)
{
  size_t p = 0;
  while (p < n && in[p] != 0xFA) ++p;
  if (p >= n) { used = n; return false; }
  if (n - p < 6) { used = p; return false; }

  uint8_t len = in[p+3];
  size_t need = 4 + len + 2;
  if (n - p < need) { used = p; return false; }

  uint16_t crc_calc = crc16_modbus(&in[p], 4 + len);
  uint16_t crc_rx   = (uint16_t)in[p+4+len] | ((uint16_t)in[p+5+len] << 8);
  if (crc_calc != crc_rx) { used = p + 1; return false; }

  addr = in[p+1]; cmd = in[p+2]; plen = len;
  if (len && payload) memcpy(payload, &in[p+4], len);
  used = p + need;
  return true;
}

// Hardcoded location: x,y in cm (signed), heading in centideg (0..35999)
static void sendHardcodedLocation() {
  int16_t x_cm = -25359;    // -253.59 cm
  int16_t y_cm = -1600;     // -16.00 cm
  uint16_t h_cdeg = 35999;  // 359.99 deg

  uint8_t p[6];
  p[0] = (uint8_t)(x_cm & 0xFF);
  p[1] = (uint8_t)(x_cm >> 8);
  p[2] = (uint8_t)(y_cm & 0xFF);
  p[3] = (uint8_t)(y_cm >> 8);
  p[4] = (uint8_t)(h_cdeg & 0xFF);
  p[5] = (uint8_t)(h_cdeg >> 8);

  sendFrame(ADDR_ME, CMD_LOC_RSP, p, 6);
}

static void task_rx(void*) {
  uart_event_t ev;
  static uint8_t q[256]; size_t qn = 0;

  for (;;) {
    if (xQueueReceive(uart_queue, &ev, portMAX_DELAY)) {
      if (ev.type == UART_DATA && ev.size > 0) {
        uint8_t buf[256];
        size_t to_read = (ev.size < sizeof(buf)) ? ev.size : sizeof(buf);
        int n = uart_read_bytes(UART_PORT, buf, to_read, 0);
        if (n > 0) {
          if (qn + (size_t)n > sizeof(q)) qn = 0;
          memcpy(&q[qn], buf, n);
          qn += n;

          size_t used = 0;
          while (true) {
            uint8_t addr, cmd, payload[64], plen = 0;
            size_t consumed = 0;
            bool ok = parseOneFrame(q, qn, consumed, addr, cmd, payload, plen);
            if (!ok) {
              if (consumed && consumed < qn) memmove(q, q + consumed, qn - consumed), qn -= consumed;
              else if (consumed == qn) qn = 0;
              break;
            }
            memmove(q, q + consumed, qn - consumed);
            qn -= consumed;

            if ((addr == ADDR_ME) && (cmd == CMD_LOC_REQ) && (plen == 0)) {
              sendHardcodedLocation();
            }
          }
        }
      }
    }
  }
}

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
  ESP_ERROR_CHECK(uart_set_mode(UART_PORT, UART_MODE_RS485_HALF_DUPLEX));
}

void setup() {
  Serial.begin(115200);
  rs485_init(1000000);
  xTaskCreatePinnedToCore(task_rx, "rx", 4096, nullptr, 3, nullptr, 0); // core 0
}

void loop() { vTaskDelay(pdMS_TO_TICKS(1000)); }

#include <Arduino.h>
#include "LogQueue.h"
#include "RS485.h"
extern "C" {
  #include "driver/uart.h"
  #include "esp_timer.h"
}

// ==== RS-485 / UART pins (XIAO ESP32-S3) ====
#define UART_PORT   UART_NUM_1
#define TXD_PIN     GPIO_NUM_44   // D7
#define RXD_PIN     GPIO_NUM_7    // D8
#define RTS_DE_PIN  GPIO_NUM_8    // D9 -> tie to DE & !RE of MAX3485

// ==== Protocol ====
static const uint8_t STX = 0xFA;
static const uint8_t ADDR_SLAVE = 0x01;
static const uint8_t CMD_LOC_REQ = 0x04;
static const uint8_t CMD_LOC_RSP = 0x84;   // reply = cmd | 0x80

// ==== Globals for timing ====
static volatile int64_t g_t0_req_begin_us = 0;
static volatile int64_t g_t1_req_sent_us  = 0;

// UART RX event queue
static QueueHandle_t uart_queue = nullptr;

// ------------ CRC16 Modbus (0xA001), little-endian ------------
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

// ------------ Send framed packet ------------
static void sendFrame(uint8_t addr, uint8_t cmd, const uint8_t* payload, uint8_t plen) {
  uint8_t buf[64];
  size_t i = 0;
  buf[i++] = STX;
  buf[i++] = addr;
  buf[i++] = cmd;
  buf[i++] = plen;
  if (plen && payload) { memcpy(&buf[i], payload, plen); i += plen; }
  uint16_t crc = crc16_modbus(buf, i);
  buf[i++] = (uint8_t)(crc & 0xFF);
  buf[i++] = (uint8_t)(crc >> 8);

  // t0: right before TX
  g_t0_req_begin_us = esp_timer_get_time();

  // Auto-DE handles direction; just write
  uart_write_bytes(UART_PORT, (const char*)buf, i);
  // Wait until *all* bytes leave the HW (end of stop bit)
  uart_wait_tx_done(UART_PORT, pdMS_TO_TICKS(20));

  // t1: just after TX completed
  g_t1_req_sent_us = esp_timer_get_time();
}

// ------------ Simple framed parser ------------
static bool parseOneFrame(uint8_t* in, size_t n, size_t& used,
                          uint8_t& addr, uint8_t& cmd, uint8_t* payload, uint8_t& plen)
{
  // Hunt for STX
  size_t p = 0;
  while (p < n && in[p] != STX) ++p;
  if (p >= n) { used = n; return false; }       // consumed all
  if (n - p < 6) { used = p; return false; }    // not enough for hdr+crc

  uint8_t len = in[p+3];
  size_t need = 4 + len + 2;
  if (n - p < need) { used = p; return false; } // incomplete

  uint16_t crc_calc = crc16_modbus(&in[p], 4 + len);
  uint16_t crc_rx   = (uint16_t)in[p+4+len] | ((uint16_t)in[p+5+len] << 8);
  if (crc_calc != crc_rx) { used = p + 1; return false; }

  addr = in[p+1]; cmd = in[p+2]; plen = len;
  if (len && payload) memcpy(payload, &in[p+4], len);
  used = p + need;
  return true;
}

// ------------ RX task (parses replies) ------------
void task_rx(void*) {
  uart_event_t ev;
  static uint8_t q[256]; size_t qn = 0;

  for (;;) {
    if (xQueueReceive(uart_queue, &ev, portMAX_DELAY)) {
      if (ev.type == UART_DATA && ev.size > 0) {
        uint8_t buf[256];
        size_t to_read = (ev.size < sizeof(buf)) ? ev.size : sizeof(buf);
        int n = uart_read_bytes(UART_PORT, buf, to_read, 0);
        // append to queue
        if (n > 0) {
          if (qn + (size_t)n > sizeof(q)) qn = 0; // simple overflow recovery
          memcpy(&q[qn], buf, n);
          qn += n;

          // parse as many frames as available
          size_t used = 0;
          while (true) {
            uint8_t addr, cmd, payload[64], plen = 0;
            size_t consumed = 0;
            bool ok = parseOneFrame(q, qn, consumed, addr, cmd, payload, plen);
            if (!ok) { // either incomplete or no STX; compact if we consumed some
              if (consumed && consumed < qn) memmove(q, q + consumed, qn - consumed), qn -= consumed;
              else if (consumed == qn) qn = 0;
              break;
            }
            // got a frame at start; consume it
            memmove(q, q + consumed, qn - consumed);
            qn -= consumed;

            if (addr == 0x00 /* master */ || addr == 0xFF /* broadcast */) {
              // ignore
            }

            if (cmd == CMD_LOC_RSP && plen == 6) {
              int64_t t2 = esp_timer_get_time(); // received timestamp
              // Decode payload
              int16_t x_in =  (int16_t)((uint16_t)payload[0] | ((uint16_t)payload[1] << 8));
              int16_t y_in =  (int16_t)((uint16_t)payload[2] | ((uint16_t)payload[3] << 8));
              uint16_t h_cdeg = (uint16_t)payload[4] | ((uint16_t)payload[5] << 8);

              // Print timestamps + deltas
              int64_t t0 = g_t0_req_begin_us;
              int64_t t1 = g_t1_req_sent_us;
              // Serial.printf("t0(before TX)=%lld us, t1(after TX)=%lld us, t2(recv)=%lld us\n",
              //               (long long)t0, (long long)t1, (long long)t2);
              uint32_t br = 0;
              uart_get_baudrate(UART_PORT, &br);
              LOGF("UART%u actual baud: %u\n", UART_PORT, br);
              LOGF("dur_tx=%lld us, resp_after_tx=%lld us, rtt=%lld us | LOC: x=%.2f in, y=%.2f in, heading=%.2f deg\n\n",
                            (long long)(t1 - t0), (long long)(t2 - t1), (long long)(t2 - t0),
                            x_in / 100.0f, y_in / 100.0f, h_cdeg / 100.0f);
            }
          }
        }
      }
      // TODO: handle UART_FIFO_OVF / UART_BUFFER_FULL etc if needed
    }
  }
}

// ------------ TX task (sends request periodically) ------------
void task_tx(void*) {
  const TickType_t period = pdMS_TO_TICKS(1000); // request every 1000 ms (adjust as needed)
  for (;;) {
    sendFrame(ADDR_SLAVE, CMD_LOC_REQ, nullptr, 0);
    vTaskDelay(period);
  }
}

// ------------ UART/RS485 init ------------
void rs485_init(uint32_t baud = 1000000) { // 1 Mbps
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
  ESP_ERROR_CHECK(uart_set_mode(UART_PORT, UART_MODE_RS485_HALF_DUPLEX)); // auto-DE mode
}

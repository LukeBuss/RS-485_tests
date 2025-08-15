#include <Arduino.h>
#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_idf_version.h"

// Choose a UART that Arduino isn't using. We'll use UART1 here.
static constexpr uart_port_t PORT  = UART_NUM_1;

// Your pins on the XIAO S3:
static constexpr int PIN_TX  = D7;  // to MAX3485 DI
static constexpr int PIN_RX  = D8;  // from MAX3485 RO
static constexpr int PIN_RTS = D9;  // to MAX3485 DE+!RE (tied together)

// Baud for the RS-485 bus
static constexpr int BAUD = 1000000; // 1 Mbps (go lower first if debugging)

void rs485_begin()
{
  // 1) Configure UART parameters
  uart_config_t cfg{};
  cfg.baud_rate  = BAUD;
  cfg.data_bits  = UART_DATA_8_BITS;
  cfg.parity     = UART_PARITY_DISABLE;
  cfg.stop_bits  = UART_STOP_BITS_1;
  cfg.flow_ctrl  = UART_HW_FLOWCTRL_DISABLE;

  #if defined(UART_SCLK_DEFAULT)
    cfg.source_clk = UART_SCLK_DEFAULT;    // IDF 5.x style
  #elif defined(UART_SCLK_APB)
    cfg.source_clk = UART_SCLK_APB;        // IDF 4.x style
  #else
    // Safe fallback: most chips default to APB when 0
    cfg.source_clk = (uart_sclk_t)0;
  #endif

  ESP_ERROR_CHECK(uart_param_config(PORT, &cfg));


  // 2) Map the pins (RTS will be used for DE/!RE)
  ESP_ERROR_CHECK(uart_set_pin(PORT, PIN_TX, PIN_RX, PIN_RTS, UART_PIN_NO_CHANGE));

  // 3) Install the driver (set RX/TX buffers; no event queue needed)
  const int RX_BUF = 512;
  const int TX_BUF = 512;
  ESP_ERROR_CHECK(uart_driver_install(PORT, RX_BUF, TX_BUF, 0, nullptr, 0));

  // 4) Enable RS-485 half-duplex mode (auto toggles RTS during TX)
  ESP_ERROR_CHECK(uart_set_mode(PORT, UART_MODE_RS485_HALF_DUPLEX));

  // Optional: set an RX idle timeout (counts of character times) to end frames
  ESP_ERROR_CHECK(uart_set_rx_timeout(PORT, 4));  // ~3.5 chars
}

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("\n[IDF RS485 demo]");

  rs485_begin();

#ifdef ROLE_TX
  Serial.println("Role: TX (sender)");
#else
  Serial.println("Role: RX (receiver)");
#endif
}

void loop() {
#ifdef ROLE_TX
  static uint16_t counter = 0;
  uint8_t frame[6] = { 0x55, 0xAA, uint8_t(counter>>8), uint8_t(counter), 'H', 'I' };
  // Just write; IDF toggles DE/!RE on RTS for you
  int n = uart_write_bytes(PORT, (const char*)frame, sizeof(frame));
  Serial.printf("TX #%u (%d bytes)\n", counter, n);
  counter++;
  delay(100);

#else   // ROLE_RX
  uint8_t buf[128];
  // Read whatever arrived; timeout 50 ms
  int n = uart_read_bytes(PORT, buf, sizeof(buf), pdMS_TO_TICKS(50));
  if (n > 0) {
    // Look for our simple 0x55 0xAA header
    for (int i = 0; i < n - 5; ++i) {
      if (buf[i] == 0x55 && buf[i+1] == 0xAA) {
        uint16_t cnt = (uint16_t(buf[i+2]) << 8) | buf[i+3];
        char c1 = (char)buf[i+4], c2 = (char)buf[i+5];
        Serial.printf("RX OK  counter=%u  text=%c%c\n", cnt, c1, c2);
        i += 5;
      }
    }
  }
#endif
}

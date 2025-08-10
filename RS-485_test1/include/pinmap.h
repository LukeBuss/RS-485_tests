#pragma once
#include <Arduino.h>

// -----------------------------------------------------------------------------
// Pin mappings per board (edit to match your wiring)
// For MAX3485/SN75176, DE and RE are usually tied together → one control pin.
// -----------------------------------------------------------------------------

// ===== Arduino Nano (ATmega328P) =====
#if defined(TARGET_NANO)
  // AltSoftSerial uses fixed pins: RX = 8, TX = 9
  #define DE_RE_PIN        2        // change to your DE/RE control pin
  // Serial port is owned inside the RS485 class (AltSoftSerial)

// ===== Seeed XIAO nRF52840 =====
#elif defined(TARGET_NRF) || defined(TARGET_NRF52840)
  // Choose the UART you’ve wired to the transceiver
  #define RS485_PORT       Serial1   // or Serial2 if you’ve wired that instead
  #define DE_RE_PIN        2         // DE (or DE+RE if tied)
  // If RE is separate from DE, also define RE_PIN and wire accordingly
  // #define RE_PIN        3

// ===== Seeed XIAO ESP32-S3 =====
#elif defined(ARDUINO_ARCH_ESP32) || defined(TARGET_ESP32)
  // UART selection
  #define RS485_PORT       Serial1
  // Optional: explicit pin mapping (uncomment and set to your wiring)
  // #define RS485_RX_PIN  44
  // #define RS485_TX_PIN  43
  // #define RS485_RTS     5     // RTS → DE for hardware RS485 (optional)

  // DE/RE control (use if you’re not using RTS hardware control)
  #define DE_RE_PIN        4
  // If RE is separate from DE, define it here; otherwise leave undefined
  // #define RE_PIN        6

// ===== Fallback / Unknown target =====
#else
  #define RS485_PORT       Serial1
  #define DE_RE_PIN        2
#endif

// -----------------------------------------------------------------------------
// Helper: initialize Serial pins on ESP32-S3 if RX/TX are defined here
// Call this near the top of setup() before bus.begin(...):
//   #if defined(ARDUINO_ARCH_ESP32) && defined(RS485_RX_PIN) && defined(RS485_TX_PIN)
//     RS485_PORT.begin(SERIAL_SPEED, SERIAL_8N1, RS485_RX_PIN, RS485_TX_PIN);
//   #else
//     RS485_PORT.begin(SERIAL_SPEED);
//   #endif
// If using hardware RS485, also call: bus.enableHardwareRS485(RS485_RTS);
// -----------------------------------------------------------------------------

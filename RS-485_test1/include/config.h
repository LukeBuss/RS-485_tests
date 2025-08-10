#pragma once

// ---- Build-wide configuration ----
// These can be overridden by -D flags in platformio.ini
#ifndef SERIAL_SPEED
  #define SERIAL_SPEED 9600UL   // default baud; override per env
#endif

// Optional: high-speed RS-485 baud hint. If not set, fall back to SERIAL_SPEED.
#ifndef RS485_BAUD
  #define RS485_BAUD SERIAL_SPEED //FIXME: I feel like this will lead to issues later where no default is set, but we don't know its changed to the default SERIAL_SPEED
#endif

// Protocol / framing
#ifndef RS485_MAX_FRAME
  #define RS485_MAX_FRAME 64        // max bytes we expect to RX
#endif
#ifndef RS485_SILENCE_CHARS
  #define RS485_SILENCE_CHARS 3.5f  // inter-character silence to end a frame
#endif

// Device role → human-readable name (derived from -D ROLE_* flags)
#if defined(ROLE_MASTER)
  #define ROLE_NAME "MASTER"
#elif defined(ROLE_SLAVE1)
  #define ROLE_NAME "SLAVE_1"
#elif defined(ROLE_SLAVE2)
  #define ROLE_NAME "SLAVE_2"
#else
  #define ROLE_NAME "UNKNOWN"
#endif

// Platform detection helpers (for conditional code / logging)
#if defined(ARDUINO_ARCH_ESP32) || defined(TARGET_ESP32)
  #define IS_ESP32 1
#else
  #define IS_ESP32 0
#endif

#if defined(TARGET_NRF) || defined(TARGET_NRF52840)
  #define IS_NRF 1
#else
  #define IS_NRF 0
#endif

#if defined(TARGET_NANO)
  #define IS_NANO 1
#else
  #define IS_NANO 0
#endif

// Feature toggles
#ifndef RS485_DEBUG
  #define RS485_DEBUG 1           // 1=enable debug prints via Serial
#endif

// IDs (set per device)
#ifndef SLAVE_ID
  #define SLAVE_ID 0x01
#endif

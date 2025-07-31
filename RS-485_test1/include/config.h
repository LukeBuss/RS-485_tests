#pragma once

// ===== Serial Speeds =====
#ifndef SERIAL_SPEED
  #define SERIAL_SPEED 9600   // fallback if not defined in platformio.ini
#endif

#define RS485_BAUD 38400

// ===== Role String Helper =====
#if defined(ROLE_MASTER)
  #define ROLE_NAME "MASTER"
#elif defined(ROLE_SLAVE1)
  #define ROLE_NAME "SLAVE_1"
#elif defined(ROLE_SLAVE2)
  #define ROLE_NAME "SLAVE_2"
#else
  #define ROLE_NAME "UNKNOWN"
#endif

// ===== Platform Detection =====
#if defined(TARGET_ESP32)
  #define IS_ESP32 true
#else
  #define IS_ESP32 false
#endif

#pragma once

// ===== Serial Speeds =====
#ifdef TARGET_ESP32S3
  #ifndef SERIAL_SPEED
    #define SERIAL_SPEED 115200 // Default for ESP32S3
  #endif
  #ifndef RS485_BAUD
    #define RS485_BAUD 1000000 //38400 57600 74880 115200 250000 500000 1000000
  #endif
#endif

#ifdef TARGET_NRF52840
  #ifndef SERIAL_SPEED
    #define SERIAL_SPEED 115200 // Default for nRF52840
  #endif
  #ifndef RS485_BAUD
    #define RS485_BAUD 115200 //38400 57600 74880 115200 250000 500000 1000000
  #endif
#endif

#ifdef TARGET_NANO
  #ifndef SERIAL_SPEED
    #define SERIAL_SPEED 9600   // fallback if not defined in platformio.ini
  #endif
  #ifndef RS485_BAUD
    #define RS485_BAUD 115200
  #endif
#endif

#ifdef ROLE_MASTER
  #define ROLE_NAME "MASTER"
#elif ROLE_SLAVE1
  #define ROLE_NAME "SLAVE_1"
#elif ROLE_SLAVE2
  #define ROLE_NAME "SLAVE_2"
#else
  #define ROLE_NAME "UNKNOWN"
#endif

// ===== Platform Detection =====
// #if defined(TARGET_ESP32)
//   #define IS_ESP32 true
// #else
//   #define IS_ESP32 false
// #endif

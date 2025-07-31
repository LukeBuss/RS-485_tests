#pragma once

// ESP32-specific pin assignments
#ifdef TARGET_ESP32

  #ifdef ROLE_MASTER
    #define DE_RE_PIN 27
    #define RS485_RX 16
    #define RS485_TX 17
    #define LED_BUILTIN 2
  #elif defined(ROLE_SLAVE1)
    #define DE_RE_PIN 26
    #define RS485_RX 18
    #define RS485_TX 19
    #define LED_BUILTIN 2
  #endif

#endif

// Nano-specific pin assignments
#ifdef TARGET_NANO

  #ifdef ROLE_MASTER
    #define DE_RE_PIN 4
    #define RS485_RX 10
    #define RS485_TX 11
    #define LED_BUILTIN 13
  #elif defined(ROLE_SLAVE1)
    #define DE_RE_PIN 5
    #define RS485_RX 10
    #define RS485_TX 11
    #define LED_BUILTIN 13
  #endif

#endif

#pragma once

// Nano-specific pin assignments
#ifdef TARGET_NANO
  #define DE_RE_PIN 5
  #define LED_BUILTIN 13
#endif

#ifdef TARGET_NRF52840
  // nRF52840 pin assignments
  // RS485_TX 6
  // RS485_RX 7
  #define DE_RE_PIN 8 // D8 drives DE & /RE (tied)
#endif

#ifdef TARGET_ESP32S3
  #define RS485_TX 7
  #define RS485_RX 8
  #define DE_RE_PIN 9
#endif
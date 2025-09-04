#pragma once
#include <RS485ModbusRTU.h>

// 0x03: sum N one-byte values
inline void handleFunction03_Add(RS485ModbusRTU& bus, const uint8_t* req, size_t len) {
  if (len < 3) return;
  const uint8_t count = req[2];
  if (len < (size_t)(3 + count)) return;

  uint16_t sum = 0;
  for (uint8_t i = 0; i < count; ++i) sum += req[3 + i];

  uint8_t reply[5] = {
    req[0], 0x03, 0x02,
    (uint8_t)(sum >> 8), (uint8_t)(sum & 0xFF)
  };
  bus.sendRequest(reply, sizeof(reply));
}

// 0x04: return (x, y, heading)
// You can wire in real sensors here; for now produce a simple synthetic pose.
inline void handleFunction04_Location(RS485ModbusRTU& bus, const uint8_t* req, size_t len) {
  (void)len;
  static int16_t x = -254;   // example units (cm*1)
  static int16_t y = -16;
  static uint16_t h = 35999; // heading in deg*100 (0..35999)

  // demo: tweak a little each call
  x += 1; if (x > 500) x = -500;
  h += 37; if (h >= 36000) h -= 36000;

  uint8_t reply[2 + 1 + 6] = {
    req[0], 0x04, 0x06,
    (uint8_t)(x >> 8), (uint8_t)(x & 0xFF),
    (uint8_t)(y >> 8), (uint8_t)(y & 0xFF),
    (uint8_t)(h >> 8), (uint8_t)(h & 0xFF)
  };
  bus.sendRequest(reply, sizeof(reply));
}

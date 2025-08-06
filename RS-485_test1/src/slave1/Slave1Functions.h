#pragma once
#include <RS485ModbusRTU.h>

// Handles function code 0x03: ADD N values
inline void handleFunction03_Add(RS485ModbusRTU& bus, const uint8_t* request, size_t length) {
  if (length < 3) return; // need at least [ID][Func][Count]
  uint8_t count = request[2];
  if (length < 3 + static_cast<size_t>(count)) return;
  
  uint16_t sum = 0;
  for (uint8_t i = 0; i < count; ++i) {
    sum += request[3 + i];
  }

  uint8_t reply[] = {
    request[0],      // Slave ID
    0x03,            // Function code
    0x02,            // Byte count
    (uint8_t)(sum >> 8),
    (uint8_t)(sum & 0xFF)
  };

  bus.sendRequest(reply, sizeof(reply));
}

inline void handleFunction04_Location(RS485ModbusRTU& bus, const uint8_t* request, size_t length) {
  if (length < 2) return;

  // Dummy location data (example values)
  uint16_t x = 25359;      // e.g., in millimeters
  uint16_t y = -1600;
  uint16_t heading = 35999; // e.g., in tenths of degrees

  uint8_t reply[] = {
    request[0],     // Slave ID
    0x04,           // Function code (same as request)
    0x06,           // Byte count (3 values × 2 bytes)

    static_cast<uint8_t>(x >> 8), static_cast<uint8_t>(x & 0xFF),
    static_cast<uint8_t>(y >> 8), static_cast<uint8_t>(y & 0xFF),
    static_cast<uint8_t>(heading >> 8), static_cast<uint8_t>(heading & 0xFF)
  };

  bus.sendRequest(reply, sizeof(reply));
}


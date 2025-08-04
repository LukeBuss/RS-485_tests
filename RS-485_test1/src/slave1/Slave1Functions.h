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

#pragma once
#include <RS485ModbusRTU.h>

// Example: send variable-length ADD request (function 0x03)
inline void sendAddCommand(RS485ModbusRTU& bus, uint8_t slaveID, const uint8_t* numbers, uint8_t count) {
  if (count == 0 || count > 61) return; // protect buffer size (max 64 total)

  uint8_t packet[64];
  packet[0] = slaveID;
  packet[1] = 0x03;       // Custom ADD function
  packet[2] = count;      // Number of arguments
  for (uint8_t i = 0; i < count; ++i) {
    packet[3 + i] = numbers[i];
  }

  bus.sendRequest(packet, 3 + count);
}

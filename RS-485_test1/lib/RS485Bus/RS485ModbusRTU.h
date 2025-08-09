// RS485ModbusRTU.h
#pragma once
#include <Arduino.h>
//#include <stdint.h>

// Fallback definitions for XAIO nRF52840 to recognize HIGH and LOW
#ifndef HIGH
  #define HIGH 0x1
#endif
#ifndef LOW
  #define LOW 0x0
#endif

#ifdef TARGET_NANO
  #include <AltSoftSerial.h>
  #define RS485Serial AltSoftSerial
  using rs485_byte   = uint8_t;
  using rs485_word16 = uint16_t;
#elif TARGET_NRF52840
  using rs485_byte   = std::uint8_t;
  using rs485_word16 = std::uint16_t;
#else
  #include <HardwareSerial.h>
  #define RS485Serial HardwareSerial
  using rs485_byte   = uint8_t;
  using rs485_word16 = uint16_t;
#endif

class RS485ModbusRTU {
public:
#ifdef TARGET_NANO
  RS485ModbusRTU(rs485_byte derePin);
#else
  RS485ModbusRTU(HardwareSerial& serialPort, rs485_byte derePin);
#endif

  void begin(unsigned long baud = 38400);
  void setDebug(Stream* debugStream);
  void enableDebug(bool enable = true) { debugEnabled = enable; }
  bool isDebugEnabled() const { return debugEnabled; }
  void printBytes(const rs485_byte* data, size_t len);

  void sendRequest(const rs485_byte* data, size_t len);
  size_t receiveResponse(rs485_byte* buffer, size_t maxLen);

  rs485_word16 computeCRC(const rs485_byte* data, size_t len);

private:
#ifdef TARGET_NANO
  RS485Serial serial;
#else
  HardwareSerial& serial;
#endif

  rs485_byte derePin;
  Stream* debugOut;
  bool debugEnabled = false;
  unsigned long charTimeMicros;

  void enableTransmit();
  void enableReceive();
};

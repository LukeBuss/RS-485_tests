// RS485ModbusRTU.h
#pragma once
#include <Arduino.h>

#ifdef TARGET_NANO
  #include <AltSoftSerial.h>
  #define RS485Serial AltSoftSerial
#endif

class RS485ModbusRTU {
public:
#ifdef TARGET_NANO
  RS485ModbusRTU(uint8_t derePin);
#else
  RS485ModbusRTU(HardwareSerial& serialPort, uint8_t derePin);
#endif

  void begin(unsigned long baud = 38400);
  void setDebug(Stream* debugStream);
  void enableDebug(bool enable = true) { debugEnabled = enable; }
  bool isDebugEnabled() const { return debugEnabled; }
  void printBytes(const uint8_t* data, size_t len);

  void sendRequest(const uint8_t* data, size_t len);
  size_t receiveResponse(uint8_t* packet, size_t maxLen);

  uint16_t computeCRC(const uint8_t* data, size_t len); // exposed for use outside

private:
#ifdef TARGET_NANO
  RS485Serial serial;
#else
  HardwareSerial& serial;
#endif

  uint8_t derePin;
  Stream* debugOut;
  bool debugEnabled = false;
  unsigned long charTimeMicros;

  void enableTransmit();
  void enableReceive();
};

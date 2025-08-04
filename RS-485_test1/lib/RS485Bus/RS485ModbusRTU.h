// RS485ModbusRTU.h
#pragma once
#include <Arduino.h>

#ifdef TARGET_NANO
  #include <SoftwareSerial.h>
#endif

class RS485ModbusRTU {
public:
#ifdef TARGET_NANO
  RS485ModbusRTU(uint8_t rx, uint8_t tx, uint8_t derePin);
#else
  RS485ModbusRTU(HardwareSerial& serialPort, uint8_t derePin);
#endif

  void begin(unsigned long baud = 38400);
  void setDebug(Stream* debugStream);

  void sendRequest(const uint8_t* data, size_t len);
  size_t receiveResponse(uint8_t* buffer, size_t maxLen);

  uint16_t computeCRC(const uint8_t* data, size_t len); // exposed for use outside

private:
#ifdef TARGET_NANO
  SoftwareSerial serial;
#else
  HardwareSerial& serial;
#endif

  uint8_t derePin;
  Stream* debugOut;
  unsigned long charTimeMicros;

  void enableTransmit();
  void enableReceive();
};

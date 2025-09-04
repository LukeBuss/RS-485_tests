#pragma once
#include <Arduino.h>

#ifdef TARGET_NANO
  #include <AltSoftSerial.h>
  #define RS485Serial AltSoftSerial
#endif

class RS485ModbusRTU {
public:
#ifdef TARGET_NANO
  explicit RS485ModbusRTU(uint8_t derePin);
#else
  RS485ModbusRTU(HardwareSerial& serialPort, uint8_t derePin);
#endif

  void begin(unsigned long baud);

  // Optional debugging to Serial, etc.
  void setDebug(Stream* s) { debugOut = s; }
  void enableDebug(bool en) { debugEnabled = en; }

  // TX a packet (raw bytes, no CRC by design in your project)
  void sendRequest(const uint8_t* data, size_t len);

  // RX until 3.5 character times of silence or timeout (ms)
  size_t receiveResponse(uint8_t* buffer, size_t maxlen, unsigned long timeoutMs = 50);

  void printBytes(const uint8_t* data, size_t len);

private:
#ifdef TARGET_NANO
  RS485Serial serial;
#else
  HardwareSerial& serial;
#endif

  uint8_t derePin;
  Stream*  debugOut = nullptr;
  bool     debugEnabled = false;
  unsigned long charTimeMicros = 0;

  void enableTransmit();
  void enableReceive();
};

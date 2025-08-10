#ifndef RS485MODBUSRTU_H
#define RS485MODBUSRTU_H

#include <Arduino.h>
#include <stdint.h>

// IntelliSense/editor fallbacks (safe when Arduino.h fails to load)
#ifndef HIGH
  #define HIGH 0x1
#endif
#ifndef LOW
  #define LOW  0x0
#endif

// --- Platform type aliases ---------------------------------------------------
#ifdef TARGET_NANO
  #include <AltSoftSerial.h>
  #define RS485Serial AltSoftSerial
  using rs485_byte   = uint8_t;
  using rs485_word16 = uint16_t;
#elif defined(TARGET_NRF) || defined(TARGET_NRF52840)
  using rs485_byte   = std::uint8_t;
  using rs485_word16 = std::uint16_t;
#elif defined(ARDUINO_ARCH_ESP32) || defined(TARGET_ESP32)
  using rs485_byte   = uint8_t;
  using rs485_word16 = uint16_t;
#else
  using rs485_byte   = uint8_t;
  using rs485_word16 = uint16_t;
#endif

// --- Class -------------------------------------------------------------------
class RS485ModbusRTU {
public:
#ifdef TARGET_NANO
  explicit RS485ModbusRTU(rs485_byte derePin);
#else
  RS485ModbusRTU(HardwareSerial& serialPort, rs485_byte derePin);
#endif

  void begin(unsigned long baud = 38400);

  // ESP32-S3 optional: enable true hardware RS485 (auto-DE via RTS)
  // Returns true if enabled; safe no-op (returns false) on non-ESP32.
  bool enableHardwareRS485(int rtsPin);

  void setDebug(Stream* debugStream);
  void enableDebug(bool enable = true) { debugEnabled = enable; }
  bool isDebugEnabled() const { return debugEnabled; }
  void printBytes(const rs485_byte* data, size_t len);

  void sendRequest(const rs485_byte* data, size_t len);
  size_t receiveResponse(rs485_byte* buffer, size_t maxLen);

  rs485_word16 computeCRC(const rs485_byte* data, size_t len);

private:
#ifdef TARGET_NANO
  RS485Serial serial;             // Own AltSoftSerial instance (Nano)
#else
  HardwareSerial& serial;         // HW UART reference (nRF / ESP32)
#endif

  rs485_byte derePin;             // DE (or DE+RE if tied)
  Stream* debugOut = nullptr;
  bool debugEnabled = false;
  unsigned long charTimeMicros = 0; // ≈ 11 bits / baud

  void enableTransmit();
  void enableReceive();
};

#endif // RS485MODBUSRTU_H

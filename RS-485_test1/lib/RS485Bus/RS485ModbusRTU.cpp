// RS485ModbusRTU.cpp
#include "RS485ModbusRTU.h"

#ifdef TARGET_NANO
RS485ModbusRTU::RS485ModbusRTU(uint8_t rx, uint8_t tx, uint8_t derePin)
  : serial(rx, tx), derePin(derePin), debugOut(nullptr) {}
#else
RS485ModbusRTU::RS485ModbusRTU(HardwareSerial& serialPort, uint8_t derePin)
  : serial(serialPort), derePin(derePin), debugOut(nullptr) {}
#endif

void RS485ModbusRTU::begin(unsigned long baud) {
  pinMode(derePin, OUTPUT);
  digitalWrite(derePin, LOW);
  serial.begin(baud);
  charTimeMicros = (1000000UL * 11) / baud; // 11 bits per character
}

void RS485ModbusRTU::setDebug(Stream* debugStream) {
  debugOut = debugStream;
}

void RS485ModbusRTU::enableTransmit() {
  digitalWrite(derePin, HIGH);
  delayMicroseconds(100);
}

void RS485ModbusRTU::enableReceive() {
  delayMicroseconds(100);
  digitalWrite(derePin, LOW);
}

uint16_t RS485ModbusRTU::computeCRC(const uint8_t* data, size_t len) {
  uint16_t crc = 0xFFFF;
  for (size_t i = 0; i < len; ++i) {
    crc ^= data[i];
    for (int j = 0; j < 8; ++j) {
      if (crc & 0x0001)
        crc = (crc >> 1) ^ 0xA001;
      else
        crc >>= 1;
    }
  }
  return crc;
}

void RS485ModbusRTU::sendRequest(const uint8_t* data, size_t len) {
  enableTransmit();
  delayMicroseconds(100);

  uint16_t crc = computeCRC(data, len);
  serial.write(data, len);
  serial.write(crc & 0xFF);
  serial.write((crc >> 8) & 0xFF);
  serial.flush();

  enableReceive();

  if (debugOut) {
    debugOut->print(F("[ModbusTX] "));
    for (size_t i = 0; i < len; ++i) {
      debugOut->print("0x"); debugOut->print(data[i], HEX); debugOut->print(" ");
    }
    debugOut->print("CRC=0x"); debugOut->println(crc, HEX);
  }
}

size_t RS485ModbusRTU::receiveResponse(uint8_t* buffer, size_t maxLen) {
  size_t count = 0;
  unsigned long lastByteTime = millis();
  unsigned long timeout = millis();

  while ((millis() - timeout < 100) && count < maxLen) {
    if (serial.available()) {
      buffer[count++] = serial.read();
      lastByteTime = millis();
    } else if (millis() - lastByteTime > (charTimeMicros * 3.5) / 1000) {
      break; // silent interval
    }
  }

  if (debugOut && count > 0) {
    debugOut->print(F("[ModbusRX] "));
    for (size_t i = 0; i < count; ++i) {
      debugOut->print("0x"); debugOut->print(buffer[i], HEX); debugOut->print(" ");
    }
    debugOut->println();
  }

  return count;
}

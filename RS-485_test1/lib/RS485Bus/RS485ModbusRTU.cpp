// RS485ModbusRTU.cpp
#include "RS485ModbusRTU.h"

#ifdef TARGET_NANO
RS485ModbusRTU::RS485ModbusRTU(rs485_byte derePin)
  : derePin(derePin), debugOut(nullptr) {}
#else
RS485ModbusRTU::RS485ModbusRTU(HardwareSerial& serialPort, rs485_byte derePin)
  : serial(serialPort), derePin(derePin), debugOut(nullptr) {}
#endif

void RS485ModbusRTU::begin(unsigned long baud) {
  pinMode(derePin, OUTPUT);
  enableReceive();

#ifdef TARGET_NANO
  serial.begin(baud);   // AltSoftSerial (fixed pins on Nano)
#else
  serial.begin(baud);   // HardwareSerial on nRF / others
#endif

  // ≈ 11 bits per character (start + 8 data + stop + cushion)
  charTimeMicros = (11UL * 1000000UL) / baud;
}

void RS485ModbusRTU::setDebug(Stream* debugStream) {
  debugOut = debugStream;
}

void RS485ModbusRTU::printBytes(const rs485_byte* data, size_t len) {
  if (!debugEnabled || !debugOut) return;
  for (size_t i = 0; i < len; i++) {
    debugOut->print(F("0x"));
    if (data[i] < 0x10) debugOut->print('0');
    debugOut->print(data[i], HEX);
    debugOut->print(' ');
  }
}

void RS485ModbusRTU::enableTransmit() {
  digitalWrite(derePin, HIGH);
  // small lead time so the first start bit isn't truncated
  delayMicroseconds(charTimeMicros / 4);
}

void RS485ModbusRTU::enableReceive() {
  // small tail time to ensure last stop bit clears the line
  delayMicroseconds(charTimeMicros);
  digitalWrite(derePin, LOW);
}

rs485_word16 RS485ModbusRTU::computeCRC(const rs485_byte* data, size_t len) {
  rs485_word16 crc = 0xFFFF;
  for (size_t i = 0; i < len; ++i) {
    crc ^= data[i];
    for (int j = 0; j < 8; ++j) {
      if (crc & 0x0001) crc = (crc >> 1) ^ 0xA001;
      else              crc >>= 1;
    }
  }
  return crc;
}

void RS485ModbusRTU::sendRequest(const rs485_byte* data, size_t len) {
  enableTransmit();

  const rs485_word16 crc = computeCRC(data, len);
  serial.write(data, len);
  serial.write((rs485_byte)(crc & 0xFF));        // CRC low
  serial.write((rs485_byte)((crc >> 8) & 0xFF)); // CRC high
  serial.flush();                                // wait for TX to fully shift

  enableReceive();

  if (debugEnabled && debugOut) {
    debugOut->print(F("[ModbusTX] "));
    printBytes(data, len);
    debugOut->print(F("CRC=0x"));
    debugOut->println(crc, HEX);
  }
}

size_t RS485ModbusRTU::receiveResponse(rs485_byte* buffer, size_t maxLen) {
  size_t count = 0;
  unsigned long lastByteTime = micros();
  const unsigned long overallStart = micros();
  const unsigned long overallTimeout = 10000UL; // ~10 ms total window

  while ((micros() - overallStart) < overallTimeout && count < maxLen) {
    if (serial.available()) {
      int b = serial.read();
      if (b >= 0) {
        buffer[count++] = (rs485_byte)b;
        lastByteTime = micros();
      }
    } else {
      // break after 3.5 character times of silence
      if ((micros() - lastByteTime) > (unsigned long)(charTimeMicros * 3.5f)) {
        break;
      }
    }
  }

  if (debugEnabled && debugOut && count > 0) {
    debugOut->print(F("[ModbusRX] "));
    printBytes(buffer, count);
    debugOut->println();
  }

  return count;
}

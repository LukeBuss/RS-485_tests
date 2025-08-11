// RS485ModbusRTU.cpp
#include "RS485ModbusRTU.h"

#ifdef TARGET_NANO
RS485ModbusRTU::RS485ModbusRTU(uint8_t derePin)
  : derePin(derePin), debugOut(nullptr) {}
#else
RS485ModbusRTU::RS485ModbusRTU(HardwareSerial& serialPort, uint8_t derePin)
  : serial(serialPort), derePin(derePin), debugOut(nullptr) {}
#endif

void RS485ModbusRTU::begin(unsigned long baud) {
  pinMode(derePin, OUTPUT);
  enableReceive();

#ifdef TARGET_NANO
  serial.begin(baud);  // AltSoftSerial fixed pins
#else
  serial.begin(baud);
#endif

  // 1 character time in microseconds = 11 bits / baud rate * 1e6
  charTimeMicros = (11UL * 1000000UL) / baud;
}


void RS485ModbusRTU::setDebug(Stream* debugStream) {
  if (!debugStream || !debugEnabled) {
    debugEnabled = false;
    debugOut = nullptr;
    return;
  }
  debugOut = debugStream;
}

void RS485ModbusRTU::printBytes(const uint8_t* data, size_t len) {
  if (!debugEnabled || !debugOut) return;
  for (size_t i = 0; i < len; i++) {
    debugOut->print("0x");
    if (data[i] < 0x10) debugOut->print("0");
    debugOut->print(data[i], HEX);
    debugOut->print(" ");
  }
}


void RS485ModbusRTU::enableTransmit() {
  digitalWrite(derePin, HIGH);
  delayMicroseconds(charTimeMicros);  // Optional safety delay
}

void RS485ModbusRTU::enableReceive() {
  delayMicroseconds(charTimeMicros);  // Optional flush delay
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
  //delayMicroseconds(100);

  uint16_t crc = computeCRC(data, len);
  serial.write(data, len);
  serial.write(crc & 0xFF);
  serial.write((crc >> 8) & 0xFF);
  serial.flush();

  enableReceive();

  if (debugEnabled && debugOut) {
    debugOut->print(F("[ModbusTX] "));
    printBytes(data, len);
    debugOut->print("CRC=0x"); debugOut->println(crc, HEX);
  }
}

size_t RS485ModbusRTU::receiveResponse(uint8_t* packet, size_t maxLen) {
  size_t count = 0;
  unsigned long lastByteTime = micros();
  unsigned long timeout = micros();

  while ((micros() - timeout < 10000) && count < maxLen) {
    if (serial.available()) {
      packet[count++] = serial.read();
      lastByteTime = micros();
    } else if (micros() - lastByteTime > (charTimeMicros * 3.5)) {
      break; // silent interval
    }
  }

  if (debugEnabled && debugOut && count > 0) {
    debugOut->print(F("[ModbusRX] "));
    printBytes(packet, count);
    debugOut->println();
  }

  return count;
}

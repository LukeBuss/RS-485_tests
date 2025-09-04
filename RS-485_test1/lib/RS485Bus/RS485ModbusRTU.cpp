#include "RS485ModbusRTU.h"
#include "pinmap.h"
#include "config.h"

#ifdef TARGET_NANO
RS485ModbusRTU::RS485ModbusRTU(uint8_t derePin)
: derePin(derePin) {}
#else
RS485ModbusRTU::RS485ModbusRTU(HardwareSerial& serialPort, uint8_t derePin)
: serial(serialPort), derePin(derePin) {}
#endif

void RS485ModbusRTU::begin(unsigned long baud) {
  pinMode(derePin, OUTPUT);
  enableReceive();

#ifdef TARGET_NANO
  serial.begin(baud);
#else
  // ESP32-S3: map RX/TX pins explicitly (from pinmap.h)
  serial.begin(baud, SERIAL_8N1, RS485_RX, RS485_TX);
#endif

  // ~11 bits per char @ N,8,1 -> compute character time
  charTimeMicros = (unsigned long)((11.0f * 1e6f) / (float)baud);
}

void RS485ModbusRTU::sendRequest(const uint8_t* data, size_t len) {
  if (!data || len == 0) return;

  if (debugEnabled && debugOut) {
    debugOut->print(F("[ModbusTX] "));
    printBytes(data, len);
    debugOut->println();
  }

  enableTransmit();
  serial.write(data, len);
  serial.flush();                       // wait for TX FIFO to drain

  // On ESP32 Arduino, flush() waits for buffer empty; add ~1 char to clear shifter
  delayMicroseconds(charTimeMicros + 2);

  enableReceive();
}

size_t RS485ModbusRTU::receiveResponse(uint8_t* buffer, size_t maxlen, unsigned long timeoutMs) {
  if (!buffer || maxlen == 0) return 0;

  const unsigned long start = millis();
  size_t count = 0;

  // Wait for the first byte or timeout
  while ((millis() - start) < timeoutMs) {
    if (serial.available()) break;
    delayMicroseconds(50);
  }
  if (!serial.available()) return 0; // timed out

  // We got at least one byte. Now read until 3.5 char-times of silence.
  unsigned long lastByteTime = micros();
  const unsigned long silentGap = (unsigned long)(charTimeMicros * 3.5f);

  while (true) {
    while (serial.available()) {
      int b = serial.read();
      if (b < 0) break;
      if (count < maxlen) buffer[count++] = (uint8_t)b;
      lastByteTime = micros();
    }

    // Check for silent interval to end frame
    if ((micros() - lastByteTime) > silentGap) break;

    // Also guard overall timeout in case of noise
    if ((millis() - start) > timeoutMs) break;
  }

  if (debugEnabled && debugOut && count > 0) {
    debugOut->print(F("[ModbusRX] "));
    printBytes(buffer, count);
    debugOut->println();
  }

  return count;
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
  digitalWrite(derePin, LOW);   // DE high, /RE high -> driver on, receiver off
  // allow driver enable time (~1/4 char time is plenty)
  delayMicroseconds(charTimeMicros / 4 + 2);
}

void RS485ModbusRTU::enableReceive() {
  digitalWrite(derePin, HIGH);    // DE low, /RE low -> receiver on, driver off
  // small guard time to ensure bus release
  delayMicroseconds(charTimeMicros / 8 + 2);
}

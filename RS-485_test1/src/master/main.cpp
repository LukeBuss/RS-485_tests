// src/master/main.cpp
#include <Arduino.h>
#include <RS485ModbusRTU.h>
#include "config.h"
#include "pinmap.h"
#include "MasterFunctions.h"

#ifdef TARGET_NANO
  RS485ModbusRTU bus(DE_RE_PIN);
#elif TARGET_NRF52840
  RS485ModbusRTU bus(Serial1, DE_RE_PIN);
  #include <Adafruit_TinyUSB.h>
#endif

unsigned long lastSendTime = 0;
unsigned long start = micros();
unsigned long totalTime = micros();

void printTiming(unsigned long t1, unsigned long t2, unsigned long t3) {
  Serial.print("Send time: ");
  Serial.print(t2 - t1);
  Serial.println(" µs");
  Serial.print("Receive time: ");
  Serial.print(t3 - t2);
  Serial.println(" µs");
  Serial.print("Total time: ");
  Serial.print(t3 - t1);
  Serial.println(" µs");
}

void setup() {
  Serial.begin(SERIAL_SPEED);
#ifdef TARGET_NRF52840
  while (!Serial) { delay(10); } // let USB enumerate
#endif

#ifdef TARGET_NRF52840
  Serial1.begin(RS485_BAUD); // For nRF52840, Serial1 is used
  // while (!Serial1) { delay(10); } // let USB enumerate
// #elif TARGET_NANO
  // AltSoftSerial is started inside bus.begin(...)
#endif

  bus.begin(RS485_BAUD);
  bus.setDebug(&Serial);
  bus.enableDebug(true);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_GREEN, LOW); // Start with LED on

  Serial.println("Master ready: sending ADD commands over RS485 Modbus RTU");
}

void loop() {
  // uint8_t values[16];
  // for (uint8_t i = 0; i < 16; ++i) {
  //   values[i] = 1; // or any number
  // }

  // Step timing variables
  unsigned long t1 = micros();

  // sendAddCommand(bus, 0x01, values, sizeof(values));
  sendRequestLocation(bus, 0x01);

  unsigned long t2 = micros();  // after send

  // uint8_t response[16];
  // size_t len = bus.receiveResponse(response, sizeof(response));

  uint8_t response[64];
  size_t len = bus.receiveResponse(response, sizeof(response));


  unsigned long t3 = micros();  // after receive

  if (len > 0) {
    // if (len >= 5 && response[0] == 0x01 && response[1] == 0x03 && response[2] == 0x02) {
    //   uint16_t sum = (response[3] << 8) | response[4];
    //   Serial.print("Received sum from slave: ");
    //   Serial.println(sum);

    if (len >= 9 && response[0] == 0x01 && response[1] == 0x04 && response[2] == 0x06) {
      uint16_t x = (response[3] << 8) | response[4];
      uint16_t y = (response[5] << 8) | response[6];
      uint16_t heading = (response[7] << 8) | response[8];

      Serial.print("X: "); Serial.print(x);
      Serial.print("  Y: "); Serial.print(y);
      Serial.print("  Heading: "); Serial.println(heading);

      printTiming(t1, t2, t3);
    } else {
      Serial.println("Invalid response:");
      bus.printBytes(response, len);
    }
  } else {
    Serial.println("No response or timeout");
  }

  Serial.println();

  delay(1000);
}
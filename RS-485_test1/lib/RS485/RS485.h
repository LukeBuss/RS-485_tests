#pragma once
#include <Arduino.h>

class RS485 {
public:
    RS485(HardwareSerial& serial, uint8_t dePin, uint8_t rePin, unsigned long baudrate = 9600);

    void rs485_init(uint32_t baud = 1000000);
    void task_rx(void* arg);
    void task_tx(void* arg);

    bool rs485_parse_frame(uint8_t* in, size_t n, size_t* used,
                        uint8_t* addr, uint8_t* cmd,
                        uint8_t* payload, uint8_t* plen);
    void rs485_send_frame(uint8_t addr, uint8_t cmd,
                        const uint8_t* payload, uint8_t plen);

    void begin();
    void end();

    size_t write(uint8_t data);
    size_t write(const uint8_t *buffer, size_t size);

    int read();
    int available();
    void flush();

    void setBaudrate(unsigned long baudrate);

private:
    HardwareSerial& _serial;
    uint8_t _dePin;
    uint8_t _rePin;
    unsigned long _baudrate;

    void enableTransmit();
    void enableReceive();
};
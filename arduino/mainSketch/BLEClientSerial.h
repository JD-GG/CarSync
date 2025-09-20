#ifndef _BLE_CLIENT_SERIAL_H_
#define _BLE_CLIENT_SERIAL_H_

#include "Arduino.h"
#include "Stream.h"
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include "secrets.h"

class BLEClientSerial {
public:
    BLEClientSerial();
    ~BLEClientSerial();

    bool begin(char *localName);
    bool connect(void);
    int available(void);
    int peek(void);
    int read(void);
    size_t write(uint8_t c);
    size_t write(const uint8_t *buffer, size_t size);
    void flush();
    void end();

    // static instance pointer (needed for callbacks)
    static BLEClientSerial* instance;

private:
    BLERemoteCharacteristic* pRxCharacteristic;
    BLERemoteCharacteristic* pTxCharacteristic;
};


#endif
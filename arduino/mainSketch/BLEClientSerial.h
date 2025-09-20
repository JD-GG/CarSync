#ifndef BLEClientSerial_h
#define BLEClientSerial_h

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <BLE2902.h>
#include "secrets.h"
#include <Stream.h>

class BLEClientSerial : public Stream {
public:
    BLEClientSerial();
    ~BLEClientSerial();
    
    bool begin(char *localName);
    bool connect(void);
    
    // Stream interface implementation - remove the duplicate declarations
    int available() override;
    int read() override;
    int peek() override;
    void flush() override;
    
    size_t write(uint8_t data) override;
    size_t write(const uint8_t *buffer, size_t size) override;
    
    void end();

    String readLine();

private:
    // No private members needed for this simple implementation
};

#endif
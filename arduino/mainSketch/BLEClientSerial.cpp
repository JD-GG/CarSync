#include "BLEClientSerial.h"
#include <Arduino.h>

static boolean doConnect = false;
static boolean connected = false;
static boolean doScan = false;
static std::string staticBuffer = "";

std::string targetDeviceName = OBD_NAME;
BLEUUID serviceUUID_FFF0("FFF0"); 
BLEUUID rxUUID("FFF1");
BLEUUID txUUID("FFF2");

static BLEAdvertisedDevice *myDevice;
static BLERemoteCharacteristic* pTxCharacteristic = nullptr;
static BLERemoteCharacteristic* pRxCharacteristic = nullptr;

static void printFriendlyResponse(uint8_t *pData, size_t length) {
    for (int i = 0; i < length; i++) {
        char recChar = (char)pData[i];
        if (recChar == '\f') Serial.print(F("\\f"));
        else if (recChar == '\n') Serial.print(F("\\n"));
        else if (recChar == '\r') Serial.print(F("\\r"));
        else if (recChar == '\t') Serial.print(F("\\t"));
        else if (recChar == '\v') Serial.print(F("\\v"));
        else if (recChar == ' ') Serial.print(F("_"));
        else Serial.print(recChar);
    }
    Serial.println("");
}

static void notifyCallback(BLERemoteCharacteristic *pBLERemoteCharacteristic,
                          uint8_t *pData, size_t length, bool isNotify) {   
    Serial.print("[DEBUG] ELM RESPONSE > ");
    printFriendlyResponse(pData, length);

    // Store received data
    std::string receivedData((char*)pData, length);
    staticBuffer += receivedData;
    
    // Prevent buffer from growing too large
    if (staticBuffer.length() > 200) {
        staticBuffer = staticBuffer.substr(staticBuffer.length() - 100);
    }
}

class MyClientCallback : public BLEClientCallbacks {
    void onConnect(BLEClient *pclient) { connected = true; }
    void onDisconnect(BLEClient *pclient) { connected = false; }
};

class MySecurity : public BLESecurityCallbacks {
    uint32_t onPassKeyRequest() { return 123456; }
    void onPassKeyNotify(uint32_t pass_key) { Serial.printf("Passkey Notify: %d", pass_key); }
    bool onConfirmPIN(uint32_t pass_key) { Serial.printf("Passkey YES/NO: %d", pass_key); return true; }
    bool onSecurityRequest() { Serial.printf("Security Request"); return true; }
    void onAuthenticationComplete(esp_ble_auth_cmpl_t auth_cmpl) {
        Serial.printf("Pair status = %s", auth_cmpl.success ? "success" : "fail");
    }
};

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
        if (advertisedDevice.getName() == targetDeviceName) {
            Serial.print(targetDeviceName.c_str());
            Serial.println(" found.");
            BLEDevice::getScan()->stop();
            myDevice = new BLEAdvertisedDevice(advertisedDevice);
            doConnect = true;
            doScan = true;
        }
    }
};

BLEClientSerial::BLEClientSerial() {}
BLEClientSerial::~BLEClientSerial(void) {}

bool BLEClientSerial::begin(char *localName) {
    targetDeviceName = localName;
    BLEDevice::init("");
    BLEScan* pBLEScan = BLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    pBLEScan->setInterval(1349);
    pBLEScan->setWindow(449);
    pBLEScan->setActiveScan(true);
    pBLEScan->start(5, false);
    return true;
}

int BLEClientSerial::available() { return staticBuffer.length(); }
int BLEClientSerial::peek() { return (staticBuffer.length() > 0) ? staticBuffer[0] : -1; }

bool BLEClientSerial::connect(void) {
    if (myDevice == nullptr) {
        Serial.println("Error: No BLE device found!");
        return false;
    }

    BLEDevice::setEncryptionLevel(ESP_BLE_SEC_ENCRYPT);
    BLEDevice::setSecurityCallbacks(new MySecurity());

    BLESecurity *pSecurity = new BLESecurity();
    pSecurity->setKeySize();
    pSecurity->setStaticPIN(123456);
    pSecurity->setAuthenticationMode(ESP_LE_AUTH_BOND);
    pSecurity->setCapability(ESP_IO_CAP_NONE);

    BLEClient *pClient = BLEDevice::createClient();
    pClient->setClientCallbacks(new MyClientCallback());
    pClient->connect(myDevice);

    BLERemoteService *pService = pClient->getService(serviceUUID_FFF0);
    if (pService) {
        pRxCharacteristic = pService->getCharacteristic(rxUUID);
        pTxCharacteristic = pService->getCharacteristic(txUUID);

        if (pRxCharacteristic->canNotify()) {
            pRxCharacteristic->registerForNotify(notifyCallback, true);
        }
    }
    
    return true;
}

int BLEClientSerial::read() {
    if (staticBuffer.length() > 0) {
        uint8_t c = staticBuffer[0];
        staticBuffer.erase(0, 1);
        return c;
    }
    return -1;
}

size_t BLEClientSerial::write(uint8_t data) {
    if (pTxCharacteristic) {
        pTxCharacteristic->writeValue(&data, 1, true);
        delay(10);
    }
    return 1;
}

size_t BLEClientSerial::write(const uint8_t *buffer, size_t size) {
    if (!connected || !pTxCharacteristic) {
        return 0;
    }
    
    // Use const_cast to handle the const/non-const mismatch
    for (size_t i = 0; i < size; i++) {
        pTxCharacteristic->writeValue(const_cast<uint8_t*>(&buffer[i]), 1, false);
    }
    
    return size;
}

String BLEClientSerial::readLine() {
    String line = "";
    unsigned long startTime = millis();
    
    while (millis() - startTime < 1000) { // 1 second timeout
        while (available() > 0) {
            char c = read();
            if (c == '\r' || c == '\n') {
                if (line.length() > 0) {
                    return line; // Return the line when we hit carriage return
                }
            } else {
                line += c;
            }
        }
        delay(10);
    }
    return line;
}

void BLEClientSerial::flush() { staticBuffer.clear(); }
void BLEClientSerial::end() {}
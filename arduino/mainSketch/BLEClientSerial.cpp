#include "BLEClientSerial.h"

static boolean doConnect = false;
static boolean connected = false;
static boolean doScan = false;
static std::string staticBuffer = "";

std::string targetDeviceName = OBD_NAME;
BLEUUID serviceUUID_FFF0("FFF0"); 
BLEUUID rxUUID("FFF1");
BLEUUID txUUID("FFF2");

static BLEAdvertisedDevice *myDevice;
static std::string elmInputBuffer;

// Add a static instance pointer inside the library
BLEClientSerial* BLEClientSerial::instance = nullptr;

static void printFriendlyResponse(uint8_t *pData, size_t length)
{
    Serial.print("");
    for (int i = 0; i < length; i++)
    {
        char recChar = (char)pData[i];
        if (recChar == '\f')
            Serial.print(F("\\f"));
        else if (recChar == '\n')
            Serial.print(F("\\n"));
        else if (recChar == '\r')
            Serial.print(F("\\r"));
        else if (recChar == '\t')
            Serial.print(F("\\t"));
        else if (recChar == '\v')
            Serial.print(F("\\v"));
        else if (recChar == ' ')
            Serial.print(F("_"));
        else
            Serial.print(recChar);
    }
    Serial.println("");
}

// notify callback
static void notifyCallback(
    BLERemoteCharacteristic* pBLERemoteCharacteristic,
    uint8_t* pData, size_t length,
    bool isNotify)
{
    for (size_t i = 0; i < length; i++) {
        char c = (char)pData[i];
        Serial.print(c); // debug
        staticBuffer.push_back(c);  // <- feed into RX buffer for ELMduino
    }
}


class MyClientCallback : public BLEClientCallbacks
{
    void onConnect(BLEClient *pclient) { connected = true; }
    void onDisconnect(BLEClient *pclient) { connected = false; }
};

class MySecurity : public BLESecurityCallbacks
{
    uint32_t onPassKeyRequest() { return 123456; }
    void onPassKeyNotify(uint32_t pass_key) { Serial.printf("The passkey Notify number: %d", pass_key); }
    bool onConfirmPIN(uint32_t pass_key) { Serial.printf("The passkey YES/NO number: %d", pass_key); vTaskDelay(5000); return true; }
    bool onSecurityRequest() { Serial.printf("Security Request"); return true; }
    void onAuthenticationComplete(esp_ble_auth_cmpl_t auth_cmpl)
    {
        if (auth_cmpl.success) {
            Serial.printf("remote BD_ADDR: ");
            Serial.printf("address type = %d ", auth_cmpl.addr_type);
        }
        Serial.printf("pair status = %s ", auth_cmpl.success ? "success" : "fail");
    }
};

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
    void onResult(BLEAdvertisedDevice advertisedDevice)
    {
        if (advertisedDevice.getName().c_str() == targetDeviceName)
        {
            Serial.print(targetDeviceName.c_str());
            Serial.println(" found.");
            BLEDevice::getScan()->stop();
            myDevice = new BLEAdvertisedDevice(advertisedDevice);
            doConnect = true;
            doScan = true;
        }
    }
};

// Constructor
BLEClientSerial::BLEClientSerial() {
    instance = this; // store pointer for callbacks
}

// Destructor
BLEClientSerial::~BLEClientSerial(void) {
}

// Begin bluetooth serial
bool BLEClientSerial::begin(char *localName)
{
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

int BLEClientSerial::available(void)
{
    return staticBuffer.length();
}

int BLEClientSerial::peek(void)
{
    if ((staticBuffer.length() > 0)) return staticBuffer[0];
    return -1;
}

bool BLEClientSerial::connect(void)
{
    Serial.println("Forming a connection to ");
    Serial.println("Forming a connection to ");
    if (myDevice == nullptr) {
        Serial.println("Error: No BLE device found! Make sure OBD is turned on.");
        return false;
    }
    Serial.println(myDevice->getAddress().toString().c_str());

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

    std::map<std::string, BLERemoteService *> *pRemoteServices = pClient->getServices();
    if (pRemoteServices == nullptr) Serial.println(" - No services");

    BLERemoteService *pService = pClient->getService(serviceUUID_FFF0);
    if (pService)
    {
        Serial.println("[DEBUG] Service FFF0 found.");

        pRxCharacteristic = pService->getCharacteristic(rxUUID);
        Serial.println("[DEBUG] CHAR rxUUID found.");
        Serial.print("[DEBUG] canNotify ");
        Serial.println(pRxCharacteristic->canNotify());

        pTxCharacteristic = pService->getCharacteristic(txUUID);
        Serial.println("[DEBUG] CHAR txUUID found.");
        Serial.print("[DEBUG] canWrite ");
        Serial.println(pTxCharacteristic->canWrite());

        if (pRxCharacteristic->canNotify()) {
            Serial.println("[DEBUG] RX subscribed");
            pRxCharacteristic->registerForNotify(notifyCallback, true);
        }
    }
    return true;
}

int BLEClientSerial::read(void)
{
    if ((staticBuffer.length() > 0))
    {
        uint8_t c = staticBuffer[0];
        staticBuffer.erase(0, 1);
        return c;
    }
    return -1;
}

size_t BLEClientSerial::write(uint8_t c)
{
    pTxCharacteristic->writeValue(c, true);
    delay(10);
    return 1;
}

size_t BLEClientSerial::write(const uint8_t *buffer, size_t size)
{
    for (int i = 0; i < size; i++)
        pTxCharacteristic->writeValue(buffer[i], false);
    return size;
}

void BLEClientSerial::flush()
{
    staticBuffer.clear();
}

void BLEClientSerial::end()
{
}

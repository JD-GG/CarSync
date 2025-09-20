// Installation instructions for the ESP32-Board
// https://randomnerdtutorials.com/installing-the-esp32-board-in-arduino-ide-windows-instructions/

// Boards:
// esp32 by Espressif Systems@2.0.17

// Select ESP32 Dev Module as your board
// Select in ArduinoIDE Tools > Partition Scheme > Huge APP

// Libraries:
// ELMDuino by PowerBroker2@3.4.1
// ESP8266 Influxdb by Tobias Schürg@3.13.2
// EspSoftwareSerial by Dirk Kaar@8.1.0
// TinyGPSPlus by Mikal Hart@1.0.3

#include <WiFi.h>
#include <esp_wifi.h>
#include <WiFiClientSecure.h>
#include <InfluxDbClient.h>
#include <Arduino.h>
#include <SoftwareSerial.h>
#include <TinyGPS++.h>
#include "secrets.h"
#include "BLEClientSerial.h"

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;
uint64_t macInt = 0;
uint32_t rpm = 0;
float latitude = 0.0;
float longitude = 0.0; 
float kmh = 0.0;

// Server certificate in PEM format, placed in the program (flash) memory to save RAM
// Neccessary for HTTPS connection validation
const char ServerCert[] PROGMEM =  R"EOF(
-----BEGIN CERTIFICATE-----
MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw
TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh
cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4
WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu
ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY
MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc
h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+
0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U
A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW
T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH
B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC
B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv
KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn
OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn
jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw
qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI
rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV
HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq
hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL
ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ
3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK
NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5
ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur
TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC
jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc
oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq
4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA
mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d
emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=
-----END CERTIFICATE-----
)EOF";

// BLE serial client
BLEClientSerial BLESerial;

// TinyGPS instance
TinyGPSPlus gps;

// Die serielle Verbindung zum GPS Modul
// ESP PIN 4 --> TX
// ESP PIN 5 --> RX
SoftwareSerial ss(4, 5);

// Zeitsync (für saubere Timestamps)
#define TZ_INFO "CET-1CEST,M3.5.0/2,M10.5.0/3"

// Influx-Client + Measurement-Point
InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN, ServerCert);
Point dataPoint("data"); // measurement name
Point initPoint("init");

// Intervall-Einstellungen
const uint32_t INFLUX_INTERVAL_MS    = 3000;
const uint32_t RPM_QUERY_INTERVAL_MS = 600;
const uint32_t BLE_RETRY_MS          = 5000;
const uint32_t ELM_RETRY_MS          = 5000;

// Zustandsvariablen
uint32_t lastInfluxMs   = 0;
uint32_t lastRPMQueryMs = 0;
uint32_t lastBLETryMs   = 0;
uint32_t lastELMTryMs   = 0;

bool bleConnected  = false;
bool elmReady      = false;
bool gpsReady      = false;

// ELM327 response buffer
std::string elmResponseBuffer = "";

// Calculate MAC-Adress in integer representation for easier comparison
void calcMacInt(){
  // As raw bytes
  uint8_t mac[6];
  WiFi.macAddress(mac);
  for (int i = 0; i < 6; i++) {
    macInt = (macInt << 8) | mac[i];
  }
}

// Connect to Wi-Fi with retries
void connectWiFi() {
  if (WiFi.status() == WL_CONNECTED) return;

  // Connect to STA first
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("Retry WiFi");
  
  // Attempt connection until found
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("Connected to WiFi!");
}

// Connect BLE
bool connectBLE() {
  static bool bleInitialized = false;
  if (!bleInitialized) {
    BLESerial.begin(OBD_NAME);
    bleInitialized = true;
  }

  bleConnected = BLESerial.connect();
  Serial.println(bleConnected ? "BLE verbunden." : "BLE fehlgeschlagen.");
  return bleConnected;
}

// Initialize ELM327 (just handshake / setup)
bool initELM327() {
  if (elmReady) return true;
  if (!bleConnected) return false;

  Serial.println("Initialisiere ELM327...");
  // No blocking: just make sure BLE is ready
  elmReady = true;
  Serial.println("ELM327 bereit.");
  return elmReady;
}

// Write RPM/GPS to Influx
void writeRPMToInflux() {
  if((!bleConnected || !elmReady) && !gpsReady){
    Serial.println("Nothing to send to Influx!");
    return;
  }

  dataPoint.clearFields();
  dataPoint.addField("mac", (uint64_t)macInt);
  if(bleConnected && elmReady){
    dataPoint.addField("rpm", (int32_t)rpm);
  }

  // If GPS was initialized once, write last known position
  if(gpsReady){
    dataPoint.addField("latitude", (float)latitude);
    dataPoint.addField("longitude", (float)longitude);
    dataPoint.addField("kmh", (float)kmh);
  }

  Serial.println(dataPoint.toLineProtocol());

  if (!client.writePoint(dataPoint)) {
    Serial.print("InfluxError: ");
    Serial.println(client.getLastErrorMessage());
  }
}

// Write an initial debug point
void writeInitialDebugPoint() {
  initPoint.clearFields();
  initPoint.addField("helloMac", (uint64_t)macInt);
  if (!client.writePoint(initPoint)) {
    Serial.print("Init-InfluxError: ");
    Serial.println(client.getLastErrorMessage());
  }
}

// Read GPS data from serial
void readGPS() {
  while (ss.available() > 0) {
    gps.encode(ss.read());
    if (gps.location.isUpdated()) {
      gpsReady = true;
      latitude = gps.location.lat();
      longitude = gps.location.lng();
      kmh = gps.speed.kmph();
      Serial.print("Lat="); Serial.print(latitude,4);
      Serial.print(" Lon="); Serial.print(longitude,4);
      Serial.print(" km/h="); Serial.println(kmh);
    }
  }
}

// Send RPM request command over BLE
void requestRpm() {
  const char rpmCmd[] = "010C\r";
  BLESerial.write((const uint8_t*)rpmCmd, strlen(rpmCmd));
}

// Parse RPM from ELM327 response buffer
bool parseRpmFromBuffer(uint32_t &outRpm) {
  while (BLESerial.available() > 0) {
    char c = BLESerial.read();
    elmResponseBuffer += c;
  }

  size_t pos = elmResponseBuffer.find("410C");
  if (pos != std::string::npos && elmResponseBuffer.size() >= pos + 6) {
    int A = strtol(elmResponseBuffer.substr(pos + 4, 2).c_str(), nullptr, 16);
    int B = strtol(elmResponseBuffer.substr(pos + 6, 2).c_str(), nullptr, 16);
    outRpm = ((A * 256) + B) / 4;
    elmResponseBuffer.erase(0, pos + 6);
    return true;
  }

  // Clear buffer if no valid data
  if (elmResponseBuffer.find("NO_DATA") != std::string::npos || elmResponseBuffer.find("?") != std::string::npos) {
    elmResponseBuffer.clear();
  }

  return false;
}

// Read RPM periodically without blocking
void readRpm() {
  uint32_t now = millis();
  if (now - lastRPMQueryMs >= RPM_QUERY_INTERVAL_MS) {
    lastRPMQueryMs = now;
    requestRpm();
  }

  uint32_t tempRpm = 0;
  if (parseRpmFromBuffer(tempRpm)) {
    rpm = tempRpm;
    Serial.print("RPM: "); Serial.println(rpm);
  }
}

void setup() {
  Serial.begin(115200);
  delay(300);

  // Free some CPU power
  esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT);

  // GPS setup
  ss.begin(9600);

  // Network setup
  connectWiFi();

  // Zeit synchronisieren (wichtig für TLS & Timestamps)
  timeSync(TZ_INFO, "pool.ntp.org", "0.pool.ntp.org");

  // Mac-Adresse berechnen
  calcMacInt();
  Serial.print("MAC: "); Serial.println(WiFi.macAddress());
  Serial.print("MAC int: "); Serial.println(macInt);

  // Influx testen
  if (client.validateConnection()) {
    Serial.print("Verbunden mit InfluxDB: "); Serial.println(client.getServerUrl());
  } else {
    Serial.print("InfluxDB-Verbindung fehlgeschlagen: "); Serial.println(client.getLastErrorMessage());
  }

  writeInitialDebugPoint();
}

void loop() {
  uint32_t now = millis();

  // === WiFi sicherstellen ===
  if (WiFi.status() != WL_CONNECTED) connectWiFi();

  // === BLE sicherstellen ===
  if (!bleConnected && now - lastBLETryMs >= BLE_RETRY_MS) {
    lastBLETryMs = now;
    connectBLE();
    if (bleConnected) lastELMTryMs = 0;
  }

  // === ELM initialisieren (non-blocking) ===
  if (bleConnected && !elmReady && now - lastELMTryMs >= ELM_RETRY_MS) {
    lastELMTryMs = now;
    initELM327();
    delay(2000); // Give ELM time to settle
  }

  // === GPS Daten lesen ===
  readGPS();

  // === RPM lesen ===
  if (elmReady) readRpm();

  // === Alle 1s RPM lesen & schreiben ===
  if (WiFi.status() == WL_CONNECTED && now - lastInfluxMs >= INFLUX_INTERVAL_MS) {
    lastInfluxMs = now;
    writeRPMToInflux();
  }
}

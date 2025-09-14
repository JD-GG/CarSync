#include <WiFiMulti.h>
#include <InfluxDbClient.h>
#include <Arduino.h>
#include <ELMduino.h>
#include <../BLEClientSerial/BLEClientSerial.h>
#include <secrets.h>

WiFiMulti wifiMulti;
BLEClientSerial BLESerial;
ELM327 myELM327;

// ==== Zeitsync (für saubere Timestamps) ====
#define TZ_INFO "CET-1CEST,M3.5.0/2,M10.5.0/3"

// Influx-Client + Measurement-Point
InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN);
Point rpmPoint("car_telemetry"); // measurement name

// ==== Intervall-Einstellungen ====
const uint32_t READ_INTERVAL_MS  = 1000;   // jede Sekunde RPM lesen/schreiben
const uint32_t WIFI_RETRY_MS     = 5000;
const uint32_t BLE_RETRY_MS      = 5000;
const uint32_t ELM_RETRY_MS      = 5000;

// ==== Zustandsvariablen ====
uint32_t lastReadMs     = 0;
uint32_t lastWiFiTryMs  = 0;
uint32_t lastBLETryMs   = 0;
uint32_t lastELMTryMs   = 0;

bool bleConnected  = false;
bool elmReady      = false;

void connectWiFi() {
  if (WiFi.status() == WL_CONNECTED) return;

  Serial.print("Verbinde WiFi");
  WiFi.mode(WIFI_STA);
  wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);
  while (wifiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(150);
  }
  Serial.print("\nWiFi OK, IP: ");
  Serial.println(WiFi.localIP());
}

bool connectBLE() {
  if (bleConnected) return true;
  Serial.println("Verbinde BLE zu OBD (VEEPEAK)...");
  BLESerial.begin("VEEPEAK"); // Name deines Adapters
  bleConnected = BLESerial.connect();
  if (bleConnected) {
    Serial.println("BLE verbunden.");
  } else {
    Serial.println("BLE-Verbindung fehlgeschlagen.");
  }
  return bleConnected;
}

bool initELM327() {
  if (elmReady) return true;
  if (!bleConnected) return false;

  Serial.println("Initialisiere ELM327...");
  // (stream, debug, timeout_ms)
  if (!myELM327.begin(BLESerial, true, 2000)) {
    Serial.println("ELM327 init fehlgeschlagen.");
    elmReady = false;
  } else {
    Serial.println("ELM327 bereit.");
    elmReady = true;
  }
  return elmReady;
}

bool ensureInflux() {
  if (client.validateConnection()) return true;

  Serial.print("InfluxDB Verbindung fehlgeschlagen: ");
  Serial.println(client.getLastErrorMessage());
  // Kein direkter Reconnect nötig: writePoint() versucht erneut; validateConnection() nur für Status
  return false;
}

void setupInfluxPoint() {
  rpmPoint.clearTags();
  rpmPoint.addTag("source", "obd");
  // weitere Tags wie VIN, Fahrzeug, etc. könntest du hier hinzufügen
}

void setup() {
  Serial.begin(115200);
  delay(300);

  connectWiFi();

  // Zeit synchronisieren (wichtig für TLS & Timestamps)
  timeSync(TZ_INFO, "pool.ntp.org", "time.nis.gov");

  // Influx testen
  if (client.validateConnection()) {
    Serial.print("Verbunden mit InfluxDB: ");
    Serial.println(client.getServerUrl());
  } else {
    Serial.print("InfluxDB-Verbindung fehlgeschlagen: ");
    Serial.println(client.getLastErrorMessage());
  }

  setupInfluxPoint();
}

void writeRPMToInflux(uint32_t rpm) {
  rpmPoint.clearFields();
  rpmPoint.addField("rpm", (int32_t)rpm); // als Integer schreiben
  // Optional: zusätzlich RSSI oder Status mitgeben
  // rpmPoint.addField("wifi_rssi", WiFi.RSSI());

  Serial.print("Schreibe: ");
  Serial.println(rpmPoint.toLineProtocol());

  if (!client.writePoint(rpmPoint)) {
    Serial.print("Influx write failed: ");
    Serial.println(client.getLastErrorMessage());
  }
}

void loop() {
  const uint32_t now = millis();

  // === WiFi sicherstellen ===
  if (WiFi.status() != WL_CONNECTED && now - lastWiFiTryMs >= WIFI_RETRY_MS) {
    lastWiFiTryMs = now;
    connectWiFi();
  }

  // === BLE sicherstellen ===
  if (!bleConnected && now - lastBLETryMs >= BLE_RETRY_MS) {
    lastBLETryMs = now;
    connectBLE();
    if (bleConnected) lastELMTryMs = 0; // nach erfolgreichem BLE neu versuchen ELM
  }

  // === ELM initialisieren ===
  if (bleConnected && !elmReady && now - lastELMTryMs >= ELM_RETRY_MS) {
    lastELMTryMs = now;
    initELM327();
  }

  // === Alle 1s RPM lesen & schreiben ===
  if (elmReady && WiFi.status() == WL_CONNECTED && now - lastReadMs >= READ_INTERVAL_MS) {
    lastReadMs = now;

    float tempRPM = myELM327.rpm();

    if (myELM327.nb_rx_state == ELM_SUCCESS) {
      uint32_t rpm = (uint32_t)tempRPM;
      Serial.print("RPM: ");
      Serial.println(rpm);
      writeRPMToInflux(rpm);
    } else if (myELM327.nb_rx_state != ELM_GETTING_MSG) {
      // Fehler anzeigen
      myELM327.printError();

      // Bei hartem Fehler ggf. ELM reconnect versuchen
      elmReady = false;
    }
  }

  // Optional: kurze Pause, damit die Loop nicht 100% CPU zieht
  delay(5);
}

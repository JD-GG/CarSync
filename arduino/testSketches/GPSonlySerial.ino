// install libraries: EspSoftwareSerial v8.1.0 and TinyGPSPlus v1.0.3

#include <SoftwareSerial.h>
#include <TinyGPS++.h>

TinyGPSPlus gps;

// Die serielle Verbindung zum GPS Modul
// ESP PIN 4 --> TX
// ESP PIN 5 --> RX
SoftwareSerial ss(4, 5);

void setup() {
  Serial.begin(115200);
  ss.begin(9600);
}

void loop() {
  while (ss.available() > 0) {
    gps.encode(ss.read());
    if (gps.location.isUpdated()) {
      // Breitengrad mit 3 Nachkommastellen
      Serial.print("Breitengrad= ");
      Serial.print(gps.location.lat(), 4);
      // Längengrad mit 3 Nachkommastellen
      Serial.print(" Längengrad= ");
      Serial.println(gps.location.lng(), 4);
      Serial.print("Km/h=");
      Serial.println(gps.speed.kmph());
    }
  }
}
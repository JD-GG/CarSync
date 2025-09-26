# CarSync
DHBW-Projekt verteilte Systeme von Luca Müller und Jan-David Oberländer

## Deployment
1. Repo clone
2. .env ins Root Verzeichnis
3. docker compose up --build -d --scale frontend=3 --scale backend=3
4. http://localhost:{GATEWAY_PORT}

## Grafana
Optional können vorgegebene dashboards importiert werden. (import von ./dashboards.json) Ohne tatsächliche Daten aber eher nutzlos.
Configuriere Influx als datenquelle mit URL und Token. Query language: Flux

## Container
Zur Verfügung stehen die folgenden Services:
- Frontend / Backend hinter LoadBalancer
- Adminer-UI
- Grafana-UI
- InfluxDB-UI

Ports und Benutzernamen bitte der .env entnehmen.

## Arduino / ESP32
Es wird ein ESP32 benötigt, um via BLE eine Verbindung zu dem ELM327 aufzubauen. Der ELM327 ist einer der verbreitesten Chips, wenn es um Bluetooth OBDII-Scanner geht. Der Hauptsketch befindet sich in ./arduino/mainSketch/mainSketch.ino. Dort sind zusätzliche Anweisungen vorhanden. Im selben Verzeichnis muss sich eine secrets.h befinden. Der Sketch muss derzeit mit der Arduino-IDE gebaut werden. 

### GPS-Modul
Verwendet wurde ein Aideepen GPS-Modul Positionsmodul (https://www.amazon.de/dp/B08CZSL193)
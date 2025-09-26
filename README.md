# CarSync
DHBW-Projekt verteilte Systeme von Luca Müller und Jan-David Oberländer

## Deployment
1. Repo clone
2. .env ins Root Verzeichnis
3. docker compose up --build -d --scale frontend=3 --scale backend=3
4. http://localhost:{GATEWAY_PORT}

## Grafana
Optional können vorgegebene dashboards importiert werden. (import von ./dashboards.json) Ohne tatsächliche Daten aber eher nutzlos.

## Container
Zur Verfügung stehen die folgenden Services:
- Frontend / Backend hinter LoadBalancer
- Adminer-UI
- Grafana-UI
- InfluxDB-UI

Ports und Benutzernamen bitte der .env entnehmen.
# CarSync
DHBW-Projekt verteilte Systeme 

## Deployment
1. Repo clone
2. .env ins Root Verzeichnis
3. docker compose up --build -d --scale frontend=3 --scale backend=3
4. http://localhost:{GATEWAY_PORT}
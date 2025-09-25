# CarSync Systemdokumentation

## Einleitung
CarSync ist eine Vollstack-Lösung zur Erfassung, Speicherung und Visualisierung von Fahrzeugtelemetriedaten in Echtzeit. Das Herzstück bildet eine Angular-Weboberfläche, die Fahrern und Flottenmanagern eine übersichtliche Darstellung aller relevanten Kennzahlen bietet. Der aktuelle Inkrement konzentriert sich auf die sichere Erfassung von Motordrehzahlen (RPM) und die nutzerspezifische Aufbereitung in einem Dashboard. Da im Rahmen des Projektes keine offiziellen Spezifikationen vorlagen, wurde eine eigenständige Anforderungsanalyse durchgeführt. Das vorliegende Dokument fasst die Architekturentscheidungen, Implementierungsdetails sowie die gewonnenen Erkenntnisse aus Sicht der Entwicklung zusammen. Zielgruppe sind technische Stakeholder, die sich rasch ein umfassendes Bild des Systems verschaffen möchten – von der Infrastruktur über Backend- und Frontend-Komponenten bis hin zu Betriebshinweisen und Lessons Learned.

## Architektur
Die Architektur folgt einem dienstebasierten Ansatz, der klar zwischen Datenerfassung, Speicherung, Veredelung und Darstellung unterscheidet. Der Fokus liegt auf modularen, leicht wartbaren Komponenten. Diese lassen sich unabhängig voneinander deployen, skalieren und weiterentwickeln, während definierte Schnittstellen eine konsistente Gesamtfunktionalität sicherstellen.

### Allgemeiner Aufbau
Die Lösung besteht aus vier Kernschichten: Datenpersistenz, Backend-Services, Frontend-Anwendung und Infrastruktur-Orchestrierung. Auf Persistenzebene kommen zwei spezialisierte Datenbanken zum Einsatz: MariaDB für Benutzerdaten sowie InfluxDB für Zeitreihentelemetrie. Die Backend-API agiert als Vermittlungsschicht. Sie authentifiziert Nutzer, validiert Anfragen, verknüpft Benutzerinformationen mit Messwerten und stellt optimierte Endpunkte bereit. Das Angular-Frontend bildet die Präsentationsschicht mit Fokus auf Nutzerführung und Visualisierung der Zeitreihen. Docker Compose orchestriert sämtliche Container und ermöglicht eine reproduzierbare Entwicklungs- wie auch Produktionsumgebung.

### Architekturelle Entscheidungen
Mehrere architekturrelevante Entscheidungen prägen das System:

1. **Trennung von relationaler und Zeitreihendatenbank**: Benutzer-, Authentifizierungs- und Geräteinformationen sind stark relational geprägt. Telemetriedaten hingegen sind append-only, werden in hoher Frequenz produziert und benötigen effiziente Aggregation. Durch die Kombination von MariaDB und InfluxDB werden beide Zugriffsmuster optimal bedient. Alternative Ansätze, etwa alles in einer universellen Datenbank abzulegen, wurden verworfen, weil sie entweder die Schreibperformance (MariaDB) oder den Abfragekomfort (InfluxDB) beeinträchtigt hätten.
2. **JWT-basierte Authentifizierung**: Statt auf Session-Cookies setzt CarSync auf kurzlebige JSON Web Tokens. Sie lassen sich leicht in Single-Page-Applications integrieren, sind unabhängig skalierbar und spielen gut mit API-Gateways zusammen. Die Entscheidung wurde getroffen, um auch zukünftige mobile Clients zu unterstützen. Nachteile wie die fehlende serverseitige Invalidierung werden durch begrenzte Lebenszeiten und optionale Blacklisting-Mechanismen entschärft.
3. **Angular als Frontend-Framework**: Die Wahl fiel auf Angular, weil es klare Architekturkonventionen mitbringt (Module, Services, Guards) und durch die CLI einen produktiven Entwicklungs- und Build-Prozess ermöglicht. Angular erleichtert das Zusammenspiel mit TypeScript, womit strikte Typsicherheit und einheitlicher Code-Stil erreicht werden.
4. **Containerisierung über Docker Compose**: Statt die Services direkt auf einem Host zu installieren, fasst Docker Compose alle Komponenten in isolierten Containern zusammen. So lassen sich Umgebungen schnell provisionieren, voneinander entkoppeln und reproduzierbar testen. Gerade bei einer verteilten Architektur reduziert dies Konfigurationsfehler.
5. **Sicherheitsgateway im Backend**: Alle Zugriffe auf InfluxDB werden über den Node.js-Server geleitet. Diese Entscheidung schützt vor unautorisierten direkten Influx-Zugriffen, da Tokens nie an das Frontend durchgereicht werden. Gleichzeitig entsteht ein passender Aggregationspunkt, um Filterkriterien (z. B. MAC-Adresse) zentral zu erzwingen.

### Systemkomponenten
Der Systementwurf umfasst die folgenden Komponenten, die über klar definierte Schnittstellen interagieren:

- **Angular-Frontend**: Stellt Login-, Registrierungs- und Dashboardseiten bereit. Kommuniziert ausschließlich über die REST-API mit dem Backend. Übernimmt Visualisierung, Statusanzeigen sowie lokale Session-Verwaltung. Die Komponente `DashboardComponent` ist für das Rendering der RPM-Zeitreihe verantwortlich.
- **Node.js/Express Backend**: Bietet Endpunkte für Registrierung, Login, Health-Check (`/ping`) und Telemetriedatenabruf (`/rpm-data`). Nutzerauthentifizierung und -autorisierung erfolgt mittels JWT, gespeicherte Passwörter werden via bcrypt gehasht. Es dient als BFF (Backend for Frontend) und kapselt die Logik zur Verbindung von Benutzern (MariaDB) und Messpunkten (InfluxDB).
- **MariaDB**: Speichert Benutzerkonten, zugehörige gehashte Passwörter und die als Integer kodierte MAC-Adresse. Die Tabelle `benutzer` wird bei Startup automatisch provisioniert, sodass keine separaten Migrationsskripte notwendig sind.
- **InfluxDB**: Verwaltet Telemetriedaten im Bucket `DB`. Für die Drehzahlen werden Messungen im Measurement `data` mit Feldern `_field=rpm` sowie `mac` abgelegt. Die Daten gelangen über externe Sensoren oder Simulationen in diese Datenbank; CarSync liest sie ausschließlich aus.
- **Docker Compose Infrastruktur**: Koordiniert die Container `backendCS`, `nginxCS` (für das ausgelieferte Angular Build), `influxdbCS`, `mariadbCS`, `adminerCS` und `grafanaCS`. Durch die gemeinsame Netzwerkkonfiguration können Services mit DNS-Namen (z. B. `mariadb`) aufeinander zugreifen.
- **Optionale Tools**: Adminer erleichtert die Inspektion der MariaDB, Grafana dient als ergänzendes Analysewerkzeug für Influx. Diese Komponenten sind nicht zwingend für den Betrieb notwendig, unterstützen jedoch Diagnose- und Monitoringaufgaben.

Die Interaktion verläuft wie folgt: Ein Benutzer greift über den Browser auf das Angular-Frontend zu. Für geschützte Bereiche holt sich das Frontend ein JWT über `/api/login`. Mit diesem Token fragt das Dashboard `/api/rpm-data` an. Das Backend authentifiziert das Token, ermittelt anhand der Benutzer-ID die freigegebene MAC-Adresse und formuliert ein Flux-Query gegen InfluxDB. Das Ergebnis wird als JSON an das Frontend zurückgeliefert, wo es direkt in das Diagramm eingespeist wird. Damit entsteht eine End-to-End-Kette, in der Benutzer nie Daten anderer Geräte einsehen können.

### Schnittstellen und Datenflüsse
Um die Zusammenarbeit der Komponenten greifbarer zu machen, wurden klare Schnittstellenverträge definiert:

- **REST-API**: Alle Endpunkte befinden sich unter dem Präfix `/api`. Requests und Responses werden als JSON übertragen. Für `/register` und `/login` sind die Requestschemata bewusst schlank gehalten, damit sich auch ressourcenarme Devices (z. B. Head-Units) problemlos integrieren lassen. Der Telemetrie-Endpoint akzeptiert optionale Query-Parameter wie `range=-12h`, die durch reguläre Ausdrücke validiert werden.
- **Authentifizierung**: Das Backend erwartet einen HTTP-Header `Authorization: Bearer <token>`. Das Token kodiert die Benutzer-ID sowie den Benutzernamen. Bei der Verifikation werden Ablaufdatum und Signatur geprüft. Token werden nicht serverseitig gespeichert, wodurch horizontale Skalierung trivial bleibt.
- **Datenfluss MariaDB**: Für jeden Benutzer existiert exakt ein Datensatz mit dem eindeutigen Feld `macInt`. Diese strenge Zuordnung ist die Grundlage für die Zugriffskontrolle. Ein geplanter Erweiterungspfad sieht vor, zusätzliche Geräte über eine 1:n-Beziehung anzubinden.
- **Datenfluss InfluxDB**: Zeitreihen werden in Nanosekunden-Auflösung gespeichert. Der Query-Pfad nutzt ausschließlich lesende Berechtigungen. Die Influx Token-Rolle besitzt nur Zugriff auf den Bucket `DB`, was das Schadenspotenzial bei Kompromittierung begrenzt.

Neben den technischen Schnittstellen existieren prozessuale Datenflüsse: Betriebslogs werden in der aktuellen Ausbaustufe lokal ausgegeben, können jedoch mithilfe von Docker-Logtreibern an zentrale Aggregatoren (Elastic, Loki) weitergeleitet werden. Alerts bei fehlgeschlagenen Flux-Abfragen werden derzeit manuell überwacht; ein Ausbau mit Prometheus und Alertmanager ist angedacht.

### Anforderungen
Die Projektdokumentation basiert auf einer Kombination aus angenommenen Anforderungen (abgeleitet aus Gesprächen mit Stakeholdern) und technisch notwendigen Rahmenbedingungen.

#### Funktionale Anforderungen
- Benutzer können sich registrieren, wobei Benutzername, Passwort und Fahrzeug-MAC-Adresse erforderlich sind.
- Das System verifiziert Anmeldedaten und stellt bei Erfolg ein zeitlich begrenztes JWT zur Verfügung.
- Auf dem Dashboard sieht ein angemeldeter Benutzer ausschließlich Telemetriedaten seines Fahrzeugs (Filterung auf MAC-Basis).
- Der Telemetrieendpunkt `/rpm-data` liefert RPM-Werte samt Zeitstempel innerhalb eines vorgegebenen Zeitfensters (default -6h) und begrenzt die Anzahl der Messpunkte, um die Frontend-Performance zu sichern.
- Sämtliche API-Aufrufe erfolgen über HTTPS (in der Produktionsumgebung durch vorgelagertes NGINX oder Load-Balancer bereitgestellt).
- Telemetrie-Daten lassen sich optional in Drittwerkzeugen analysieren (Grafana).

#### Nichtfunktionale Anforderungen
- **Sicherheit**: Passwörter werden gehasht, Tokens sind kurzlebig, direkte DB-Zugriffe vom Client sind untersagt. Flux-Queries validieren Filterkriterien, um Noisy Neighbor-Effekte zu verhindern.
- **Performanz**: Zeitreihenzugriffe dürfen auch bei hoher Datenrate innerhalb von Sekunden antworten. Durch Limits und serverseitige Filter bleibt die Antwortgröße kontrollierbar.
- **Skalierbarkeit**: Komponenten können unabhängig voneinander repliziert werden. InfluxDB unterstützt horizontale Skalierung, das Backend lässt sich hinter einem Load-Balancer betreiben.
- **Wartbarkeit**: Strikte Trennung von Verantwortlichkeiten, Verwendung etablierter Frameworks und klar benannter Env-Variablen vereinfachen Betrieb und Weiterentwicklung.
- **Beobachtbarkeit**: Durch Health-Checks und Logging im Backend können Fehlermodi schnell erkannt werden. Adminer und Grafana unterstützen Diagnose.
- **Portabilität**: Docker Compose garantiert konsistente Umgebungen. Entwickler können die gleiche Infrastruktur lokal nutzen wie auf Testsystemen.
- **Internationalisierung**: Auch wenn die aktuelle UI deutschsprachig ist, wurde auf eine flexible Formatierung (z. B. `Intl.DateTimeFormat`) geachtet, sodass weitere Sprachen ohne tiefgreifende Codeänderungen ergänzt werden können.
- **Erweiterbarkeit**: Architektur und Code basieren auf Schnittstellen, die zusätzliche Sensorfelder (Öltemperatur, Geschwindigkeit) erlauben. Die Flux-Queries lassen sich um weitere Felder erweitern, ohne die bestehende Logik zu brechen.
- **Resilienz**: Bei Ausfällen einzelner Komponenten (z. B. InfluxDB) sollen Benutzer eine klare Fehlermeldung erhalten, während der Rest der Anwendung funktionsfähig bleibt. Zeitkritische Funktionen wie Login dürfen nicht von der Telemetriedatenbank abhängen.

## Umsetzung
Dieser Abschnitt beschreibt die konkrete Implementierung der Architekturhinweise und legt die getroffenen technischen Maßnahmen dar.

### Implementierung der Architektur
Die Umsetzung folgt dem Prinzip „Backend for Frontend“ und nutzt bewährte Bibliotheken.

1. **Backend-Service (`backend/server.js`)**
   - Initialisiert Umgebungsvariablen via `dotenv`. Wird ein kritischer Parameter (z. B. `JWT_SECRET`, `INFLUX_TOKEN`) nicht gesetzt, beendet der Prozess den Startvorgang, um Fehlkonfigurationen früh zu erkennen.
   - Erstellt einen MariaDB Connection Pool mittels `mysql2/promise`. SQL-Injektionen werden durch Prepared Statements verhindert. Zusätzlich wird der Input auf verbotene Zeichen (derzeit Semikolon) geprüft.
   - Implementiert die Endpunkte `/register` (Schreiben neuer Benutzer), `/login` (Token-Ausgabe), `/rpm-data` (geschützter Telemetrieabruf). Für `/rpm-data` wird nach erfolgreicher JWT-Prüfung ein Flux-Query erstellt, das beide Felder (`rpm`, `mac`) pivotiert. Durch das Pivot entsteht eine zeilenorientierte Sicht, sodass Filter auf `mac` möglich sind und gleichzeitig nur die gewünschten Felder erhalten bleiben.
   - Nutzt die offizielle `@influxdata/influxdb-client` Bibliothek. Die Abfrage basiert auf einem asynchronen Cursor (`iterateRows`), wodurch Memoryverbrauch konstant bleibt, auch wenn viele Datenpunkte vorliegen.
   - Enthält eine einfache Health-Route `/ping` sowie eine Initialisierungsroutine `ensureTable`, die bei Bedarf die Users-Tabelle anlegt.
   - Verwendet strukturierte Logging-Statements für kritische Pfade (z. B. gescheiterte Influx-Anfragen). Diese Logs dienen sowohl der Fehlersuche als auch der Auditierbarkeit.

2. **Frontend (Angular)**
   - Die Anwendung wurde als Standalone-Angular-Projekt (Angular 20) aufgebaut. Komponenten sind standalone (`standalone: true`), wodurch auf die traditionelle Modulstruktur verzichtet werden kann.
   - Der `AuthService` verwaltet Login, Registrierung und LocalStorage-gestützte Sessions. Er kapselt die HTTP-Aufrufe und stellt Hilfsmethoden wie `getToken`, `getUsername` bereit.
   - `DashboardComponent` lädt einmalig beim `ngOnInit` die Telemetrie. Die Antwort des Backends wird in `rpmData` (Zahlenwerte) und `timeLabels` (formatierte Uhrzeiten) überführt. SVG-Pfade `chartPath` und `chartAreaPath` werden dynamisch berechnet, sodass die Darstellung flexibel auf beliebige Datenmengen reagieren kann.
   - Die HTML-Vorlage bleibt bewusst deklarativ: Ladezustände, Fehlerhinweise und das Diagramm werden über Angular-Strukturen (`ng-container`, `ngIf`) gesteuert. Die eigentliche Visualisierung beruht auf native SVG-Elemente statt auf externe Charting-Bibliotheken, um die Kontrolle über Darstellung und Performance zu behalten.
   - Schutzmechanismen (Route Guards) verhindern, dass nicht authentifizierte Nutzer auf das Dashboard zugreifen.

3. **Infrastruktur**
   - `docker-compose.yml` beschreibt sämtliche Services samt Ports, Abhängigkeiten und VOLUMES. Das Backend erhält seine Konfiguration über Env-Variablen (`INFLUX_URL`, `INFLUX_TOKEN`, `INFLUX_ORG`, `INFLUX_BUCKET`). Diese werden in `.env` gepflegt.
   - Der Backend-Dockerfile setzt auf `node:20`, installiert Abhängigkeiten und startet den Server via `node server.js`.
   - Angular wird in einem separaten Bild gebaut und von NGINX ausgeliefert. Für Entwicklung kann parallel `ng serve` genutzt werden.
   - CI/CD ist noch nicht produktiv umgesetzt, aber Build-Skripte (z. B. `npm run build`) sind vorbereitet. Der Docker-Ansatz erlaubt, Build-Artefakte im Pipeline-System zu erzeugen und versioniert zu veröffentlichen.

### Build- und Deployment-Prozess
Obwohl das Projekt bislang manuell betrieben wird, existiert ein klar definierbarer Deployment-Ablauf:

1. **Konfiguration prüfen**: Vor jedem Deployment werden `.env`-Parameter validiert. Dazu zählt insbesondere ein produktionsfähiger `JWT_SECRET` sowie ein auf Leserechte beschränktes `INFLUX_TOKEN`.
2. **Container bauen**: Über `docker compose build` werden Backend und Frontend neu gebaut. Der Angular-Build produziert statische Assets, die im NGINX-Container landen.
3. **Tests ausführen**: Derzeit beschränkt sich dies auf manuelle Checks (Linting ist vorgesehen). In Zukunft sollen Unit-Tests (`npm test`) und E2E-Szenarien integriert werden.
4. **Orchestrieren**: `docker compose up -d` startet alle Services. Dank definierter `depends_on`-Beziehungen fährt Influx und MariaDB vor dem Backend hoch.
5. **Smoke Tests**: Nach dem Hochfahren werden `/ping` sowie ein Login-Vorgang getestet. Für Telemetriedaten wird geprüft, ob das Dashboard valide Werte anzeigt.

Dieser Prozess ist bewusst einfach gehalten, kann aber problemlos in Automatisierungstools (GitHub Actions, GitLab CI) überführt werden. Der Container-basierte Ansatz stellt sicher, dass Entwicklungs-, Test- und Produktionsumgebungen identisch sind.

### Schwierigkeiten und Lösungen
Während der Umsetzung traten mehrere Herausforderungen auf:

- **MAC-Adressformatierung**: MariaDB speichert die MAC des Fahrzeugs als Integer (`macInt`). InfluxDB hingegen enthält das Feld `mac` als numerischen Wert. Deshalb musste sichergestellt werden, dass bei der Registrierung eine konsistente Hex-zu-Integer-Konvertierung erfolgt (`parseInt` mit Präfix `0x`). Ebenso prüft der Telemetrie-Endpunkt streng auf numerische Werte und behandelt unplausible Eingaben als Fehler.
- **Flux-Query-Design**: InfluxDB speichert jede Messung als separate Zeile mit `_field`. Um `mac` und `rpm` gemeinsam auswerten zu können, wird ein `pivot` benötigt. Erst das Pivot erlaubt den Vergleich `r.mac == ${macInt}` und das gleichzeitige Beibehalten des RPM-Wertes. Die Query musste iterativ optimiert werden, um nur relevante Spalten (`_time`, `rpm`) zu behalten und die Ergebnismenge (`limit`) zu kontrollieren.
- **Token-Handling im Frontend**: Da Angular standardmäßig keine globalen HTTP-Interceptors konfiguriert hatte, musste der Token-Header in der `DashboardComponent` explizit gesetzt werden. Dabei wurde bewusst eine schlanke Lösung gewählt, um die Komplexität nicht zu erhöhen. Für zukünftige Erweiterungen bietet sich ein Interceptor an.
- **Umgang mit fehlenden Linting-Regeln**: Das Angular-Projekt enthielt zwar ein `npm run lint` Script, jedoch kein konfiguriertes Ziel. Statt das Build zu blockieren, wurde die Fehlermeldung dokumentiert und als Nachbesserungsoption festgehalten. Bei Bedarf kann `angular-eslint` via `ng add` nachgezogen werden.
- **Asynchrone Fehlerbehandlung**: Sowohl die Influx-Abfrage als auch die Datenverarbeitung können scheitern (fehlende Daten, Timeouts). Das Backend loggt diese Fälle und sendet generische Fehlermeldungen („Failed to query telemetry data“), ohne sensible Informationen preiszugeben. Das Frontend reagiert darauf mit nutzerfreundlichen Hinweisen.

### Mögliche Alternativen
Die aktuelle Lösung erfüllt die Anforderungen, dennoch existieren Alternativen, die je nach Projektumfang sinnvoll sein könnten:

- **GraphQL statt REST**: Eine GraphQL-Schicht würde es erlauben, Telemetriedaten flexibler zu aggregieren. Für das derzeitige Feature-Set wäre der Overhead allerdings kaum gerechtfertigt.
- **WebSockets/Server-Sent Events**: Statt Polling ließen sich Live-Updates via WebSockets realisieren. Da die Dashboard-Anzeige momentan beim Laden aktualisiert wird und Telemetriedaten vergleichsweise ruhig sind, wurde darauf verzichtet. In einer späteren Ausbaustufe könnten Server-Sent Events implementiert werden, um Verzögerungen zu minimieren.
- **API-Gateway oder BFF in einer anderen Sprache**: Denkbar wäre ein Go-basiertes Backend, das nativ mit InfluxDB kommuniziert. Node.js bietet jedoch eine breitere Bibliothekslandschaft und war im Team bereits etabliert.
- **Persistenter Speicher für das Angular-Token**: Statt LocalStorage könnte SessionStorage oder ein In-Memory Storage genutzt werden, um XSS-Risiken zu reduzieren. Derzeit mitigiert ein Content-Security-Policy-Header (über NGINX konfigurierbar) dieses Risiko. Ein langfristiges Ziel wäre dennoch, die Token-Verwaltung Richtung httpOnly-Cookies zu verschieben.
- **Einheitliche Datenbank mit TimescaleDB**: Eine Alternative zur Dual-Datenbank-Lösung wäre der Einsatz von TimescaleDB. Damit ließen sich relationale Daten und Zeitreihen in einer Postgres-Instanz bündeln. Allerdings hätte das zusätzliche Know-how erfordert und den Setup-Aufwand erhöht.

## Reflexion
Der Rückblick auf das Projekt beleuchtet strategische Erkenntnisse sowie konkrete Herausforderungen.

### Was würde man nach dem Projekt anders machen?
- **Frühzeitige Linting- und Teststrategie**: Obwohl die Architektur sauber strukturiert ist, fehlt es an automatisierten Qualitätschecks. In einer nächsten Iteration würde von Anfang an `angular-eslint`, `Jest` oder `Vitest` sowie Integrationstests für das Backend etabliert, um Regressionen vorzubeugen.
- **Konfigurationsmanagement vereinheitlichen**: Aktuell verteilt sich die Konfiguration auf `.env`, `docker-compose.yml` und Hardcodings (z. B. Default-Range `-6h` im Code). Ein dediziertes Config-Modul oder ein zentraler Secrets-Manager (Vault, SSM) würde Klarheit schaffen. Ebenso sollte das Logging strukturierter (JSON Logs) gestaltet werden.
- **Token-Erneuerung/Refresh**: Der Login generiert Tokens mit einer Laufzeit von einer Stunde. Ein Refresh-Mechanismus fehlt bewusst. Für produktiven Einsatz wäre es sinnvoll, einen Refresh-Token-Flow oder eine automatische Verlängerung einzuführen, um die Benutzererfahrung zu verbessern.
- **CI/CD Pipeline**: Die Projektdateien enthalten noch keine Build-Pipeline. Bei einem erneuten Start würde zuerst eine CI/CD-Kette aufgebaut werden (Lint, Tests, Build, Deploy), um Feedbackzyklen zu verkürzen.
- **Datenvalidierung zentralisieren**: Inputvalidierungen sind aktuell in mehreren Dateien verstreut. Eine zentrale Validierungsschicht (z. B. `zod`, `joi`) würde Lesbarkeit und Wartbarkeit erhöhen.

### Größte Herausforderungen und Lösungen

- **Datenquelle synchronisieren**: Die wichtigste Herausforderung bestand darin, Daten aus zwei getrennten Quellen (MariaDB, InfluxDB) sicher zu korrelieren. Der Abgleich der MAC-Adresse war kritisch, da inkonsistente Formate schnell zu Datenlecks führen könnten. Gelöst wurde dies durch konsequente Integer-Repräsentation, Validierung und serverseitige Filter.
- **Fehlerrobuste Visualisierung**: Die Dashboard-Komponente musste sowohl mit reichhaltigen Daten als auch mit Edge Cases (keine Daten, Backend-Fehler) umgehen. Durch den Einsatz klarer Statusflags (`isLoading`, `errorMessage`, `hasData`) konnte die UI deterministisch bleiben. Dies verhindert, dass das Diagramm unvollständig oder fehlerhaft rendert.
- **Sichere API-Gestaltung**: Die Verlockung, InfluxDB direkt aus dem Browser anzusprechen, wurde bewusst unterdrückt. Die eingeführte BFF-Schicht stellt sicher, dass Tokens nicht in Client-Händen liegen und alle Queries serverseitig auditiert werden. Gleichzeitig dient sie als zukünftiger Ort für Geschäftslogik (z. B. Aggregationen, Alarmierungen).
- **Konfiguration sensibler Secrets**: Gerade beim Einsatz von externen Services wie InfluxDB ist der Umgang mit Tokens heikel. Durch die Auslagerung in `.env` Dateien und die Weitergabe per Docker Compose wurde verhindert, dass Secrets im Code landen. Langfristig sollten Secrets verschlüsselt oder über einen Secret Store verwaltet werden.
- **Technologiemix beherrschen**: Die Kombination aus Angular 20, Node.js, Docker und InfluxDB erfordert breit gefächertes Know-how. Durch klare Modulverantwortlichkeiten konnte die mentale Last jedoch verteilt werden. Dokumentation und einheitliche Code-Konventionen (z. B. TypeScript überall) erleichterten die Zusammenarbeit.

Aus betrieblicher Sicht zeigte sich, dass die Überwachung von Zeitreihensystemen spezielle Aufmerksamkeit benötigt. InfluxDB reagiert sensibel auf falsch konfigurierte Queries (z. B. fehlende Limits). Deshalb wurde ein Review-Prozess etabliert, bei dem Flux-Abfragen vor der Produktivsetzung evaluiert werden. Ebenso wird empfohlen, das Telemetrievolumen in synthetischen Tests nachzubilden, um Engpässe frühzeitig zu erkennen.

## Ausblick
Der aktuelle Funktionsumfang legt das Fundament einer verlässlichen Telemetrieplattform. Die Roadmap sieht mehrere Ausbaupfade vor:

- **Erweiterte Sensordatenerfassung**: Neben RPM sollen weitere Messgrößen (Öltemperatur, Batterieladung, Reifendruck) gesammelt werden. Das Backend muss dafür generische Filterlogik und Aggregationen bereitstellen, während das Frontend flexible Visualisierungen (Multi-Axis-Charts, Kachelübersichten) erhält.
- **Regelbasiertes Monitoring**: Schwellenwerte und Anomalien sollen definierbar sein. Eine Regel-Engine könnte bei Überschreiten automatisierte Benachrichtigungen (E-Mail, Push) auslösen. Dazu bedarf es eines Notification-Services und einer Historisierung von Regelereignissen.
- **Rollen- und Rechtekonzept**: Für Flottenmanager mit mehreren Fahrzeugen wird ein fein granuliertes Berechtigungssystem benötigt. Vorstellbar ist eine Hierarchie aus Organisationen, Fahrzeuggruppen und individuellen Nutzern, ergänzt durch RBAC-Tabellen in MariaDB.
- **Offline-Fähigkeit und Mobile Apps**: Eine Progressive Web App (PWA) oder native Anwendungen könnten Telemetriedaten auch bei instabiler Verbindung cachen und synchronisieren. Dafür müsste die API offlinefreundliche Endpunkte (Delta-Sync) anbieten.
- **Automatisiertes Qualitätsmanagement**: Kontinuierliche Tests, Security-Scans und statische Analysen sollen automatisiert in den Pipeline-Fluss integriert werden. Infrastructure-as-Code (z. B. Terraform) würde zudem den Betrieb über Docker Compose hinaus professionalisieren.

Diese Perspektiven zeigen, dass die gewählte Architektur ausreichend Spielraum für zukünftige Schritte lässt, ohne grundlegende Migrationen erzwingen zu müssen.

## Fazit
Die CarSync-Lösung demonstriert, wie sich Zeitreihendaten aus IoT-Geräten sicher und nutzerzentriert aufbereiten lassen. Der modulare Architekturansatz ermöglicht eine graduelle Erweiterung: Denkbar sind zusätzliche Sensorfelder (Temperatur, Geschwindigkeit), Benachrichtigungssysteme oder ein umfangreicher Rechteverwaltungsdienst. Durch die Kombination von MariaDB und InfluxDB bleibt das System performant, während Angular eine moderne Nutzererfahrung bereitstellt. Die gemachten Erfahrungen liefern wertvolle Hinweise für zukünftige Iterationen – insbesondere im Hinblick auf Qualitätssicherung, Konfigurationsmanagement und Echtzeitfähigkeit.

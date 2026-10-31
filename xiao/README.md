# XIAO ESP32-C6 — Rev F

Programmeert de losse Seeed XIAO ESP32-C6 vóór montage. Gebruik geen Atom- of Rev E-firmware: die heeft andere GPIO-aansluitingen.

Arduino IDE: ESP32 by Espressif **3.3.0**, ArduinoJson **7.4.2**, bord **XIAO_ESP32C6**, partitie **Huge APP**, USB CDC On Boot **Enabled**. Open de complete map `ChatGPTXIAO`, inclusief alle `.h`-bestanden.

Casio: bestaande V40.1 `ChatGPT.g3a`. Geen gewijzigde Casio-app nodig.

Rev F heeft geen USB/batterij-isolatie. Programmeer vóór montage. Voor later onderhoud: eerst J3-batterijstekker los, daarna USB aansluiten. USB verwijderen vóór J3 terugplaatsen. Ook zonder batterij kan USB spanning op de carrier zetten; dit is geen veilige vervanger voor loskoppelen van J3.

Pinindeling: UART1 RX GPIO17 / D7; TX GPIO16 / D6 (geïnverteerd voor Q2/Q3); heartbeat GPIO1 / D1; arm GPIO2 / D2. De hardware-gate blokkeert ROM-opstartuitvoer vóór het geldige Casio-protocolcontact.

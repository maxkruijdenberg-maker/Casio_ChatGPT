> Nieuwste uitwerking: [UPGRADE-V21.1.md](UPGRADE-V21.1.md), met chatbalk en PDF/HTML-handleiding.

# V20.1 — planeetring en de volledige OpenAI-accountlijst

Deze update vervangt geen bestaande V20.0-download: V20.1 is een apart pakket.
De donkere modus en het verfijnde logo blijven behouden.

## 1. Atom bijwerken

De volledige accountlijst vereist ook de nieuwe Atom-firmware. Alleen de .g3a
vervangen laat de oude filterende Atom-code staan.

1. Pak de volledige ZIP uit naar een gewone map op Windows.
2. Haal de Atom-USB los en koppel tijdelijk de Casio TX/RX-draden van de Atom los.
3. Sluit de Atom via USB op Windows aan.
4. Open `atom\ChatGPTAtom\ChatGPTAtom.ino` in Arduino IDE 2.x.
   Laat alle `.h`-bestanden ernaast staan, inclusief `model_catalog.h` en `model_kind.h`.
5. Kies **esp32 by Espressif Systems 3.3.0**, **ArduinoJson 7.4.2**,
   board **M5Atom**, de juiste COM-poort en uploadsnelheid **115200**.
6. Laat **Erase All Flash / volledig wissen uitgeschakeld**. Upload de sketch.
   `Hard resetting via RTS pin...` betekent dat upload klaar is.
7. Sluit Serial Monitor, haal de USB los, herstel de eerder gecontroleerde draden
   en geef de Atom opnieuw zijn eigen USB-voeding.

Voorgebouwde firmware kan ook via `py tools\flash_windows.py COM5` met esptool 5.0.0.
Vervang COM5 door je eigen poort; zie README.md. Gebruik geen erase-flash-opdracht.
Wifi, API-sleutel en model blijven in hetzelfde `config11`-opslagrecord staan.

## 2. Casio-app vervangen

1. Sluit de Casio op Windows aan en kies USB Flash / USB-opslag.
2. Vervang `ChatGPT.g3a` in de hoofdmap door `dist\ChatGPT.g3a` uit deze ZIP.
3. Laat `CGPT3` staan: je chats en scherminstellingen blijven behouden.
4. Werp veilig uit, ontkoppel de Casio-USB en herstart de Casio eenmaal.
5. Instellingen → Meer → Over deze app toont **Versie 20.1**.

## Ontwerp

De twee ongelijke boogjes zijn vervangen door één gecentreerde, schuin lopende
ellipsring. De linker- en rechterhelft liggen exact tegenover elkaar; het logo
staat in het midden als de bol van een planeet. De bestaande donkere stijl,
mint-lila logoafwerking en groene/rode verbindingsstatus blijven.
“Een vraag is genoeg om te beginnen” staat niet meer op het beginscherm.

## Alle modellen van jouw OpenAI-account

Open Instellingen → Model. De nieuwe Atom haalt de volledige `GET /v1/models`-lijst
voor jouw sleutel op, zonder familie-/modelfilter. Dit verstuurt geen chatvraag.
Beeld-, audio-, video-, embedding-, moderatie-, aangepaste en onbekende modellen
worden niet weggefilterd. Dubbele identieke IDs verschijnen eenmaal.

- Omhoog/omlaag kiest; F3 Vorig en F5 Volgend bladeren vijf regels.
- EXE/rechts/F1 stelt een chatmodel in. Bij bekende niet-chatmodellen opent F1 uitleg.
- F4 Info toont de volledige modelnaam en uitleg.
- F2 Basis toont de 26 ingebouwde SDK-keuzes. F2 Account haalt opnieuw jouw
  volledige accountlijst op. Bij de standaardlijst staat expliciet “standaardlijst”.

Bekende modeltypen hebben een kort label. Dat label is een herkenningshulp;
`/models` bevat geen complete informatie over welke API ieder model ondersteunt.
Onbekende namen blijven zichtbaar als Onbek. De app gebruikt Chat Completions:
een model kan toegang hebben en toch een andere API vereisen. Er wordt nooit
voor een keuzelijst automatisch een betaalde completion gedaan of een nieuwe
betaalde vraag gestart na een fout.

De uitgebreide basislijst bevat geverifieerde namen als `gpt-6-astra`,
`gpt-6.1-sol`, `gpt-6.1-terra`, `gpt-6-sol` en `gpt-6-luna`. SDK-vermelding is
geen bewijs dat jouw account toegang heeft. Het zijn geen verzonnen aliassen
zoals “ChatGPT 6”; de echte API-ID wordt opgeslagen.

De nieuwe firmware ondersteunt maximaal 1024 unieke IDs, 255 tekens per naam en
65535 bytes lijsttekst. Een te grote of ongeldige respons geeft een fout; er wordt
geen stilletjes ingekorte lijst als volledig gepresenteerd. Dit zijn technische
veiligheidsgrenzen, geen verborgen filter. Modellen van andere aanbieders en
web-only ChatGPT-keuzes horen niet bij jouw OpenAI-API-lijst.

## Controle en behoud

Chatopslag, verzoek-ID-beveiliging, telefooninstallatie, pins, baudrate en de
V13-toets-/verbindingsaanpak blijven. De grotere modellenlijst gebruikt tijdelijk
OS-heapgeheugen op de Casio; dat wordt vóór de MENU-overgang vrijgegeven.
De begrensde statische SDK-RAM-regio is niet vergroot.

Veertien ASan/UBSan-testgroepen controleren de ringsymmetrie in de renderer,
uitgebreide SDK-keuzes, de ongefilterde lijst, lange IDs/GET-config/upload,
niet-chat-uitleg zonder betaalde vraag, lijstgrenzen, geheugen vrijgeven en 40
menuheropeningen. Casio en Atom worden beide gebouwd.
Er is hier geen fysieke hardware of API-sleutel: test op jouw apparaat de echte
accountlijst, een vraag met een voor jouw sleutel beschikbaar model, Atom uit/aan,
veel toetsen en heropenen. De screenshots gebruiken voorbeelddata.

# ChatGPT voor Casio — V40.1

Rustigere interface, modelinfo op F2 en een volledige verbindingscontrole. Na iedere bevestigde modelkeuze volgt automatisch één kleine betaalde API-test. Het actieve model wordt alleen bevestigd met passende API-modelmetadata.

[Download V40](https://raw.githubusercontent.com/maxkruijdenberg-maker/Casio_ChatGPT/v40.1.0/dist/ChatGPT-V40.1-installatie.zip) · [Handleiding](docs/V40.1-HANDLEIDING.md) · [Bijwerken](UPGRADE-V40.1.md)

Werk de Casio-app én je wifi-module bij. Er zijn aparte builds voor de ATOM-testbench en de ESP32-C6 Rev E-print. Behoud CGPT3; volledig wissen is niet nodig. Internet/account/modelcontrole gebruikt een echte kleine aanvraag en geeft geen exact tegoedsaldo. Zie de handleiding voor kosten en annuleren.

De builds en hosttests slagen; echte hardware/API-tests voor deze nieuwe versie moet je nog uitvoeren. De bestaande Rev E-productiebestanden blijven beschikbaar. Voor de nieuwe zelf te solderen XIAO-print gebruik je de aparte Rev F-bestanden hieronder.

## Installeren en bouwen

Open START-HIER.html uit het uitgepakte klantpakket. Arduino IDE 2, ESP32-core 3.3.0 en ArduinoJson 7.4.2 zijn gebruikt. De ATOM-sketch staat in atom/ChatGPTAtom, de C6-sketch in esp32c6/ChatGPTC6. Voor C6: ESP32C6 Dev Module, 4 MB, Huge APP, USB CDC uit. Laat alle headers bij de sketches staan.

ASAN_OPTIONS=detect_leaks=0 bash tools/test.sh draait de hosttests in de voorbereide omgeving. tools/sync_c6.py synchroniseert de gedeelde backend met behoud van de aparte C6-pinnen en zendsper.

[Schermvoorbeelden](docs/design/V40.1-actual-screens.png) · [Validatiestatus](docs/V40.1-CONTROLE-VOOR-VERKOOP.md) · [PCB-bestanden Rev E 0,6 mm/OSP](hardware/RevE-06mm-OSP)

Oudere versies blijven beschikbaar via de Git-tags en bestaande upgradegidsen.

V40.1: Info op F2; Model toont de bevestigde naam. De statusbadge verschijnt alleen op het chatbeginscherm. Eindtekens en onzichtbare opmaak worden beter weergegeven. Na 35 seconden zonder Casio-contact schakelt de wifi-radio uit en gaat de CPU naar 80 MHz; UART-contact herstelt de normale werking. Deze stroom- en herverbindingswerking is nog niet fysiek gemeten.

## Nieuwe PCB: Rev F met zelf te solderen XIAO ESP32-C6

[Download JLCPCB-pakket](hardware/CG50-WiFi-RevF-JLCPCB.zip) · [Alleen drie productie-bestanden](hardware/RevF-XIAO-06mm-OSP) · [Ontwerp, maatvoering en controles](hardware/RevF-XIAO-ontwerp/README.md) · [Download XIAO-firmware + Casio-app](xiao/ChatGPT-XIAO-RevF-installatie.zip)

89 × 34 mm L-vorm, 0,6 mm, 2 lagen, OSP. JLCPCB monteert 41 SMT-onderdelen; U5 blijft leeg voor jouw eigen Seeed XIAO ESP32-C6. Veertien soldeergaten, geen headerrails, USB-C binnen de getekende contour, J3/J4 aan het rechteruiteinde. Geen schakelaar: programmeer de XIAO eerst los. Voor eventueel later USB-onderhoud altijd J3 los vóór USB; USB los vóór J3 terugplaatsen.

Gebruik uitsluitend de **XIAO**-firmware voor Rev F: RX GPIO17, TX GPIO16. De Atom- en Rev E-builds hebben andere pinnen. [Arduino IDE-instellingen](xiao/README.md).

Digitale controles: DRC/ERC/parity schoon; boorgaten, Gerber-padcentra en BOM/CPL gecontroleerd; XIAO-build geslaagd. De echte 7mm-inbouwhoogte, voeding/wifi-pieken, RF en UART moeten op een eerste PCBA worden getest. Dit is nog geen fysiek gekwalificeerde verkoopserie.

# ChatGPT voor Casio — V40.1

Rustigere interface, modelinfo op F2 en een volledige verbindingscontrole. Na iedere bevestigde modelkeuze volgt automatisch één kleine betaalde API-test. Het actieve model wordt alleen bevestigd met passende API-modelmetadata.

[Download V40](https://raw.githubusercontent.com/maxkruijdenberg-maker/Casio_ChatGPT/v40.1.0/dist/ChatGPT-V40.1-installatie.zip) · [Handleiding](docs/V40.1-HANDLEIDING.md) · [Bijwerken](UPGRADE-V40.1.md)

Werk de Casio-app én je wifi-module bij. Er zijn aparte builds voor de ATOM-testbench en de ESP32-C6 Rev E-print. Behoud CGPT3; volledig wissen is niet nodig. Internet/account/modelcontrole gebruikt een echte kleine aanvraag en geeft geen exact tegoedsaldo. Zie de handleiding voor kosten en annuleren.

De builds en hosttests slagen; echte hardware/API-tests voor deze nieuwe versie moet je nog uitvoeren. De PCB-productiebestanden zijn ongewijzigd.

## Installeren en bouwen

Open START-HIER.html uit het uitgepakte klantpakket. Arduino IDE 2, ESP32-core 3.3.0 en ArduinoJson 7.4.2 zijn gebruikt. De ATOM-sketch staat in atom/ChatGPTAtom, de C6-sketch in esp32c6/ChatGPTC6. Voor C6: ESP32C6 Dev Module, 4 MB, Huge APP, USB CDC uit. Laat alle headers bij de sketches staan.

ASAN_OPTIONS=detect_leaks=0 bash tools/test.sh draait de hosttests in de voorbereide omgeving. tools/sync_c6.py synchroniseert de gedeelde backend met behoud van de aparte C6-pinnen en zendsper.

[Schermvoorbeelden](docs/design/V40.1-actual-screens.png) · [Validatiestatus](docs/V40.1-CONTROLE-VOOR-VERKOOP.md) · [PCB-bestanden Rev E 0,6 mm/OSP](hardware/RevE-06mm-OSP)

Oudere versies blijven beschikbaar via de Git-tags en bestaande upgradegidsen.

V40.1: Info op F2; Model toont de bevestigde naam. De statusbadge verschijnt alleen op het chatbeginscherm. Eindtekens en onzichtbare opmaak worden beter weergegeven. Na 35 seconden zonder Casio-contact schakelt de wifi-radio uit en gaat de CPU naar 80 MHz; UART-contact herstelt de normale werking. Deze stroom- en herverbindingswerking is nog niet fysiek gemeten.

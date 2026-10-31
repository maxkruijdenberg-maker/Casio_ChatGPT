# Rev G — XIAO ESP32-C6, alleen TX / RX / GND

**Gebruik deze versie voor de drie-draads Casio-aansluiting. Rev F vereiste een vierde VIO-draad en past niet bij die wens. Meng geen Rev F-hardware of firmware met Rev G.**

[Onderdeelcontrole en ontwerpkeuzes](ONDERDELENCONTROLE.md)

De XIAO wordt door jou vóór montage los geprogrammeerd en daarna op 14 doorgemetalliseerde gaten gesoldeerd, zonder headerrails of sockets. De XIAO levert zelf MCU, wifi, USB-C, programmering en 3V3-regeling. Er zit geen tweede ESP32, CH340, losse USB-C-connector of extra acculader op deze carrier.

- 89 × 34mm L-vorm; 34 × 34mm kop, 55 × 19mm arm. Twee lagen, 0,6mm FR4, OSP.
- JLCPCB monteert **38 onderdelen**, verdeeld over **20 bestelcodes**. U5 blijft leeg voor de eigen XIAO ESP32-C6.
- J3 en J4 zitten aan het rechteruiteinde; de USB-C van de XIAO valt binnen de getekende contour.
- Alleen Casio TX, RX en GND. Geen Casio-VIO-draad. De aparte batterijvoeding blijft uiteraard via J3 lopen.
- Geen voedingsschakelaar. Programmeer vooraf los; voor eventueel later USB-onderhoud J3 eerst los, dan USB aansluiten. USB verwijderen vóór J3 terugplaatsen.

## Bestellen bij JLCPCB

Gebruik uitsluitend de drie bestanden in `hardware/RevG-XIAO-3wire-06mm-OSP` of pak het buitenste `CG50-WiFi-RevG-3wire-JLCPCB.zip` uit:

1. `GERBER-RevG-XIAO-3wire-06mm-OSP.zip`: upload voor fabricage.
2. `BOM-JLCPCB.csv`: onderdelenlijst voor assemblage.
3. `CPL-JLCPCB.csv`: plaatsingsbestand voor assemblage.

Kies 2 lagen, afgewerkte dikte 0,6mm, OSP en bovenkant SMT-assemblage. Geen VIPPO nodig. U5 mag NIET door JLC worden gemonteerd. De via's zijn 0,6mm diameter / 0,3mm boring, buiten SMT-pads. Laat productie-/transportstroken zo nodig door JLC toevoegen zonder de afgewerkte contour aan te passen.

Gerber, boringen en CPL gebruiken dezelfde oorsprong: linksonder, X naar rechts en Y omhoog. JST-centra zijn gecorrigeerd voor de footprint-oorsprong: J3 +0,45mm en J4 +0,425mm in lokale voorwaartse richting. Controleer de JLC-preview op de voorbeeldafbeelding: connectoropeningen naar rechts, U5 leeg. Fabrikantspecifieke JLC-modelrotaties zijn niet via jouw account ingezien. Pas geen componentvervanging toe zonder footprint/pinout te controleren.

## Kabels en aansluitingen

| Positie | Verbinding | Kabelbehuizing |
|---|---|---|
| J3.1 | Ruwe 4×AAA BAT+ | JST SH SHR-02V-S, 1mm |
| J3.2 | BAT- / GND | Zelfde 2-polige kabel |
| J4.1 | **Casio TX → PCB RX** | JST GH GHR-03V-S, 1,25mm |
| J4.2 | **Casio RX ← PCB TX** | Zelfde 3-polige kabel |
| J4.3 | Casio GND | Zelfde 3-polige kabel |

J4 is gewijzigd naar een beschikbare **driepolige JST GH**. De oude vierpolige SH-kabel past hier niet. J3 is SH, J4 is GH: verschillende stekkerfamilies. Controleer pinnummers en continuïteit; vertrouw niet op draadkleuren. Geschikte contacten/kabelassemblages apart aanschaffen. Voeg geen extra serieweerstanden toe: de carrier heeft die al.

## Montage en firmware

1. Programmeer de losse XIAO met `ChatGPT-XIAO-RevG-3wire-installatie.zip`. Arduino: ESP32-core 3.3.0, ArduinoJson 7.4.2, bord XIAO_ESP32C6, Huge APP, USB CDC Enabled. Alleen deze bordkeuze wordt geaccepteerd. De bestaande Casio V40.1-app is compatibel en zit ook in dit pakket.
2. USB los; controleer de PCBA eerst zonder XIAO met een stroombegrensde voeding. Meet ongeveer 4,98V tussen U5.14 en U5.13; controleer op kortsluiting/polariteit.
3. Soldeer de XIAO bovenop met losse passende pennen of rechte draadstukjes: 14 gaten, 1,05mm boring, 2,54mm steek, 15,24mm rijafstand. USB wijst naar de onderrand van de kop. Isoleer de onderkant van de XIAO en houd het antennegebied vrij. Sluit de onderste XIAO-BAT-pads nergens op aan.
4. Voed via J3, verbind de drie Casio-signalen via J4 en open ChatGPT. Firmware: RX GPIO17, **normale, niet-geïnverteerde TX GPIO16**; GPIO1 heartbeat, GPIO2 arm. De Rev F-firmware gebruikt andere uitgangspolariteit en is hiervoor ongeschikt.
5. Controleer op de echte eerste PCBA voeding, wifi, UART, opnieuw openen/sluiten en de totale inbouwhoogte inclusief isolatie/kabels voordat de behuizing definitief dichtgaat.

De uitgang is een 3V3-buffer zoals het signaaltype van de werkende ATOM-testbench, met hardware-zendsper. Alleen geldig Casio-protocolcontact kan de uitgang inschakelen. Na inschakelen krijgt de lijn 200us om hoog te worden voordat de eerste startbit wordt verzonden. Een lang laag blijvende RX-lijn zonder ontvangen data/protocolvoortgang sluit de uitgang vanaf 100ms; zonder geldig contact vervalt de lease na 30s. Wifi gaat na 35s contactverlies uit, CPU naar 80MHz. Dit is lijn-/protocolbewaking, geen rechtstreekse meting van de fysieke Casio-aan/uitstatus.

## Controles

DRC 0, ERC 0, onverbonden 0, schema/print-pariteit 0. Audit van bronnetten, alle componentposities, 14 XIAO-gaten, 53 via's, 174 koper-pad/via-centra en 67 boringen. Geen via in SMT-pads; geen XIAO in BOM/CPL/pasta. Voedingsberekening, TI-pinout en TI-land/mask/stencil-geometrie gecontroleerd; onderdelenmatches en catalogusvoorraad gecontroleerd. De nieuwe J4-code en bijbehorende fabrikantentabel zijn uit dezelfde vastgelegde catalogusrevisie gelezen.

De XIAO-build en 26 softwaretestgroepen slagen. Dit is een onderbouwd prototypeontwerp; voedingstransiënten, temperatuur, RF en de werkelijke Casio-ingang zijn nog niet op een gebouwde Rev G gemeten. Die resterende praktijktest is iets anders dan de nu uitgevoerde onderdeel- en ontwerplogicacontrole.

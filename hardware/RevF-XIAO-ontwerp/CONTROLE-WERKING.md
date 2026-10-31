# Rev F — functionele controle vóór bestellen

Beoordeelde hardware: commit `0a74894`, ongewijzigde Gerber/BOM/CPL. De XIAO-firmware heeft na deze controle een opstartcorrectie gekregen. Gebruik de bijgewerkte XIAO-download; de Casio-app blijft gelijk.

## Uitkomst

**De digitale tests slagen. De echte PCB-werking is nog niet bewezen.** Zonder gebouwde carrier en aangesloten Casio kunnen voeding, RF en echte UART-signalen niet worden gemeten. Deze controle is geen vrijgave voor serieproductie.

| Controle | Resultaat |
|---|---|
| Volledige schema/PCB-controle opnieuw uitgevoerd | DRC 0, ERC 0, onverbonden 0, parity 0 |
| Firmwaretests met ASan/UBSan | 24 groepen geslaagd; LeakSanitizer uit vanwege de testomgeving |
| Nieuwe test op echte XIAO GPIO-initialisatiecode | Faalt met oude code, slaagt na correctie |
| Geldig HELLO, beschadigd CRC, verboden verzoek vóór HELLO | Zendsper opent alleen via geldig protocolcontact |
| Niet-geconfigureerde UART / timeout / opnieuw openen | Software sluit de zendsper en kan opnieuw openen |
| Heartbeat via de echte GPIO-hooks | Pulsen van 5us in het hostmodel; echte puls nog niet gemeten |
| Verkeerde generieke C6-bordkeuze | Afgewezen tijdens compilatie |
| XIAO-bordvariant | Geselecteerde Arduino-variant zet de ingebouwde antenne aan |
| Echte XIAO Arduino-build | Geslaagd; 1.183.641 bytes programma, 52.076 bytes statische RAM |
| Vereenvoudigd UART-RC-model | 3.072 bytegevallen geslaagd bij de hieronder vermelde aannamen |

## Gevonden en hersteld

`boardInitPins()` gebruikte `digitalWrite(LOW)` vóór `pinMode(OUTPUT)`. In Arduino ESP32 3.3.0 negeert `digitalWrite` de opdracht als de pin nog niet als GPIO is toegewezen. De nieuwe regressietest laat de uitgangslatches bewust op HIGH starten en modelleert dit gedrag uit `esp32-hal-gpio.c`. De oorspronkelijke code faalt; de correctie met `gpio_set_level()` vóór het inschakelen van de uitgangen slaagt. Dit bewijst de softwarefout en correctie binnen het GPIO-model; er is geen glitch met een oscilloscoop gemeten.

De firmware vereist nu bovendien het bord XIAO_ESP32C6. Een generieke C6-keuze zou de Seeed-antenne-initialisatie missen. De correcte XIAO-build slaagt, een verkeerde borddefinitie wordt afgewezen.

## Elektrische beoordeling en grenzen

De netlist is vergeleken met afzonderlijk vastgelegde verwachtingen voor de voedingsaansluitingen, XIAO-pinnen, Q1/Q2/Q3, RX-buffer en monostabiele zendsper. U3/U4-pinnummers zijn naast de lokale KiCad-symbolen 74LVC1G17 en 74LVC1G123 gelegd. De test controleert onder andere dat Q2 en Q3 werkelijk in serie staan, de gate-pulldowns behouden blijven en RX niet rechtstreeks op een ESP32-ingang wordt gezet.

Het UART-model gebruikt 9600 baud, alle 256 bytes, VIO van 3/3,3/5V en ingangscapaciteit van 1pF–5nF. Aangenomen zijn 30%/70%-ontvangstdrempels, 0,2ohm voor de twee ingeschakelde MOSFETs samen en verder geen onbekende Casio-ingangsbelasting. De laagste hoge bemonstering is 91,0% van VIO en de hoogste lage 17,6%. Dit is een vereenvoudigde RC-berekening, geen transistor- of fabrikantsimulatie. Een onbekende interne Casio-pull-up of andere ingangscircuits veranderen die resultaten. Bij een parallelle interne pull-up zou die in dit statische model groter dan ongeveer 4,64kohm moeten zijn. De echte ingangseigenschappen zijn hier niet gemeten.

De uitgangsspanning is berekend op 4,978V met een aangenomen 0,8V-feedbackreferentie. Er is geen TPS63070-fabrikantmodel gedraaid. Nieuwe primaire datasheets ophalen werd met HTTP 403 geweigerd; dit rapport geeft dus geen nieuwe fabrikantcontrole van de converter of diens aangepaste landpatroon. Spanningsrimpel, regelstabiliteit, brown-out, thermisch gedrag en wifi-stroompieken zijn niet bewezen door de berekening.

## Nodig voor een werkingsbewijs

Een eerste fysieke PCBA met XIAO en Casio: voeding meten tijdens opstarten en wifi-zenden; UART-niveaus en zendsper met een oscilloscoop controleren; herhaald openen/sluiten, telefoonconfiguratie en echte API-antwoorden testen. Alleen zo kan deze combinatie werkelijk werkend worden verklaard. De digitale controles vervangen deze metingen niet. Batterijstekker J3 blijft altijd los vóór aansluiting van XIAO-USB.

Machineleesbare resultaten staan in `reports/functional-software-tests.json` en `reports/functional-electrical-analysis.json`. De testcode staat in `tests/test_xiao_hardware.cpp` en `source/functional_checks.py`.

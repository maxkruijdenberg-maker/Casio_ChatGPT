# Rev F — zelf te solderen XIAO ESP32-C6

Nieuwe, goedkopere variant op Rev D. Gebruik voor deze print uitsluitend de drie bestanden in `hardware/RevF-XIAO-06mm-OSP` op GitHub: GERBER ZIP, BOM CSV en CPL CSV. Meng geen bestanden van Rev E met deze versie.

- L-vorm: 89 × 34 mm; kop 34 × 34 mm, arm 55 × 19 mm.
- 2 koperlagen, 0,6 mm FR4, OSP; alle 41 fabriekscomponenten bovenop.
- Jij monteert de Seeed XIAO ESP32-C6 (zonder headers) op U5: 14 doorgemetalliseerde gaten, boring 1,05 mm, steek 2,54 mm, rijafstand 15,24 mm. Gebruik losse passende soldeerpennen of rechte draadstukjes, geen headerrails of sockets.
- USB-C zit op de XIAO en wijst naar de onderrand van de brede kop. De conservatief getekende USB-contour eindigt op 33,5 mm, binnen de 34mm-printcontour.
- J3 batterij en J4 Casio zitten aan het rechteruiteinde. Nieuwe batterijstekker: JST SH SHR-02V-S, 1mm; geen JST PH. J4: SHR-04V-S.
- Geen los ESP32-module, CH340, programmeerconnector, voedingsschakelaar of extra accu.

## JLCPCB

Upload `GERBER-RevF-XIAO-06mm-OSP.zip`. Kies **2 lagen, 0,6 mm, OSP, bovenkant SMT-assemblage**. Normale afgedekte vias, geen VIPPO. Upload daarna `BOM-JLCPCB.csv` en `CPL-JLCPCB.csv`.

Er zijn **41 montageposities, 22 bestelcodes**. U5 moet leeg blijven: je monteert de XIAO zelf. Bij handmatige substitutie moeten pinout en footprint opnieuw gecontroleerd worden. De voorraadmomentopname van 31 oktober 2026 had minimaal 1.678 stuks per gekozen code; dit is geen voorraadreservering of prijsopgave.

CPL heeft dezelfde oorsprong als Gerber en boorbestand: linksonder (0,0), X naar rechts, Y omhoog. JST-connectoren gebruiken het fysieke lichaamsmiddelpunt, met de 0,45mm-offset ten opzichte van hun KiCad-oorsprong. Controleer vóór bestellen de JLCPCB-preview met `previews/Assembly-top.png`: J3 en J4 openen naar rechts, U5 leeg. JLC-modelrotaties kunnen leveranciersspecifiek zijn; die online preview is hier niet ingezien. Laat JLC zo nodig productie-/assemblageranden toevoegen zonder de afgewerkte contour te veranderen.

## Montage

1. Programmeer de **losse XIAO** via USB-C met de meegeleverde **Rev F XIAO-firmware**. De Atom- en Rev E-firmware hebben andere aansluitingen en zijn hiervoor niet geschikt. Casio V40.1 blijft bruikbaar.
2. Koppel USB los. Controleer de kale PCBA met stroombegrensde voeding, zonder XIAO: correcte polariteit, ongeveer 4,98 V op U5.14 versus U5.13 en geen kortsluiting.
3. Pas XIAO en carrier droog in de behuizing. Beide printplaten, USB-behuizing, draadstukken, soldeer, isolatie en kabelbochten samen moeten binnen jouw 7mm-grens blijven. De tekening bewijst de werkelijke hoogte niet.
4. Monteer de XIAO bovenop met minimale veilige isolerende afstand; houd het antennegebied vrij. Gebruik een uitlijnmal en soldeer losse pennen eerst diagonaal, daarna de rest. Houd metalen onderzijde en Casio-print geïsoleerd. Laat geen scherpe pinnen uitsteken.
5. J3: pin 1 BAT+, pin 2 GND van het bestaande 4×AAA-pakket. J4: pin 1 Casio TX, pin 2 Casio RX, pin 3 GND, pin 4 geschakelde Casio VIO voor de RX-pull-up. Controleer de stekkernummering, vertrouw niet op draadkleuren. Geen extra serieweerstanden toevoegen: deze zitten op de carrier.
6. Test voeding, UART, wifi en herhaald openen/sluiten op de echte hardware voordat je de behuizing definitief sluit.

**Deze uitvoering heeft geen isolatie tussen batterijvoeding en XIAO-USB-VBUS. Voor eventueel later USB-onderhoud: J3 eerst los, dan pas USB aansluiten. USB los voordat J3 weer wordt aangesloten.** Eenmalig vooraf programmeren is de normale montageprocedure. Firmware-updates blijven mogelijk, maar zijn niet automatisch draadloos toegevoegd.

## Wat is gecontroleerd?

KiCad 9.0.2: **DRC 0, onverbonden 0, schema/print-verschillen 0, ERC 0**. Bron/print-audit: 127 fysieke pads en alle netten/plaatsingen. XIAO: 14 gaten en oriëntatie gecontroleerd tegen de officiële DIP-maatvoering. Via-audit: 56 vias van 0,6/0,3 mm, geen via-koper in SMT-pads. Exportcontrole: 183 koper-pad/via-middelpunten, alle 70 boringen, zes contoursegmenten en overeenkomende BOM/CPL-posities. Geen XIAO-pasta of XIAO in de fabrieks-BOM/CPL.

De XIAO-firmware compileert met ESP32-core 3.3.0 en ArduinoJson 7.4.2: 1.183.641 bytes programma en 52.076 bytes statische RAM. De gedeelde applicatiecode is gelijk aan V40.1; UART-pinnen zijn aangepast naar RX17/TX16.

Dit zijn digitale controles. De eerste PCBA moet nog worden beoordeeld op reflowkwaliteit, werkelijke 7mm-passing, kabelruimte, RF-bereik, piekbelasting, temperatuur en UART-signalen. De TPS63070-footprint is het eerder gebruikte aangepaste RNM0015A-landpatroon; inspecteer de fijne soldeerverbindingen bij de eerste assemblage. De bestanden zijn een bouwbaar prototypeontwerp, geen fysiek gekwalificeerde verkoopserie.

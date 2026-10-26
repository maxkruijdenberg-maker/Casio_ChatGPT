# ChatGPT V3 — pictogram, volledig scherm en lokale chatgeschiedenis

Deze V3 bevat uitsluitend de ChatGPT-app met het aangepaste pictogram,
volledige schermverversing en een zijbalk voor bewaarde gesprekken. Er is geen
apart startmenu, toegangscode, verborgen sneltoets, notitie-app of gamemenu.

## Bijwerken op Windows

1. Download de volledige V3-ZIP en pak die uit naar een **nieuwe map**.
2. Sluit ChatGPT op de Casio af met MENU. Zet de Casio uit, haal de USB uit de
   Atom en maak de signaaldraden aan G32/G26 tijdelijk los. GND mag blijven.
3. Sluit de Atom weer via USB aan. Open **de nieuwe**
   `atom\ChatGPTAtom\ChatGPTAtom.ino` in Arduino IDE. Houd alle bijbehorende
   `.h`-bestanden in dezelfde map; V3 voegt onder meer `context.h` toe.
4. Gebruik board **M5Atom**, esp32-core **3.3.0**, ArduinoJson **7.4.2**, jouw
   COM-poort en uploadsnelheid **115200**. Compileer en upload.
5. Laat 'Erase all flash' uit. NVS-naam, sleutels, bootloader en partitietabel
   zijn gelijk gebleven; bestaande hotspotgegevens, model en API-sleutel
   horen bij een gewone update bewaard te blijven. Controleer dat na afloop.
6. Na 'Hard resetting via RTS pin...' is uploaden klaar. Koppel Atom-USB los,
   herstel de gecontroleerde signaaldraden en voed de Atom weer via USB.
7. Zet de Casio aan op zijn AAA-batterijen, sluit hem met zijn eigen USB-kabel
   aan op Windows en kies F1 **USB Flash**.
8. Vervang het bestaande bestand in de hoofdmap door **het nieuwe
   `dist\ChatGPT.g3a`**. Maak geen tweede kopie met '(1)' of 'V3' in de naam;
   dat kan een tweede pictogram opleveren. Kopieer de Atom-bestanden niet mee.
9. Werp het Casio-station veilig uit, ontkoppel alleen de Casio-USB en open
   ChatGPT via MENU. De app opent direct het invoerscherm.

## Eerste controles

- Het menupictogram heeft een witte achtergrond en een zwart knooplogo.
  De bitmap bevat **geen naamtekst meer**. De Casio tekent zelf de normale
  grijze of geselecteerde zwarte appnaam, zodat er geen tweede naam overheen
  staat. De precieze native weergave moet op jouw Casio worden gecontroleerd.
- F6 Instellingen → Versie: **3.0 / Atom 3.0**.
- F6 Instellingen → F4 **Scherm**: controleer of je de volledige mintgroene
  rand en alle vier markeringen **TL, TR, BL, BR** ziet. EXIT keert terug.
  V3 onderdrukt de OS-statusbalk tijdens tekenen en ververst expliciet alle
  rijen 0 t/m 215. Dit is softwarematig gecontroleerd, nog niet op hardware.
- F3 in het vraag-invoerscherm: UART-diagnose. 'Protocol v1' blijft correct:
  de berichtindeling is v1, terwijl app en firmware V3 zijn.
- Controleer hotspot/model/sleutelstatus en verstuur één korte echte vraag.
  Dat kan API-kosten maken. Het antwoord wordt na ontvangst lokaal opgeslagen.

## Zijbalk en vorige berichten

- **OPTN** in het invoer- of antwoordscherm opent de zijbalk.
- Bij een antwoord werkt ook **F4 Chats**. Tijdens typen blijft F4 Tekens.
- Pijlen kiezen een opgeslagen gesprek; **F1 of EXE** opent het.
- **F2 Wis** verwijdert een gekozen gesprek na bevestiging.
- **F3 Nieuw** maakt een leeg gesprek; een nog niet verstuurde vraag wordt
  alleen na bevestiging verworpen.
- EXIT sluit de zijbalk. Je kunt chats ook zonder Atom of wifi lezen.
- Bij antwoorden bladeren **F5 Vorig** en **F6 Volgend** door opgeslagen
  vraag/antwoordparen. Bij het laatste paar is F6 weer Instellingen.
- Pijlen scrollen het huidige antwoord. **SHIFT** toont de volledige vraag
  in een apart, scrollbaar scherm. De vraagballon is alleen een korte preview.
- F1 begint een vervolgvraag bij het **laatste bericht** van het gesprek,
  ook als je zojuist een ouder bericht bekeek. Er worden geen takken gemaakt.

## Opslag, limieten en herstel

- Er zijn **acht lokale gesprekken**, ieder met maximaal **acht complete
  vraag/antwoordparen**. De grens wordt vóór een nieuwe API-vraag gecontroleerd.
  Vol is vol: er wordt niet automatisch een oud gesprek overschreven.
- Per vraag maximaal 1024 UTF-8-bytes; per antwoord maximaal 8192 UTF-8-bytes.
  Alle acht paren blijven volledig terug te lezen, inclusief de volledige vraag.
- De app maakt de map **CGPT3** in Casio-USB-opslag aan bij de eerste opslag.
  Daar staan bankbestanden C0A.dat/C0B.dat t/m C7A.dat/C7B.dat. Verplaats of
  wijzig deze bestanden niet terwijl de app actief is. Voor een backup:
  sluit de app af en kopieer de hele CGPT3-map via USB naar Windows.
- Opslaan wisselt tussen twee banken met versie, lengte en CRC32. De nieuwe
  bank wordt teruggelezen en gecontroleerd; een mislukte schrijfactie hoort
  de vorige geldige bank te behouden. Dit is met gesimuleerde schrijffouten
  getest, niet met echte stroomonderbrekingen op een Casio.
- 'Niet opgeslagen': het antwoord blijft in RAM en **F3 Bewaar** probeert
  alleen de lokale opslag opnieuw, zonder een tweede API-vraag te versturen.
  Verstuur de vraag niet opnieuw om een opslagfout op te lossen.
- Opnieuw openen na verlaten van de app leest de lokale bestanden. Een gewone
  vervanging van ChatGPT.g3a verwijdert CGPT3 niet.
- V2 bewaarde gesprekken niet blijvend op de Casio. V3 kan daarom **geen oude
  V2- of ChatGPT-websitegesprekken importeren**. De zijbalk begint met de
  gesprekken die vanaf V3 daadwerkelijk zijn opgeslagen.
- Voor vervolgvraag-context herstelt de app maximaal de twee recentste paren
  naar de Atom, binnen diens limiet van 12288 UTF-8-bytes. Openen of teruglezen
  veroorzaakt geen API-completion. Bij de volgende verstuurde vraag telt die
  context wel als API-invoer mee. Je kunt alle acht lokale paren blijven lezen,
  ook wanneer oudere paren niet meer in de API-context passen.
- Bestanden zijn **niet versleuteld** en toegankelijk via Casio-USB-opslag.
  Verwijderen verbergt het gesprek logisch met een nieuw record; dit is geen
  beveiligde fysieke gegevenswissing. Instellingen wissen op de Atom verwijdert
  deze lokale chatbestanden niet.

## Teststatus

Beide programma's zijn gebouwd. Acht softwaretestgroepen met sanitizers
controleren onder meer berichtfouten, aanvraag-ID's, uploadgrenzen, Unicode,
woordomloop, de echte opslagcode met foutinjectie, de Bfile-adapter met
hostapparaatstubs, offline zijbalknavigatie, berichtverwijdering, herstellen
zonder POST en de gedeelde Atom-contextlogica. De framebufferbeelden in
`docs/design/V3-*.png` komen uit de echte UI-code op 384×216 pixels.

Nog niet uitgevoerd: V3 op jouw fysieke Casio/Atom, echte Bfile-opslag en
hervatten na echte stroomuitval, native menutekst en schermranden op jouw OS,
een echte API-aanvraag met herstelde context, en de Windows-scripts op Windows.
Gebruik ook het hardwaretestplan in docs/HARDWARETEST.md.

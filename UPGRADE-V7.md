> Nieuwste versie: [V8 installeren](UPGRADE-V8.md).

# V7 — de app echt verlaten en een gescheiden menunaam

**Alleen `ChatGPT.g3a` vervangen als je Atom V3 gebruikt.** De Atom hoeft niet
opnieuw geprogrammeerd te worden. Je API-sleutel, chats en scherminstellingen
blijven behouden.

## Installeren

1. Pak de V7-ZIP uit naar een nieuwe map op Windows.
2. Sluit de Casio via USB aan en kies **F1 USB Flash**.
3. Vervang het bestaande bestand in de hoofdmap door **dist/ChatGPT.g3a**.
   Gebruik precies die naam en laat geen tweede '(1)'-kopie naast de oude app.
4. Laat de map **CGPT3** met je chats en scherminstellingen staan.
5. Werp het station veilig uit en ontkoppel de Casio-USB-kabel.
6. Open ChatGPT en laat EXE los. Controleer F6 Instellingen → Versie:
   **7.0 / Atom 3.0**. Zie je nog 6.0, dan draait de nieuwe app nog niet.

## Waarom V6 het menu weer overschreef

V6 had een oneindige lus om de hele appsessie. Zodra DisplayMainMenu
terugkeerde, startte de lus automatisch opnieuw en tekende ChatGPT weer
schermvullend. Dat past bij jouw melding: kort witte randen, daarna de app
weer zwart. De eerdere tests namen ten onrechte aan dat zo'n terugkeer altijd
betekende dat de gebruiker de app opnieuw had geopend.

V7 laat main na de menu-aanroep terugkeren naar de OS-aanroeper, zoals in V1.
Er is geen interne herstartlus meer. Alleen een nieuwe OS-start initialiseert
weer een sessie; die initialisatie wist ook oude afsluit- en toetsstatus zodat
heropenen niet meteen opnieuw de afsluitvraag toont.

De verbinding wordt niet vóór de menu-aanroep gesloten. De OS-afsluitcallback
of het opruimen na de menu-aanroep sluit de UART. De Atom blijft fysiek via
USB gevoed; er wordt geen slaapstand of stroomuitschakeling toegevoegd.

## Pictogram en appnaam

- Normaal: witte achtergrond met een zwart logo.
- Geselecteerd: **volledige lichtblauwe achtergrond**, niet alleen een rand.
- Het logo is 32×32 en staat bovenin op (30,4), in plaats van het 44×44-logo
  dat tot vlak bij de naamruimte doorliep.
- De onderkant van de bitmap bevat geen logo of ingebakken tekst. De ruimte
  vanaf y=40 is vrij voor de native naam.
- Casio tekent **ChatGPT** zelf, met zijn eigen normale lettergrootte en
  kleur bij selectie. De app tekent geen tweede naam of eigen lettertype.

De .g3a-iconen blijven 92×64. De werkende donkere randen van het geopende
ChatGPT-scherm zijn behouden. De menuvergelijking uit docs/V6-REFERENCES.md
blijft de bron voor de lichtblauwe selectie; de V6-rand is nu vervangen door
een volledig vlak. Precieze OS-plaatsing van naam en focus moet op jouw Casio
worden bevestigd.

## Kort controleren

1. Controleer eerst dat de versie **7.0** is.
2. F3 moet een Atom-antwoord geven. Keer terug met EXIT.
3. Druk opnieuw EXIT → F1 Ja en laat F1 los. Het gewone Casio-menu moet
   blijven staan. Open **RUN-MAT** om de echte terugkeer te controleren.
4. Open ChatGPT opnieuw. Er hoort geen afsluitvraag vanzelf te verschijnen.
   Herhaal vijfmaal; test ook EXIT → Nee en de fysieke MENU-toets.
5. Loop naar en van ChatGPT in MENU. Controleer het volledig blauwe
   geselecteerde blok en dat de hele naam leesbaar onder het logo staat.
6. Controleer behoud van chats, scherminstellingen en donkere schermranden.

## Testbewijs en beperking

Alle negen softwaretestgroepen met ASan/UBSan slagen. De nieuwe lifecycle-test
laat DisplayMainMenu direct terugkeren en controleert dat main dan retourneert
zonder verdere framebufferoverdracht. Drie losse OS-starts testen opnieuw
openen, oude afsluitstatus, vastgehouden toetsen, veranderd VRAM/DMA en de
UART-volgorde, met en zonder OS-afsluitcallback.

De nieuwe test is daarnaast tegen de echte V6-main uitgevoerd: die faalt
omdat hij weer de editor start zonder nieuwe OS-start. Zie
**docs/logs/v7-menu-regression.log**. De SDK-startcode ondersteunt terugkeren
uit main naar de oorspronkelijke aanroeper:
https://github.com/Jonimoose/libfxcg/blob/daa76d9e37758ab5b9b0f614b7e0aa6acfbe2e18/libfxcg/misc/crt0.S

De .g3a is opnieuw gebouwd en de naamvelden/ingebedde iconen worden bij het
verpakken gecontroleerd. De echte Casio-OS-menuovergang en de precieze
naamweergave zijn hier niet uitgevoerd. Softwaretests vervangen jouw fysieke
controle niet. Er zijn geen betaalde API-aanvragen gedaan.

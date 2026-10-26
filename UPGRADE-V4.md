# V4 — opnieuw openen en schermweergave herstellen

V4 repareert de sluit/heropen-route van de Casio-app en de schermverversing.
Daarnaast kun je de schaal en positie van de app zelf instellen. De functies,
het pictogram en de lokale chats van V3 blijven behouden.

**Heb je Atom-firmware V3? Vervang dan alleen ChatGPT.g3a. Je hoeft de Atom niet
opnieuw te programmeren of je API-sleutel opnieuw in te voeren.** De in dit
pakket meegeleverde Atom-bestanden zijn byte voor byte gelijk aan V3.

## Installeren op Windows

1. Download de complete V4-ZIP en pak hem uit naar een nieuwe map.
2. Ga op de Casio naar MENU en open eventueel eerst een standaardapp, zoals
   RUN-MAT. Open de oude ChatGPT-app niet opnieuw als die in de afsluitlus zit.
3. Sluit de Casio via USB op Windows aan en kies F1 USB Flash.
4. Maak desgewenst een backup van de map **CGPT3**. Daarin staan je V3-chats.
5. Vervang het bestaande bestand in de hoofdmap door het nieuwe
   **dist\ChatGPT.g3a**. Zet geen tweede '(1)'-kopie naast de oude app.
6. Laat de map CGPT3 en de bestanden daarin staan. Die worden niet gemigreerd
   of gewist door deze update.
7. Werp het Casio-station veilig uit en ontkoppel de Casio-USB-kabel.
8. Open ChatGPT en laat de toets waarmee je het opent los. Bij F6 Instellingen
   → Versie hoort **4.0 / Atom 3.0** te staan. Dat verschil is correct.

Als je nog Atom V1/V2 gebruikt, moet je wel eerst de meegeleverde V3-firmware
installeren volgens de eerdere V3-handleiding. V4 heeft geen nieuwe Atom-firmware.

## Schaal en positie zelf instellen

1. Open ChatGPT → **F6 Instellingen → F4 Scherm**.
2. Kies met omhoog/omlaag:
   - **Schaal**: 80% tot 120%; standaard 100%.
   - **Horizontaal**: -24 tot +24 fysieke pixels; standaard 0.
   - **Verticaal**: -24 tot +24 fysieke pixels; standaard 0.
3. Links/rechts verandert de gekozen waarde. Je ziet het resultaat direct.
4. **F2 Test** toont de rand en de vier hoeken TL/TR/BL/BR. EXIT keert terug.
5. **F3 100%** zet schaal en beide verschuivingen terug naar standaard.
6. **F1 of EXE** slaat de instellingen op. EXIT annuleert de wijzigingen.

Begin met 100%, horizontaal 0, verticaal 0. Als een rand op jouw apparaat nog
wegvalt, verklein het beeld enkele procenten of verschuif het. Vergroten boven
100% kan bewust delen van de rand afsnijden. Dit verandert alleen de app,
niet de fysieke schermresolutie of het gewone Casio-menu. De fysieke resolutie
blijft 384×216 en de pixeltekening wordt daarop geschaald.

Scherminstellingen worden apart, met hetzelfde dubbele-bankformaat als chats,
bewaard in **CGPT3/D0A.dat** en **CGPT3/D0B.dat**. Je chats gebruiken de C-bestanden.
Bij een onleesbare of ontbrekende scherminstelling wordt 100% / 0 / 0 gebruikt.
Mocht de interface na verkeerd instellen lastig bereikbaar zijn: F6 → F4 → F3
→ F1 herstelt standaard. Alternatief: sluit de app af en verwijder via USB
alleen D0A.dat en D0B.dat. Verwijder niet de C-bestanden of de hele CGPT3-map.

## Controleer deze twee reparaties op de Casio

1. Open de app, sluit via EXIT → Ja, selecteer hem opnieuw en laat EXE los.
   Er hoort geen afsluitvraag vanzelf te verschijnen.
2. Typ een paar letters, ga met MENU naar het gewone menu en open ChatGPT
   opnieuw. De app moet weer een normale nieuwe invoersessie tonen.
3. Test ook EXIT → Nee: de app hoort open te blijven.
4. Herhaal beide afsluitroutes vijf keer. Controleer je opgeslagen chats via OPTN.
5. Test de vier hoeken bij standaard 100%/0/0 en pas de weergave eventueel aan.
6. Sla een scherminstelling op, sluit af en controleer dat die na opnieuw openen
   nog actief is. Controleer dat de gewone Casio-apps hun eigen weergave houden.

## Wat veranderde technisch?

- De app valt niet meer uit main nadat hij het menu heeft geopend. De normale
  SDK-menuafhandeling wordt gebruikt; als dezelfde app wordt hervat, begint
  expliciet een nieuwe sessie.
- Afsluitstatus, toetsenstatus, UART-parser, schermpointer en lokale indexcache
  worden bij iedere sessie opnieuw geïnitialiseerd. Opgeslagen bestanden blijven
  behouden. Een nog ingedrukte start- of bevestigingstoets wordt niet hergebruikt.
- De 16-bit-kleurmodus wordt via de OS-toetsafhandeling geïnitialiseerd. De
  daarbij geïnjecteerde EXIT wordt intern opgebruikt en opent geen eigen dialoog.
- Voor elk frame worden kleurmodus en het volledige LCD/DMA-venster ingesteld.
  V4 gebruikt de volledige displayoverdracht in plaats van de strookoverdracht.
- Schalen en verschuiven gebeurt in de renderer met begrensde pixelmappen,
  zonder een tweede groot framebuffer in het toepassings-RAM.

## Teststatus

De Casio-add-in is daadwerkelijk opnieuw gecompileerd. Alle negen softwaretest-
groepen slagen met AddressSanitizer en UndefinedBehaviorSanitizer. De echte
appcontroller is in hostapparaatstubs driemaal achter elkaar gesloten/heropend,
met vastgehouden starttoetsen, oude afsluitstatus, een ander VRAM-adres en een
geërfd gedeeltelijk DMA-venster. De gesimuleerde LCD-inhoud wordt met de volledige
framebuffer vergeleken; alleen functie-aanroepen tellen is niet de controle.
Schaalbereiken, verschuiving, clipping, opslag, annuleren, reset en mislukte opslag
zijn getest. De Atom-binaries zijn gelijk aan de eerder gebouwde V3-versie.

**V4 is nog niet op een fysieke Casio getest.** Het werkelijke OS-toetsgedrag en
de uiteindelijke LCD-weergave moeten daarom met de bovenstaande stappen op jouw
apparaat worden bevestigd. De voorbeeldbeelden zijn hostrendering, geen foto's.

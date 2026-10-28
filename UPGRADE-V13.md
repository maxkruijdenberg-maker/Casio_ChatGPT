**Nieuwste versie: [V14](UPGRADE-V14.md) met MENU-bevestiging, =-toets, centrale Diagnose en modelkeuze.**

# V13 — verbinding behouden en toetsen apart uitlezen

Deze versie corrigeert het te agressieve sluiten/heropenen uit V12 en verandert
de toetsuitlezing vanwege je melding dat iedere toets de verbinding verbreekt.
Dit is geen bewezen fysieke reparatie; het apparaat moet onderstaande tests doorlopen.

## Installeren

1. Download en pak het hele V13-pakket uit.
2. Verbind de Casio met Windows en kies USB Flash / USB-opslag.
3. Vervang alleen `ChatGPT.g3a` in de hoofdmap door `dist\ChatGPT.g3a` uit V13.
   Geen tweede bestand `ChatGPT (1).g3a` laten staan.
4. Laat `CGPT3` staan: daarin staan je chats en scherminstellingen.
5. Werp veilig uit en ontkoppel de Casio-USB. Herstart de Casio één keer.
6. Laat de Atom via zijn eigen USB gevoed. Open ChatGPT.
7. F6 → Versie toont **13.0 / Atom 11.0**.

**Vanaf V11/V12 hoef je de Atom niet opnieuw te uploaden.** Alle Atom-broncode
én alle vier firmware-/bootbestanden zijn identiek aan de originele V11.
Wifi en API-sleutel blijven behouden. Telefooninstallatie blijft F6 → F2.
Van vóór V11: volg de Atom-stappen uit UPGRADE-V11.md, met de Casio-app uit V13.

## Wat verandert?

### Toetsen lezen zonder OS-polling

De app riep tijdens toetsen scannen steeds PRGM_GetKey_OS aan via libfxcg.
V13 leest alleen de zes KEYSC-datawoorden van de SH7305 (fx-CG50), op dezelfde
manier als de leesroutine van gint. Geen writes naar de keyboardcontroller,
geen nieuwe interrupt handler, geen wijziging van de OS-toetsenroutine.
Daarmee verwijderen we deze OS-aanroep als mogelijke nevenwerking bij toetsen.
Zonder de OS-ROM en een apparaattest is niet bewezen dat de syscall de oorzaak was.

De decoder dekt de 50 fysieke toetsen en behoudt de bestaande PRGM-keycodes.
ALPHA-lock, eenmalige SHIFT, DEL, hoofdletters en pijlen blijven werken.
Nieuwe multi-keychords worden genegeerd; een al vastgehouden toets blijft
herkend en MENU heeft prioriteit. Dit implementeert geen verborgen sneltoets.

Bronnen en registerlayout:
- https://github.com/peltlasvulpine/gint-mirror/blob/2bc6f7ebcb500b423ee9906e11664466ae913424/src/keysc/keysc.c
- https://github.com/peltlasvulpine/gint-mirror/blob/2bc6f7ebcb500b423ee9906e11664466ae913424/include/gint/mpu/keysc.h
- https://github.com/hexbinoct/casio-cg50/blob/b321ae95308a172d94ac3742b2a5e6c59e937744/re/KEYMAP.md

### De poort blijft open waar mogelijk

De app sluit de OS-UART niet meer zelf wanneer je MENU opent. Bij herselectie
wordt dezelfde geopende poort gebruikt. Er blijft geen app-timer, app-ISR of
quit-callback actief in het menu. Alleen de bestaande OS-seriële driver blijft
open; onze app draait niet op de achtergrond. De OS-driver is ook de normale
interface die libfxcg gebruikt voor seriële uitvoer.

Als het OS de poort zelf sluit, of een andere app hem overneemt, moet V13 hem
alsnog opnieuw openen. Absoluut blijvende toegang buiten de app kan een gewone
add-in niet garanderen. Controleer daarom ook andere apps, vooral LINK.

### Vertraging is niet meteen een disconnect

V12 sloot na één gemiste heartbeat (500 ms) de poort. V13 wacht 1200 ms en
houdt de poort bij een losse misser open. Bij een eerder werkende verbinding
verschijnt tijdelijk Herstel, niet meteen Offline. Pas na drie opeenvolgende
mislukte controles wordt offline gemeld en eenmaal de driver opnieuw ingesteld.
Een echte schrijffout of een daadwerkelijk gesloten poort wordt wel hersteld.

Metadata krijgt drie vensters van 1200 ms. Belangrijke opdrachten behouden
4 × 800 ms en dezelfde gecodeerde requestbytes bij retries. Dit voorkomt
onnodige verstoring; het is geen garantie op een bepaalde internetresponstijd.
De achtergrondcontrole blijft gratis HELLO-verkeer, nooit een betaalde API-vraag.
Tijdens continu typen worden die antwoorden vóór de volgende toets verwerkt.

De Atom blijft USB-gevoed. Deze update verandert geen voeding, bedrading of
spanningsniveaus. Extra Casio-batterijverbruik van de open poort is niet gemeten.
Een losse verbinding of herstartende Atom kan software niet betrouwbaar oplossen.

## Test op jouw apparaat

1. Open ChatGPT, typ letters/cijfers en druk ook ALPHA, SHIFT, pijlen en DEL.
   Controleer of de Atom-status bij toetsen stabiel blijft en of Atom zelf herstart.
2. Open Instellingen. Controleer opgeslagen wifi, model en Atom-versie.
3. EXIT → Ja → open opnieuw; herhaal **twintig keer**, ook na de derde keer.
4. Herhaal met fysieke MENU. Open tussendoor RUN-MAT, SYSTEM en LINK.
5. Herstart de Atom terwijl ChatGPT open is: tijdelijk Herstel/Offline mag,
   daarna moet hij automatisch terugkomen zonder opnieuw een vraag te betalen.
6. Als de verbinding toch wegvalt: F3 Diagnose toont nu ook het aantal
   succesvolle poortopeningen en door de app uitgevoerde sluitingen sinds start.
   Noteer die aantallen vóór en na typen/heropenen. Bij laden van een andere app
   kan het OS de app opnieuw starten en daarmee de tellers wissen.
7. Test telefooninstallatie en verstuur zelf één korte vraag wanneer je API-tegoed
   wilt gebruiken. Controleer je opgeslagen chats en scherminstellingen.

Twaalf ASan/UBSan-testgroepen slagen: echte keyboarddecoder met alle 50 toetsen,
geen PRGM-polling in de app, vertraagde metadata zonder sluiten, continu typen
met vertraagde heartbeat-ACK, drie-misserbeleid en 40 gesimuleerde menu-sessies
(20 per native route), inclusief een extern gesloten poort. SH4-build beschikbaar.
Er is hier geen fysieke Casio/Atom of OS-ROM; deze tests bewijzen niet dat de
werkelijke hardwareverbinding of het menu van OS 03.81 zich exact zo gedraagt.

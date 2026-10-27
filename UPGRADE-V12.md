**Opgevolgd door [V13](UPGRADE-V13.md): blijvende poort en nieuwe toetsuitlezing. De korte V12-timeouts zijn teruggedraaid.**

# V12 — sneller en automatisch opnieuw verbinden

V12 gebruikt het normale ChatGPT-pictogram en de telefooninstallatie uit de
**originele V11**. De wijzigingen zitten in de Casio-app; de Atom blijft V11.

## Installeren vanaf V11

1. Download en pak het V12-pakket volledig uit.
2. Verbind de Casio met Windows en kies USB Flash / USB-opslag.
3. Vervang `ChatGPT.g3a` in de hoofdmap door `dist\ChatGPT.g3a` uit V12.
   Voorkom een dubbele `ChatGPT (1).g3a`.
4. Laat de map `CGPT3` staan. Hierin staan chats en scherminstellingen.
5. Werp het station veilig uit, ontkoppel Casio-USB en herstart één keer.
6. Laat de Atom gevoed via zijn eigen USB. Open ChatGPT: automatisch verbinden.
7. F6 → Versie toont **12.0 / Atom 11.0**.

**De Atom hoeft vanaf V11 niet opnieuw geüpload te worden.** Zijn broncode,
firmware, bootloader en partities zijn identiek aan de originele V11. Wifi en
API-sleutel blijven behouden. Ontbreken die gegevens? F6 → F2 Telefoon.

Van V10 of ouder? Gebruik de Atom-installatiestappen uit UPGRADE-V11.md,
maar plaats de Casio-app uit dit V12-pakket. Arduino IDE 2.x, ESP32 3.3.0,
ArduinoJson 7.4.2, board M5Atom, 115200 en flash wissen Disabled.

## Wat is opgelost?

V11 weigerde een seriële poort die volgens de OS-driver nog open was, terwijl
onze app hem niet als geopend kende. Daardoor konden alle volgende pogingen
mislukken, bijvoorbeeld na een menu-/USB-overgang. V12 neemt de poort opnieuw
in beheer en stelt hem in op dezelfde 9600 baud, 8N1, zonder inversie.

Omgekeerd wordt ook een door het OS gesloten poort opnieuw geopend als onze
lokale vlag nog 'open' zegt. Een schrijffout of onbeantwoorde achtergrondprobe
maakt de transportstatus schoon, zodat de volgende poging opnieuw opent.

| Controle | V11 | V12 |
|---|---:|---:|
| Interval start offline achtergrondpoging | 3 seconden | 750 ms |
| Antwoordvenster achtergrondprobe | 600 ms | 500 ms |
| Online heartbeat-interval | 15 seconden | 10 seconden |
| Read-only informatie: antwoordvensters | 4 × 800 ms | 2 × 300 ms |

Dit zijn softwaretimers, geen gegarandeerde apparaatresponstijden. Drivercalls,
rendering, andere opdrachten en het opstarten van de Atom kunnen tijd toevoegen.
Het typen blijft tijdens achtergrondprobes beschikbaar. Instellingen herstelt
nu ook automatisch wanneer de Atom later beschikbaar komt; F3 blijft diagnose.

Belangrijke opdrachten (configuratie, chat, uploads, API-status enz.) behouden
hun bestaande vier antwoordvensters van 800 ms. Een retry gebruikt dezelfde
gecodeerde bytes en sequence. Er wordt geen nieuwe betaalde vraag gemaakt om
een verbinding te testen. De bestaande request-ledger blijft ongewijzigd.

UART wordt bij verlaten nog steeds vóór de menuoverdracht gesloten. Dat is
bedoeld: de app opent hem automatisch opnieuw bij heropenen. De Atom zelf
blijft aan zolang zijn USB voeding geeft. V12 verandert geen bedrading of voeding.

## Controle op jouw Casio

1. Laat de Atom via USB gevoed. Open ChatGPT zonder F3: wacht op Atom OK.
2. EXIT → Ja → open opnieuw. Herhaal tien keer.
3. Open tussendoor RUN-MAT en SYSTEM, en keer terug naar ChatGPT.
4. Open Instellingen zonder Atom-voeding. Sluit Atom-USB weer aan terwijl je
   op dat scherm blijft: de status en instellingen moeten automatisch terugkomen.
5. Controleer telefooninstallatie, één vraag, ALPHA/SHIFT, chatgeschiedenis
   en scherminstellingen. Bij onzekere API-uitkomst niet blind opnieuw versturen.

Elf hosttestgroepen met ASan/UBSan controleren onder meer de twee tegenstrijdige
poorttoestanden, open-/schrijffouten, identieke retrybytes, offline timers,
herstel in Instellingen en zes sessies over de twee menuroutes. De vorige
mock gaf altijd Serial_IsOpen=0; dat model is nu vervangen door een werkelijke
open/dicht-status. De SH4-app is gebouwd. Er is hier geen fysieke Casio/Atom;
de bovenstaande apparaattest moet het herstel op jouw opstelling bevestigen.

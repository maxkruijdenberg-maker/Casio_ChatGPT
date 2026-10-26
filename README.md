# ChatGPT voor Casio fx-CG50 + originele M5Stack Atom Lite

Deze versie bevat twee echte programma's: `dist/ChatGPT.g3a` voor de Casio en
Arduino-firmware voor de ESP32-PICO-D4 in de Atom Lite. De Atom verstuurt HTTPS-
verzoeken naar de OpenAI API; het taalmodel draait bij OpenAI. Slaapstand is op
verzoek volledig weggelaten. Er staan geen API-sleutels of hotspotwachtwoorden
in dit project. Instellingen worden met de Casio-toetsen ingevoerd.

**Status:** beide programma's zijn in Linux gecompileerd. De gedeelde transport-
en tekstfuncties, aanvraagregistratie en echte editorcode zijn softwarematig
getest. Er is nog geen test op een Casio/Atom of met een echte API-sleutel gedaan.
De Windows-hulpmiddelen zijn geschreven en statisch gecontroleerd, niet op
Windows uitgevoerd. Deze oplevering is een fysiek te valideren breadboardversie.

## Windows: installeren

1. Pak de ZIP uit naar bijvoorbeeld `C:\CasioChatGPT`.
2. Koppel Casio TX en RX tijdelijk los van de Atom. GND mag blijven zitten.
3. Installeer Arduino IDE 2.x via https://www.arduino.cc/en/software/.
4. Voeg bij Bestand > Voorkeuren > Extra Boardbeheer-URL's toe:
   `https://espressif.github.io/arduino-esp32/package_esp32_index.json`.
5. Installeer in Boardbeheer **esp32 by Espressif Systems 3.3.0**.
6. Installeer in Bibliotheekbeheer **ArduinoJson by Benoit Blanchon 7.4.2**.
7. Open `atom\ChatGPTAtom\ChatGPTAtom.ino`. De bestanden `wire.h`, `ledger.h`,
   `hardware.h` en `trust.h` moeten in dezelfde map blijven staan.
8. Selecteer board **M5Atom** (originele ESP32, geen M5AtomS3), de bijbehorende
   COM-poort en uploadsnelheid 115200. Compileer en upload. De firmware gebruikt
   geen M5Stack-displaybibliotheek of USB-configuratieprogramma.
9. Sluit de Serial Monitor. Verbind TX en RX opnieuw volgens de eerder
   gecontroleerde aansluitingen.
10. Sluit de Casio via zijn USB-kabel aan op Windows en kies op de Casio
    **USB Flash / USB-opslag**. Kopieer `dist\ChatGPT.g3a` naar de hoofdmap van
    het Casio-station. Werp het station veilig uit en ontkoppel de kabel.
11. Open in MENU het pictogram **ChatGPT**. Druk F3 voor de UART-diagnose.
12. Open F6 Instellingen. Voer hotspotnaam, hotspotwachtwoord en API-sleutel in.
    Gebruik een 2,4GHz-hotspot. Kies desgewenst een ander model. Standaard:
    `gpt-4.1-mini`. De firmware controleert dit model bij de API voordat de
    betaalde vraag wordt verstuurd. Beschikbaarheid voor jouw account is hier
    nog niet gecontroleerd.
13. Typ een korte vraag en druk F1. De eerste verbinding synchroniseert ook
    de klok voor certificaatcontrole. Zie `docs/HARDWARETEST.md` voor de tests.

OpenAI API-gebruik wordt afzonderlijk betaald. Een ChatGPT-abonnement geeft
niet automatisch API-tegoed. De app gebruikt geen bestaande ChatGPT-gesprekken.
Maak en beheer de sleutel zelf via je OpenAI-platformaccount. Voer hem alleen
lokaal in op de Casio, niet in chat, broncode of een screenshot.

### Voorgebouwde firmware flashen zonder Arduino-build

Installeer Python 3 voor Windows van https://www.python.org/downloads/windows/.
Open PowerShell in de uitgepakte projectmap:

```powershell
py -m pip install esptool==5.0.0
py tools\flash_windows.py COM5
```

Vervang COM5 door de Atom-poort in Apparaatbeheer. TX en RX blijven tijdens
flashen los. Het hulpmiddel controleert de meegeleverde SHA-256-waarden en
schrijft bootloader, partitietabel, boot_app0 en applicatie naar hun offsets.
Het wist niet doelbewust de NVS-instellingen; maak voor een nieuwe ingebruikname
Instellingen leeg als er al andere firmware op het bord stond. Bij een
afwijkende eerdere partitietabel kunnen instellingen toch verloren gaan.
Als de poort ontbreekt: controleer de USB-datakabel en de USB-serieeldriver voor
jouw exemplaar. Installeer geen willekeurige driver van een onbekende website.

## Bediening

| Toets | Invoerscherm |
|---|---|
| SHIFT | Letterstand vast aan/uit, geen herhaalde SHIFT nodig |
| ALPHA of F5 | Hoofdletters/kleine letters |
| F1 of EXE | Versturen; in Instellingen: opslaan |
| F2 | Nieuw gesprek, na bevestiging |
| F3 | UART-diagnose |
| F4 | Tekenkeuzescherm; pijlen kiezen, EXE invoegen |
| F6 | Instellingen |
| Links/rechts | Cursor naar vorige/volgende volledige letter |
| Omhoog/omlaag | Cursor ongeveer een schermregel verplaatsen |
| DEL | Vorige letter verwijderen |
| AC | Invoerveld wissen, na bevestiging |
| EXIT | Terug, annuleren of afsluiten met bevestiging |
| MENU | UART sluiten en terug naar het Casio-menu |

De letterlabels op de normale toetsen worden gebruikt. Op het scherm staat
123, abc, ABC of Abc. De eerste gewone letter van een nieuwe vraag wordt een
hoofdletter, daarna volgen kleine letters. Instellingen veranderen nooit
automatisch de letterkast. In het tekenkeuzescherm zijn alle afdrukbare
ASCII-tekens beschikbaar, inclusief spatie, streepje en underscore voor sleutels.
Het standaard GetKey-invoerveld verwerkt SHIFT intern; daarom gebruikt deze app
een eigen editor met de fysieke Casio-toetsen voor de afgesproken SHIFT-lock.

Na versturen komt een apart antwoordscherm. Omhoog/omlaag scrolt een regel;
links/rechts scrolt een pagina. F1 begint een vervolgvraag. F2 wist het gesprek
na bevestiging. EXIT tijdens wachten vraagt annulering aan. Een al verstuurde
API-aanvraag kan nog kosten maken. Een netwerkbewerking kan nog tot zijn
netwerk-time-out bezig blijven; ondertussen wordt een nieuwe vraag geweigerd.
Bij onzekere transportuitkomst wordt niet automatisch opnieuw betaald gevraagd.

Instellingen toont alleen of een waarde is ingesteld. De sleutel en het
hotspotwachtwoord worden afgeschermd ingevoerd en niet teruggelezen. Kies de
instelling opnieuw om deze te vervangen. Een leeg opgeslagen veld wist die
waarde. 'Alle instellingen wissen' verwijdert alle vier instellingen op de Atom.
De enige taalkeuze in deze versie is Nederlands.

## Limieten en opslag

- Vraag: maximaal **1024 UTF-8-bytes**. Antwoord: maximaal **8192 UTF-8-bytes**.
- Hotspotnaam: 32 bytes; wachtwoord: 63 bytes; sleutel: 256 ASCII-bytes;
  modelnaam: 80 ASCII-bytes. De hotspot kan ook zonder wachtwoord zijn.
- Maximaal 1200 completion-tokens per vraag. Het model wordt om beknopte platte
  Nederlandse tekst gevraagd. Overschrijding van de antwoordbuffer geeft een
  fout; tokenafbreking wordt zichtbaar gemeld.
- Context: maximaal twee eerdere vraag/antwoordparen en gezamenlijk maximaal
  12288 UTF-8-bytes. Het oudste paar wordt zo nodig gewist. Het antwoordscherm
  meldt dat oude context is verwijderd. Reset, instellingen wijzigen of
  'Nieuw gesprek' verwijdert de context; de instellingen blijven bewaard.
- Het bitmaplettertype bevat ASCII en de expliciete tekens in `casio/font.h`:
  Nederlandse accenten, enkele andere Latijnse letters, Griekse letters en
  veelgebruikte wiskundige tekens. Dit is **niet de volledige Casio-tekentabel**.
  Andere antwoordtekens krijgen een zichtbaar vervangingsteken. Onbekende
  invoertoetsen geven een melding. Formules zijn lineair, zonder LaTeX-renderer.
- De editor, cursor en verwijderfunctie bewaren UTF-8-grenzen. Transportchunks
  kunnen een teken splitsen; decodering gebeurt pas na samenvoegen.
- Instellingen staan in gewone ESP32 NVS-flash, **zonder flashversleuteling**.
  Afscherming op het scherm beschermt niet tegen fysieke toegang tot de Atom.
  Geheimen gaan over de directe UART **zonder encryptie**, daarna via HTTPS
  met certificaatcontrole naar OpenAI. Configuratie- en USB-debuglogs zijn uit.
- De Atom blijft via USB gevoed. GPIO26 is tijdens actieve firmware UART-TX
  met hoog rustniveau. TX wordt niet per bericht losgekoppeld. Softwarematig
  loskoppelen kan via de ESP32-pinmatrix, maar dat vereist apart testen van
  heraansluiten en UART-timing. Het geeft geen elektrische isolatie wanneer
  een apparaat uitstaat. De weerstanden van 1 kΩ zijn geen levelshifters en
  verhinderen terugvoeding niet volledig. Gebruik de gecontroleerde hardware.

## Transport en API

9600 baud, 8N1; Atom RX32, TX26, geen inversie. Stop-and-wait met berichten
van maximaal 24 payloadbytes, versie/type/volgnummer/lengte, HDLC-achtige
escaping, CRC16-CCITT en maximaal vier transportpogingen. Een upload gebruikt
expliciete byte-offsets. Ontvangen ACK's worden op type en volgnummer gecontroleerd.

Ieder betaald verzoek krijgt vooraf een door NVS opgeslagen oplopend ID.
De Atom registreert dat ID duurzaam **vóór** de HTTP POST. Identieke
transportherhalingen krijgen de vorige reactie. Ook na een reset start hetzelfde
gecommitte ID geen nieuwe POST. Een reset tussen registratie en POST kan de
vraag laten vervallen: de garantie is hoogstens één poging per ID, niet dat
een vraag altijd aankomt. Een expliciet opnieuw verstuurde vraag krijgt een
nieuw ID en kan opnieuw kosten veroorzaken.

De gebruikte officiële API-specificatie beschrijft `GET /v1/models/{model}`
en `POST /v1/chat/completions`, met `messages`, `max_completion_tokens` en
`choices[0].message.content`. Modelnaam is instelbaar. HTTPS gebruikt ingebedde
Mozilla-rootcertificaten (GTS R1/R2/R3/R4, DigiCert Global Root G2, ISRG Root X1)
en NTP-tijd via pool.ntp.org/time.google.com. Certificaatcontrole wordt niet
uitgeschakeld. Een toekomstige wijziging van certificaatketen of API kan een
firmware-update vergen. Zie `docs/VERSIONS.md` voor bronnen en versies.

## Zelf de Casio-app bouwen op Windows

Download de officiële **PrizmSDK-win-0.6.zip** van
https://github.com/Jonimoose/libfxcg/releases/tag/v0.6 en pak hem uit.
De SDK-map moet onder meer `bin\sh3eb-elf-gcc.exe`, `bin\mkg3a.exe`,
`include`, `lib` en `toolchain\prizm.x` bevatten.

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File tools\build-casio.ps1 -SdkPath C:\PrizmSDK-win-0.6
```

Dit bouwt broncode en pakt de eigen menu-iconen in tot `dist\ChatGPT.g3a`.
Het PowerShell-script moet nog op Windows worden uitgevoerd/gevalideerd.
De meegeleverde G3A is in deze Linux-omgeving gebouwd met een SH4-crosscompiler,
de originele SDK 0.6-bibliotheken/linkerscript en mkg3a.

## Softwaretests en bekende grenzen

Op Linux met een C-compiler: `sh tools/test.sh`. De tests gebruiken AddressSanitizer
en UndefinedBehaviorSanitizer. Resultaten staan in `docs/logs/tests.log`.
Transport en UTF-8: 19748 asserts; aanvraagregistratie: 10000 cycli met herhaling,
reset en opslagfalen; editor: SHIFT-lock, hoofdletters, exacte sleutelletterkast,
maskering, cijferstand en UTF-8-bewerking. De editor-test gebruikt expliciete
hostapparaatstubs; dat is geen fysieke toetsentest en geen API-test.

De twee bouwlogs en het Casio-linkoverzicht staan in `docs/logs`. De
Arduino-build gebruikt lokaal geïnstalleerde officiële packages omdat de
CLI in deze cloudomgeving zijn netwerkindexen niet kon ophalen. De zichtbare
index/discovery-waarschuwingen hebben de succesvolle build niet verhinderd.

Nog **niet fysiek gevalideerd**: keymatrix op jouw OS-versie, schermweergave,
UART-open/sluiten, echte buffers en kabels, hotspot/NTP, certificaatketen op de
Atom, modeltoegang/tegoed, echte antwoorden, annulering en opnieuw openen.
Er worden geen gesimuleerde antwoorden als productfunctionaliteit gebruikt.
Slaapstand en de latere XIAO ESP32-C6/PCB vallen buiten deze versie.

In de voorbereide cloudmachine kun je met `bash tools/build-cloud.sh` beide programma's opnieuw bouwen. Gebruik de bestaande checkout; een Git-worktree is niet nodig. De toolchains staan buiten de checkout in `/workspace/casio-tools`. Deze Linux-helper vereist die voorbereide toolchains; gebruik op Windows de bovenstaande IDE en SDK-stappen.

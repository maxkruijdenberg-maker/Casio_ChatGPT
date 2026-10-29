# ChatGPT V20.0 voor Casio fx-CG50 + originele M5Stack Atom Lite

De donkere uitvoering van V19, met een diepblauwe achtergrond, rustige kaarten,
mintgroene acties en lila-accenten. Alle tekst-, selectie- en statuskleuren zijn
mee aangepast; dit is geen omgekeerd licht thema.

- Abstract logo en laadanimatie met twee bewegende accenten.
- Gecentreerde kop en ondertekst, met meer onderscheid tussen beide.
- Gekleurde selectieranden op donkere, getinte kaarten.
- Groene Verbonden-badge en rode Verbroken-badge, beide met leesbare tekst.
- Dezelfde bediening, Hulp, accountmodellen, chats en =-weergave als V19.

**Vanaf V17 t/m V19 vervang je alleen `dist/ChatGPT.g3a`. De Atom blijft V17.0.**
Laat `CGPT3` staan. Vanaf V16 of ouder is de Atom-update uit UPGRADE-V17.md nodig.
Zie [UPGRADE-V20.md](UPGRADE-V20.md) en [de gebruikersgids](docs/V20-STARTGIDS.md).

[Bekijk de donkere V20-schermen](docs/design/V20-actual-screens.png),
[de rode statusbadge](docs/design/V20-offline.png) en
[de laadanimatie](docs/design/V20-loading.gif). Dit zijn renders uit de echte
UI-hosttests; de modellen zijn voorbeelddata en geen privéaccount.

De verbindingsstatus beschrijft lokaal contact met de Atom, geen garantie voor
internet of API-tegoed. Niet ieder accountmodel ondersteunt de gebruikte Chat
Completions API. Veertien softwaretestgroepen en de Casio-build controleren de
software; fysieke controle op jouw apparaat blijft nodig.

## Windows: installeren

1. Pak de ZIP uit naar bijvoorbeeld `C:\CasioChatGPT`.
2. Koppel Casio TX en RX tijdelijk los van de Atom. GND mag blijven zitten.
3. Installeer Arduino IDE 2.x via https://www.arduino.cc/en/software/.
4. Voeg bij Bestand > Voorkeuren > Extra Boardbeheer-URL's toe:
   `https://espressif.github.io/arduino-esp32/package_esp32_index.json`.
5. Installeer in Boardbeheer **esp32 by Espressif Systems 3.3.0**.
6. Installeer in Bibliotheekbeheer **ArduinoJson by Benoit Blanchon 7.4.2**.
7. Open `atom\ChatGPTAtom\ChatGPTAtom.ino`. Laat **alle `.h`-bestanden** in dezelfde sketchmap staan. De firmware bevat extra
   gedeelde headers; alleen het `.ino`-bestand overnemen is niet voldoende.
8. Selecteer board **M5Atom** (originele ESP32, geen M5AtomS3), de bijbehorende
   COM-poort en uploadsnelheid 115200. Compileer en upload. De firmware gebruikt
   geen M5Stack-displaybibliotheek of USB-configuratieprogramma.
9. Sluit de Serial Monitor. Verbind TX en RX opnieuw volgens de eerder
   gecontroleerde aansluitingen.
10. Sluit de Casio via zijn USB-kabel aan op Windows en kies op de Casio
    **USB Flash / USB-opslag**. Kopieer `dist\ChatGPT.g3a` naar de hoofdmap van
    het Casio-station. Werp het station veilig uit en ontkoppel de kabel.
11. Open in MENU het pictogram **ChatGPT**. Open Instellingen → Hulp → Verbinding controleren voor een controle.
12. Open **F6 Instel → Wifi en account → EXE of pijl rechts**. Verbind je telefoon met het netwerk
    en wachtwoord op de Casio. Open **http://192.168.4.1**, vul je 2,4GHz-wifi in
    en plak je volledige OpenAI API-sleutel. Laat de Casio open tot de wifi-status
    verschijnt. Het model blijft je bestaande keuze (standaard `gpt-4.1-mini`).
13. Typ een korte vraag en druk F1. De eerste verbinding synchroniseert ook
    de klok voor certificaatcontrole. Zie `docs/HARDWARETEST.md` voor de tests.

OpenAI API-gebruik wordt afzonderlijk betaald. Een ChatGPT-abonnement geeft
niet automatisch API-tegoed. De app gebruikt geen bestaande ChatGPT-gesprekken.
Maak en beheer de sleutel zelf via je OpenAI-platformaccount. Plak hem alleen op de lokale installatiepagina op je telefoon, niet in chat, broncode of een screenshot.

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
| ALPHA | Lettervergrendeling aan/uit; eenmaal drukken is genoeg |
| SHIFT | Eenmalige gele functie, bijvoorbeeld sin → asin; nogmaals annuleert SHIFT |
| F5 | Hoofdletters/kleine letters |
| F1 of EXE | Versturen; in Instellingen: opslaan |
| F2 | Nieuw gesprek; vraagt bevestiging als een concept verloren zou gaan |
| F3 | Chats openen |
| F4 | Tekenkeuzescherm; pijlen kiezen, EXE invoegen |
| F6 | Instellingen |
| Links/rechts | Cursor naar vorige/volgende volledige letter |
| Omhoog/omlaag | Cursor ongeveer een schermregel verplaatsen |
| DEL | Vorige letter verwijderen |
| AC | Invoerveld wissen, na bevestiging |
| EXIT | Terug/annuleren binnen de app; op het beginscherm blijft je concept staan |
| MENU | Vraagt om afsluiten; Blijven bewaart de huidige invoer en verbinding |

De letterlabels op de normale toetsen worden gebruikt. Op het scherm staat
123, abc, ABC of Abc. De eerste gewone letter van een nieuwe vraag wordt een
hoofdletter, daarna volgen kleine letters. Instellingen veranderen nooit
automatisch de letterkast. In het tekenkeuzescherm zijn alle afdrukbare
ASCII-tekens beschikbaar, inclusief spatie, streepje en underscore voor sleutels.
De eigen editor gebruikt de rode letterlabels bij ALPHA-lock. In cijferstand
voeren de wiskundetoetsen functietekst in; SHIFT gebruikt eenmalig de gele
functie. Bijvoorbeeld sin → `sin(`, SHIFT+sin → `asin(`, SHIFT+x² → `sqrt(`
en SHIFT+×10ˣ → `π`. Zie UPGRADE-V10.md voor de volledige tabel. Functies zijn
vraagtekst voor ChatGPT, geen lokale berekening of native 2D-Casio-formule.

Bij openen/heropenen start automatisch een gratis UART-HELLO. Tijdens wachten
op toetsen in het invoerscherm volgen nieuwe controles bij herstel (minimaal 1 seconde tussen startmomenten)
en een controle iedere 10 seconden als de Atom online is. Het typen blijft
beschikbaar. Verbinding controleren staat onder Instellingen → Hulp; verbinden vereist geen handmatige diagnose.

Na versturen komt een apart antwoordscherm. Omhoog/omlaag scrolt een regel;
links/rechts scrolt een pagina. F1 Verder begint een vervolgvraag. F2 begint een nieuw gesprek; de oude
opgeslagen chat blijft behouden. EXIT tijdens wachten vraagt annulering aan. Een al verstuurde
API-aanvraag kan nog kosten maken. Een netwerkbewerking kan nog tot zijn
netwerk-time-out bezig blijven; ondertussen wordt een nieuwe vraag geweigerd.
Bij onzekere transportuitkomst wordt niet automatisch opnieuw betaald gevraagd.

Instellingen toont vier duidelijke keuzes. Wifi en account opent de telefooninstallatie; Model opent de keuzelijst. Je wifi-wachtwoord en API-sleutel worden niet teruggelezen of getoond.
Aanpassen gaat via je telefoon. Via Meer kun je de lokale wifi en
sleutel wissen; opgeslagen chats blijven behouden. Die bevestiging reset ook
de modelkeuze op de Atom. De interface is Nederlands.

Instellingen probeert ook automatisch opnieuw verbinding te maken. Een korte
vertraging sluit de poort niet. Hulp → Verbinding controleren geeft uitleg in gewone taal.
Via **Wifi en account** kun je wifi en API-sleutel invullen via de tijdelijke
installatiepagina. Zie [UPGRADE-V17.md](UPGRADE-V17.md) voor de stappen.

## Lokale chatgeschiedenis

Open de zijbalk met **OPTN**, of bij een antwoord met **F4 Chats**. Acht
lokale gesprekken met elk maximaal acht complete vraag/antwoordparen worden
in Casio-opslag bewaard. F5/F6 bladeren door eerdere paren; SHIFT toont de
volledige vraag. Je kunt oude chats offline lezen en later vervolgen met
begrensde context. Zie UPGRADE-V3.md voor limieten, backups en herstel.
V2- en ChatGPT-websitegesprekken worden niet geïmporteerd.

## Scherm instellen

F6 Instellingen → Meer → Scherm aanpassen. Pijlen omhoog/omlaag kiezen schaal of verschuiving;
links/rechts past aan. Schaal: 80–120%, verschuiving: -24 tot +24 pixels.
F1 slaat op, F2 toont de hoekentest, F3 herstelt 100% / 0 / 0, EXIT annuleert.
Alleen de app wordt aangepast. Zie UPGRADE-V4.md voor herstel en grenzen.

## Limieten en opslag

- Vraag: maximaal **1024 UTF-8-bytes**. Antwoord: maximaal **8192 UTF-8-bytes**.
- Hotspotnaam: 32 bytes; wachtwoord: 63 bytes; sleutel: 256 ASCII-bytes;
  modelnaam: 80 ASCII-bytes. De hotspot kan ook zonder wachtwoord zijn.
- Maximaal 1200 completion-tokens per vraag. Het model wordt om beknopte platte
  Nederlandse tekst gevraagd. Overschrijding van de antwoordbuffer geeft een
  fout; tokenafbreking wordt zichtbaar gemeld.
- API-context: maximaal twee eerdere vraag/antwoordparen en gezamenlijk maximaal
  12288 UTF-8-bytes. Het oudste paar wordt zo nodig gewist. Het antwoordscherm
  meldt dat oude context is verwijderd. Reset, instellingen wijzigen of
  'Nieuw gesprek' verwijdert de Atom-context; de instellingen blijven bewaard.
  V3 bewaart daarnaast afzonderlijke lokale chatbestanden en herstelt de
  recente context wanneer je een opgeslagen gesprek vervolgt.
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
  met certificaatcontrole naar OpenAI. USB-debuglogs zijn uit. De tijdelijke installatiepagina toont geen opgeslagen geheimen.
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
reset en opslagfalen; editor: ALPHA-lock en eenmalige SHIFT-functies, hoofdletters, exacte sleutelletterkast,
maskering, cijferstand en UTF-8-bewerking. De editor-test gebruikt expliciete
hostapparaatstubs; dat is geen fysieke toetsentest en geen API-test. V3 test
ook de echte upload-state-machine, woordomloop, offline instellingen,
teruglezen van het bestaande model en de gedeelde OpenAI-responseparser
(inclusief weigering, limieten, afbreking en ingebedde NUL).

De bouwlogs en het Casio-linkoverzicht staan in `docs/logs`. De
Arduino-build gebruikt lokaal geïnstalleerde officiële packages omdat de
CLI in deze cloudomgeving zijn netwerkindexen niet kon ophalen. De zichtbare
index/discovery-waarschuwingen hebben de succesvolle build niet verhinderd.

Nog **niet fysiek gevalideerd**: keymatrix op jouw OS-versie, schermweergave,
UART-open/sluiten, echte buffers en kabels, hotspot/NTP, certificaatketen op de
Atom, modeltoegang/tegoed, echte antwoorden, annulering en opnieuw openen.
Er worden geen gesimuleerde antwoorden als productfunctionaliteit gebruikt.
Slaapstand en de latere XIAO ESP32-C6/PCB vallen buiten deze versie.

In de voorbereide cloudmachine bouw je alleen de Casio-app met `bash tools/build-casio.sh` (inclusief `casio/native_syscalls.S`). Met `bash tools/build-cloud.sh` bouw je beide programma's opnieuw. Gebruik de bestaande checkout; een Git-worktree is niet nodig. De toolchains staan buiten de checkout in `/workspace/casio-tools`. Deze Linux-helper vereist die voorbereide toolchains; gebruik op Windows de bovenstaande IDE en SDK-stappen.

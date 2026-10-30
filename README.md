# ChatGPT V32.3 voor Casio fx-CG50 + originele M5Stack Atom Lite

[Installatiepakket](dist/ChatGPT-V32.3-installatie.zip) · [START-HIER](START-HIER.md) ·
[PDF](docs/Handleiding-V32.3.pdf) · [Offline handleiding](docs/Handleiding-V32.3.html)

- Alle 30 roze invoerlabels en 22 gele invoerfuncties zijn gecontroleerd tegen de foto.
- CLIP, PASTE, INS en UNDO werken lokaal in het invoerveld; native Casio-systeemacties voegen geen verkeerde tekens toe.
- SHIFT werkt eenmalig bij aantikken én bij vasthouden met een functietoets.
- SHIFT + ( voegt ∛( in voor de derdemachtswortel.
- Nieuwe antwoorden openen bij de eerste zin, met het ChatGPT-label erboven.
- Omhoog/omlaag blijft je gesprek lezen; typen laat de weergave niet naar het einde springen.
- De vroege EXIT/F1-annulering uit V32 blijft behouden.
- Een afgebroken voorbereiding gaat niet later alsnog automatisch verder met versturen.
- Netwerkwerk houdt de bestaande totale tijdslimiet en annuleringscontrole.
- Accountcontrole stoppen vraagt ook de Atom om te stoppen; een nog lopende afronding blijft zichtbaar.
- Modelkeuze toont drie GPT-6-aanbevelingen en daaronder Overige modellen voor de achttien andere keuzes.
- De onderste Open/Terug-hints zijn weg; de fysieke pijlen blijven werken.
- In Chats zijn F1 Wis en F2 Nieuw de twee actieknoppen.
- EXIT gaat vanuit een gesprek lokaal naar het beginscherm; opgeslagen chats blijven bewaard.
- Tekens bevat veertig basissymbolen; SP is spatie en het lage streepje (_) is zichtbaar.
- Openen en teruggaan met rechts/links of EXE/EXIT; Hulp staat alleen in Instellingen.
- Doorlopende chat, automatische zinshoofdletters en blijvende ALPHA-lock behouden.

**Atom V31 al geïnstalleerd?** Vervang alleen dist/ChatGPT.g3a op de Casio,
laat CGPT3 staan en herstart de Casio eenmaal. De meegeleverde Atom-code en
firmware zijn ongewijzigd V31. Voor een oudere Atom upload je eerst de
meegeleverde V31-firmware zonder volledig wissen. Zo blijven wifi, API-sleutel en
opgeslagen chats normaal behouden.
[Bijwerken](UPGRADE-V32.3.md) · [Schermen](docs/design/V32.3-actual-screens.png).

Bestaande chats, acht paren per gesprek en begrensde API-context blijven behouden.
V32.3 is een kandidaat voor praktijkcontrole. Softwaretests zijn geen bewijs dat een
specifiek gebouwd apparaat klaar is voor verkoop. Er is in deze bouwomgeving geen
fysieke Casio/Atom of live betaald API-account getest. Zie de
[praktijk- en overdrachtscontrole](docs/V32.3-CONTROLE-VOOR-VERKOOP.md).

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
    verschijnt. Kies daarna via Instellingen → Model een actuele chatkeuze; een oude modelinstelling wordt niet automatisch vervangen.
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
| SHIFT | Eenmalige gele functie; aantikken of samen vasthouden, nogmaals drukken annuleert de klaarstaande stand |
| F1 of EXE | Versturen; in Instellingen: opslaan |
| F2 | Nieuw gesprek; vraagt bevestiging als een concept verloren zou gaan |
| F3 | Chats openen |
| F4 | Tekenkeuzescherm; pijlen kiezen, EXE invoegen |
| F6 | Instellingen |
| Links/rechts | Cursor naar vorige/volgende volledige letter |
| Omhoog/omlaag | Door de chatgeschiedenis lezen; in nieuwe invoer de cursor verplaatsen |
| DEL | Vorige letter verwijderen |
| AC | Invoerveld wissen, na bevestiging |
| EXIT | Vanuit een chat naar het beginscherm; op het beginscherm blijft je concept staan |
| MENU | Vraagt om afsluiten; Blijven bewaart de huidige invoer en verbinding |

De letterlabels op de normale toetsen worden gebruikt. Op het scherm staat
123, abc of Abc. ALPHA wisselt tussen cijfers en letters; dit is geen hoofdletterlock.
De eerste gewone letter van een nieuwe vraag wordt een hoofdletter; ook na . ! ?
begint een nieuwe zin automatisch met een hoofdletter. Er is geen handmatige
hoofdletterlock meer, ook niet in Tekens. F4 Tekens toont veertig basissymbolen
voor rekenen en gewone leestekens, zonder alfabet of cijfers. De geselecteerde tegel
is duidelijk omlijnd. SP staat voor spatie; bij selectie verschijnt Spatie.
EXIT keert terug naar de invoer. Wifi en API-sleutel vul je via je telefoon in,
met de exact gewenste letterkast.
De eigen editor gebruikt de rode letterlabels bij ALPHA-lock. In cijferstand
123 voeren de wiskundetoetsen functietekst in. `sin` voegt `sin(` in;
SHIFT + sin voegt `asin(` in. Dat betekent arcsinus, dezelfde inverse functie
als sin⁻¹ op het gele Casio-label. SHIFT + ( voegt `∛(` in voor de derdemachtswortel.
Bijvoorbeeld: `∛(8)` is tekst voor een vraag over de derdemachtswortel van 8.

Tik SHIFT aan en gebruik daarna één functietoets, of houd SHIFT samen met die
toets ingedrukt. Ook gelijktijdig indrukken werkt. De eenmalige SHIFT-stand is
na die functie verbruikt; dezelfde nog vastgehouden SHIFT-toets zet hem niet
opnieuw aan. Het maakt niet uit welke van de twee toetsen je eerst loslaat.
Nogmaals op SHIFT drukken annuleert een klaarstaande SHIFT-stand. ALPHA blijft
wisselen tussen cijfers en letters. Functies zijn vraagtekst voor ChatGPT,
geen lokale berekening of native 2D-Casio-formule.

Bij openen/heropenen start automatisch een gratis UART-HELLO. Tijdens wachten
op toetsen in het invoerscherm volgen nieuwe controles bij herstel (minimaal 1 seconde tussen startmomenten)
en een controle iedere 10 seconden als de Atom online is. Het typen blijft
beschikbaar. Verbinding controleren staat onder Instellingen → Hulp; verbinden vereist geen handmatige diagnose.

Na versturen staat het antwoord boven de invoerbalk in hetzelfde gesprek.
Een nieuw antwoord opent bij het ChatGPT-label en de eerste zin. Dit geldt ook
voor een vervolgvraag. Scroll omlaag om verder te lezen en omhoog om eerdere
berichten te bekijken. Een concept typen springt niet automatisch naar het einde.
Omhoog/omlaag leest de historie; typ direct je vervolgvraag en gebruik F1 Stuur of EXE.
Links/rechts verplaatst de cursor. F2 begint bewust een nieuw gesprek; de oude
opgeslagen chat blijft behouden. EXIT gaat vanuit een chat lokaal terug naar het beginscherm,
zonder laadscherm of Atom-aanvraag. Bij een ongestuurd concept vraagt de app eerst
om bevestiging; een niet-opgeslagen antwoord geeft een aparte waarschuwing.
Op het beginscherm blijft EXIT binnen de app en behoudt het je concept.
EXIT of F1 tijdens laden wordt ook tijdens de voorbereiding verwerkt: bij het
herstellen van een gesprek, uploaden van de vraag en aanvragen van een verzoeknummer.
Annuleren vóór de verstuuropdracht voorkomt dat de vraag alsnog wordt verstuurd.
Het concept blijft beschikbaar. Zodra de verstuuropdracht al naar de Atom is
verzonden, kan een API-vraag wel al gestart zijn. De laadplaneet verdwijnt bij
annuleren en de app vraagt de Atom waar nodig om te stoppen. Een al verstuurde
API-aanvraag kan nog kosten maken. De bestaande Atom V31 beëindigt zijn netwerkwerk bij annulering of de totale tijdslimiet.
Een lopende netwerkleesactie kan nog enkele seconden nodig hebben om terug te
keren. De app wacht op de bevestiging dat de Atom klaar is met stoppen.
Zonder bevestiging verschijnt Stoppen niet bevestigd en blijft een nieuwe vraag
geblokkeerd. Als je later zelf opnieuw Stuur kiest, controleert de app eerst de
vorige taak en vraagt zo nodig nogmaals om stoppen; er start pas een nieuwe vraag
als de Atom weer beschikbaar is.
Bij onzekere transportuitkomst wordt niet automatisch opnieuw betaald gevraagd.

Instellingen toont vier duidelijke keuzes. Wifi en account opent de telefooninstallatie; Model opent de keuzelijst. Je wifi-wachtwoord en API-sleutel worden niet teruggelezen of getoond.
Aanpassen gaat via je telefoon. Via Meer kun je de lokale wifi en
sleutel wissen; opgeslagen chats blijven behouden. Die bevestiging reset ook
de modelkeuze op de Atom. De interface is Nederlands.

Instellingen probeert ook automatisch opnieuw verbinding te maken. Een korte
vertraging sluit de poort niet. Hulp → Verbinding controleren geeft uitleg in gewone taal. Hulp staat alleen in
Instellingen en heeft geen afzonderlijke F-sneltoets. Lijsten openen met EXE of
rechts; links of EXIT gaat terug. De onderste Open/Terug-hints zijn verwijderd;
deze navigatie werkt met de fysieke toetsen.
Via **Wifi en account** kun je wifi en API-sleutel invullen via de tijdelijke
installatiepagina. Zie [UPGRADE-V20.1.md](UPGRADE-V20.1.md) voor de stappen.

## Modellen en accountcontrole

Instellingen → Model toont GPT-6.1 Sol, GPT-6 Luna en GPT-6 Astra, met daaronder
Overige modellen →. Kies die vierde regel met rechts of EXE om de aparte lijst
met de achttien andere ondersteunde modellen te openen. De drie aanbevelingen
staan daar niet dubbel in. Links of EXIT gaat vanuit die lijst terug naar de
drie aanbevelingen, en vanuit het hoofdmodelmenu terug naar Instellingen.
De aanbevelingen zijn appadvies, geen prijs- of prestatiegarantie. Navigeren
verandert je ingestelde model niet. Na accountcontrole tonen beide lijsten
alleen de passende namen uit jouw account; ontbrekende namen worden niet als
beschikbaar toegevoegd. Andere gevonden modellen blijven via Overige modellen
bereikbaar als jouw account geen van de drie aanbevelingen aanbiedt.
Zie docs/V32.3-MODELLEN.md voor namen, bronnen en aangekondigde einddatums.

Omhoog/omlaag kiest een regel. Bij een model geeft F4 Info uitleg en bewaart
F1 Kies, rechts of EXE de keuze. Overige modellen is een groepsregel, geen model;
rechts of EXE opent die lijst en Info is daar niet beschikbaar. F2 Account gebruikt één gestreamde GET met begrensd geheugen.
Na afronden verschijnt kort een groen vinkje bij een geslaagde controle,
ook als geen passende modellen zijn gevonden. Een rood kruis markeert een
mislukte controle; daarna verschijnt de reden. Annuleren toont geen rood kruis.
Bij succes verschijnt Account gecontroleerd; een fout laat herkenbare vaste,
ongecontroleerde keuzes zien. Accountmetadata bewijst geen werkende chatvraag.
Modelrechten, dienstcompatibiliteit, internet en tegoed moeten ook kloppen.

Voor accountcontrole en daadwerkelijk stoppen blijft Atom V31 nodig. EXIT of F1 tijdens
de controle vraagt de Atom om te stoppen. De laadplaneet verdwijnt en de app laat
zien wanneer de afronding nog bezig is; die wordt niet verborgen achter een normaal
klaar-scherm. Zonder bevestiging verschijnt Stoppen niet bevestigd en wordt geen
nieuwe vraag gestart voordat de Atom weer beschikbaar is. Een expliciete nieuwe
poging controleert eerst de oude taak. Er wordt niets automatisch betaald herhaald.

Chatverzoeken hebben een netwerkbudget van 120 seconden; accountcontrole heeft
60 seconden. Een al begonnen SDK-lees- of verbindingsactie kan nog kort doorlopen
binnen zijn eigen begrensde time-out. Dit is geen belofte dat de dienst altijd
binnen die tijd antwoord geeft of dat annuleren op een exact aantal milliseconden
klaar is. Na een stop herstelt de app bij verdergaan de recente context uit de opgeslagen
lokale chat en vervangt daarmee de oude Atom-context.

## Lokale chatgeschiedenis

Open de zijbalk met **OPTN** of **F3 Chats**. In die zijbalk verwijdert **F1 Wis**
na bevestiging het geselecteerde gesprek en begint **F2 Nieuw** een nieuw gesprek.
Buiten de zijbalk blijven F1 Stuur en F2 Nieuw de normale chatknoppen. Acht gesprekken met elk maximaal
acht complete vraag/antwoordparen blijven lokaal bewaard. Lees alle paren met
omhoog/omlaag in dezelfde tijdlijn en typ een vervolgvraag onderaan. De recente
API-context blijft begrensd. Bewaar verschijnt tijdelijk op F3 bij opslagfout.
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
en UndefinedBehaviorSanitizer. Resultaten voor deze versie staan in `docs/logs/v32.3-tests.log`.
De regressieproeven uit V32 voor vroege annulering blijven behouden. Die controleren dat een
annulering tijdens de voorbereiding geen W_ASK-verstuuropdracht laat volgen; de
proeven gebruiken vertraagde host-UART-antwoorden, geen echte API-aanvraag.
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

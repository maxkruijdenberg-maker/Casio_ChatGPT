# Hardwaretest in stappen

Deze controles zijn nog niet uitgevoerd door de ontwikkelomgeving. Eerdere
spannings-, polariteits- en draadidentificatiemetingen zijn door de gebruiker
als correct bevestigd; die claim wordt niet als een eigen meting gerapporteerd.

Stop bij een mislukte stap en noteer wat op het scherm staat. Deel nooit een
API-sleutel of hotspotwachtwoord in foto's, screenshots of logbestanden.

1. **Opstelling bevestigen.** Originele ESP32-PICO-D4 Atom Lite, USB-gevoed;
   Casio op vier AAA-batterijen. Casio-batterijplus niet aan de Atom. Beide
   signaalniveaus en hun rustniveau moeten eerder met gemeenschappelijke GND
   gecontroleerd zijn als geschikt voor 3,3V-ESP32-ingangen. Controleer opnieuw
   als de bedrading of voeding sinds die meting is veranderd.
2. **Bedrading.** Casio TX via 1 kΩ naar G32; G26 via 1 kΩ naar Casio RX;
   GND rechtstreeks. Geen inversie. Jouw breadboard: a5 Atom G32, d5-g5
   weerstand, j5 Casio TX; a10 Atom G26, d10-g10 weerstand, j10 Casio RX;
   a15 Atom GND, e15-f15 brug, j15 Casio GND. Voedingsrails ongebruikt.
   Weerstanden veranderen het spanningsniveau niet en zijn geen isolatie.
3. **Atom flashen.** TX/RX los. Upload de firmware voor M5Atom. Geen S3/C6.
   Upload moet zonder fout voltooien. Sluit Serial Monitor, sluit TX/RX aan.
4. **Casio installeren.** Kopieer ChatGPT.g3a via USB-opslag naar de hoofdmap;
   veilig uitwerpen, USB los. MENU moet ChatGPT met het eigen ChatGPT-knooplogo-
   icoon tonen. Openen moet een invoerscherm geven zonder crash.
5. **UART beide richtingen.** F3: geldige v1-reactie bij 9600 8N1 zonder
   inversie. Herhaal vijf keer. Er mogen normaal geen CRC-fouten of retries
   bij komen. Een geldige antwoordframe controleert beide richtingen tegelijk.
   Test ook Atom uit/los: begrijpelijke fout binnen ongeveer 3,2 seconden
   per RPC; daarna moet de app bruikbaar blijven. UART-diagnose is geen
   vervanging voor signaalmetingen.
6. **Toetsen.** SHIFT één keer: typ met de letterlabels 'Hallo'. Verwacht
   hoofdletter H en daarna kleine letters. Controleer F5-hoofdletters,
   SHIFT terug naar cijfers, spatie, DEL, cursor links/rechts, AC met
   bevestiging. F4: voeg é, ë, π en √ in. Verwijder die één voor één;
   nooit halve of kapotte tekens. Controleer alle alfabetlabels: raw matrix-
   posities zijn softwarematig getest maar nog niet op jouw Casio-OS.
7. **Instellingen.** F6: voer hotspotnaam, wachtwoord en sleutel lokaal in.
   Hoofdletters en kleine letters moeten exact behouden blijven. Geheime
   velden tonen sterretjes; opgeslagen waarden worden niet teruggelezen.
   Herstart de Atom en controleer dat INFO nog 'ingesteld' aangeeft.
8. **Wifi/NTP/API.** Activeer een 2,4GHz-telefoonhotspot met internet. Stel
   het gewenste API-model in. Stuur 'Antwoord alleen met: verbinding gelukt'.
   Verwacht voortgang wifi -> tijd -> model -> antwoord. Alleen een echt
   teruggekomen antwoord valideert deze stap. Controleer API-verbruik in je
   eigen account; deze test kan geld kosten. Exacte tekst kan door het model
   afwijken, dus controleer ook dat een antwoord daadwerkelijk is ontvangen.
9. **Scrollen/context.** Vraag om ongeveer 1000 tekens met accenten en een
   simpele formule. Scroll regels en pagina's, test F1 vervolgvraag en
   verwijzing naar de vorige vraag. F2 Nieuw moet bevestigen en de context
   wissen. Na voldoende lange vragen moet oude context zichtbaar verdwijnen.
10. **Fouten.** Hotspot uit -> wifi-fout. Verkeerd wachtwoord -> melding om
    hotspot te controleren. Ongeldige sleutel -> autorisatiefout. Onbekend
    model -> modelfout. Controleer dat je kunt terugkeren en corrigeren.
    Quota-, certificaat-, internet- en time-outfouten zijn in de firmware
    afgehandeld maar moeten worden getest wanneer je die omstandigheden
    verantwoord kunt nabootsen; beschrijf ze anders als ongetest.
11. **Annuleren.** EXIT tijdens wachten: UI blijft bruikbaar; een reeds
    verzonden API-aanvraag kan worden gefactureerd. De Atom kan zijn lopende
    netwerkactie eerst afronden. Direct opnieuw proberen kan 'Atom is nog
    bezig' geven. Test pas opnieuw nadat de actie klaar is.
12. **Afsluiten/heropenen.** MENU tijdens invoer, antwoord en wachten moet de
    poort sluiten en naar het normale Casio-menu gaan. Open opnieuw en voer
    F3 uit. Test daarna een normale Casio-functie. Bij het wachten wordt een
    best-effort annuleerbericht verstuurd vóór sluiten; geen garantie dat de
    API-aanvraag nog te stoppen is.
13. **Reset/herhaling.** Reset de Atom tijdens een aanvraag. Verwacht een
    fout of verloren gesprek, zonder automatische nieuwe POST voor hetzelfde
    ID. Begin pas bewust opnieuw na controle van de fout. Een expliciete
    nieuwe F1-aanvraag krijgt een nieuw ID en kan opnieuw kosten maken.

Slaapstand en ontwaken zijn op verzoek weggelaten. Doe geen ruststroommeting
voor het eindproduct op basis van dit USB-gevoede ontwikkelbord. De latere
XIAO ESP32-C6 vereist een aparte firmwareport en hardwarevalidatie.

## Aanvullingen voor V2

- Voer eerst UPGRADE-V2.md uit. Controleer bij Instellingen → Versie dat zowel
  app als Atom dezelfde actuele versie aangeven (V3: 3.0). Protocol v1 bij F3 is nog steeds correct.
- Controleer leesbaarheid van de donkere interface op je echte scherm en
  de normale/geselecteerde menu-iconen. De voorbeeldafbeeldingen zijn alleen
  hostrendering. Alleen het eigen app-icoon verandert, niet het systeemmenu.
- Atom los/onbereikbaar: Instellingen moet Onbekend tonen zonder voorafgaande
  foutpopup. Sluit de Atom opnieuw aan en gebruik F3 in Instellingen.
- Controleer dat hotspotnaam en eigen model worden teruggelezen. Een
  gewijzigde modelnaam moet bij opnieuw bewerken behouden zijn; wachtwoord
  en sleutel mogen nooit worden teruggelezen.
- Onderbreek bij een proefinvoer de verbinding tijdens opslaan. Na de
  foutmelding moet de invoer in het veld blijven voor een nieuwe poging.
  Gebruik hiervoor een onschuldig proefveld, geen echte sleutel op een foto.
- Test lange antwoorden met woorden en accenten, paginascrollen tot het eind,
  een lange foutmelding, F1 vervolgvraag en F2 Nieuw gesprek.
- Stel meerdere korte vragen na elkaar om geheugenstabiliteit in de praktijk
  te controleren. Deze duurtest is hier nog niet op de Atom uitgevoerd.
- Controleer na firmware-update dat bestaande instellingen behouden zijn.
  De NVS-indeling is gelijk gebleven, maar behoud is hier niet fysiek getest.

## V3: scherm, menunaam en lokale geschiedenis

1. Voer UPGRADE-V3.md uit en controleer app 3.0 / Atom 3.0. Controleer dat het
   menu één ChatGPT-naam toont: normaal grijs, geselecteerd zwart. Het logo is
   zwart op wit; de app zelf tekent geen tweede naam in de bitmap.
2. Open F6 Instellingen → F4 Scherm. De rand moet rondom zichtbaar zijn met
   TL/TR/BL/BR, zonder witte OS-balk die een deel bedekt. EXIT keert terug.
3. Verstuur één korte vraag. Deze moet automatisch lokaal worden opgeslagen.
   Sluit de app af met MENU, open opnieuw en kies OPTN → opgeslagen gesprek.
4. Ontkoppel de Atom-signaalverbinding alleen met beide apparaten uit, volgens
   de eerdere hardwareafspraken. Controleer daarna teruglezen op Casio-batterijen
   zonder Atom: historie lezen vereist geen netwerk of UART. Herstel de
   gecontroleerde opstelling voordat je opnieuw een vraag verstuurt.
5. Stel een vervolgvraag in een heropend gesprek. Controleer of recente context
   terugkomt. Het herstellen van context op zichzelf mag geen tweede API-vraag
   starten. De volgende bewust verzonden vraag kan wel worden gefactureerd.
6. Maak twee chats. Test wisselen, F5/F6 eerdere/volgende berichten, SHIFT voor
   de volledige vraag, scrollen, en dat een ingetypte maar nog niet verstuurde
   vraag niet zonder bevestiging verdwijnt bij het wisselen.
7. Sluit de app af en maak een backup van de CGPT3-map via Casio-USB-opslag.
   Controleer opnieuw openen nadat alleen het .g3a-bestand vervangen is.
8. Wis bewust een proefgesprek met F2 in de zijbalk. Controleer dat andere
   gesprekken blijven bestaan. Op volle opslag moet er een melding komen;
   opnieuw proberen met Bewaar mag alleen lokaal opslaan, zonder nieuwe POST.
9. De hosttests simuleren onderbroken schrijfacties en corrupte banken. Doe
   geen ongecontroleerde stroomonderbreking als vervanging voor een backup.
   Werkelijke flashfouten en OS-opslaggedrag zijn hier nog niet getest.

## V4: regressiecontrole op het apparaat

Voer UPGRADE-V4.md uit. Controleer Casio 4.0 / Atom 3.0; voor deze update hoeft
de Atom niet opnieuw geflasht te worden. Herhaal vijfmaal EXIT → Ja → opnieuw
openen en vijfmaal MENU → opnieuw openen, inclusief kort vasthouden van EXE.
Test EXIT → Nee. Er mag geen afsluitdialoog vanzelf terugkomen.

F6 → F4 opent nu Scherminstellingen. F2 toont de hoekentest. Controleer eerst
100%/0/0, daarna enkele andere waarden. F1 bewaart; EXIT annuleert; F3 herstelt
standaard. Controleer behoud na heropenen en controleer dat gewone Casio-apps
hun normale weergave houden. De nieuwe displaybestanden zijn alleen D0A.dat
en D0B.dat; C-bestanden met chats mogen niet verdwijnen of wijzigen door
alleen scherminstellingen aan te passen. V4 is hier niet fysiek getest.

## V5: menuoverdracht en aparte schermrand

Voer UPGRADE-V5.md uit. EXIT → Ja moet een interactief Casio-menu tonen;
controleer dit door RUN-MAT te openen. Herhaal vijfmaal met opnieuw openen
en een F3 Atom-diagnose. Controleer ook EXIT → Nee en MENU. Alleen UART
sluiten zonder menuoverdracht is een fout (zoals gemeld voor V4).

Controleer bij 100%/0/0 de fysieke zij- en onderrand: die wordt apart via
DrawFrame ingekleurd. Controleer herstel naar wit in standaardapps. Loop
naar en van ChatGPT in MENU; de geselecteerde bitmap moet het hele blok
markeren, met één appnaam. De OS-tekst/focuskleur blijft systeemgedrag.
Deze hardwarecontroles zijn hier nog niet uitgevoerd.

## V6: directe menu-aanroep, UART-volgorde en blauwe selectie

Volg UPGRADE-V6.md. Test eerst F3 met een aangesloten Atom, dan EXIT → Ja.
Controleer MENU door RUN-MAT te openen; alleen een UART-statusverandering
geldt niet als succes. Controleer opnieuw openen en F3 vijfmaal. Test ook
Nee en fysiek MENU. V6 sluit UART niet vóór de directe DisplayMainMenu-aanroep;
de OS-appwisselcallback en de fallback bij terugkeer ruimen op. De werkelijke
OS-callbacktiming is hier niet uitgevoerd. Controleer behoud van de donkere
schermrand en vergelijk het rechte lichtblauwe/witte geselecteerde pictogram
met de standaardapps, inclusief naam/focus die het OS zelf tekent.

## V7: menu laten staan, niet automatisch ChatGPT heropenen

Controleer eerst versie 7.0. Test F3, vervolgens EXIT → Ja en laat F1 los.
MENU moet blijven staan; open RUN-MAT. Herhaal met ChatGPT vijfmaal en test
Nee en fysieke MENU. Deze keer keert main terug naar het OS en bevat geen
interne sessielus. Test dat opnieuw openen geen oude afsluitvraag toont.

Controleer dat het geselecteerde pictogram volledig lichtblauw is, de naam
volledig onder het kleinere logo staat en dezelfde native lettergrootte heeft
als andere apps. Controleer behoud van donkere schermranden en chats.
De hosttests reproduceren de automatische herstart van V6 en slagen met V7;
de echte OS-menu- en naamweergave zijn hier niet uitgevoerd.

## V8: sluit én heropen, zonder terugval naar MENU

Controleer eerst 8.0, test F3, kies EXIT → Ja, open dezelfde app opnieuw.
Herhaal vijfmaal en met RUN-MAT ertussen. Test Nee, fysiek MENU, lang genoeg
vasthouden/loslaten van F1 en EXE en F3 na heropenen. Controleer behoud van
chats en donkere schermranden. De native GetKey-dispatcher wordt hier alleen
in hoststubs gemodelleerd; juist dit sluiten/heropenen vereist devicecontrole.
Vergelijk het geselecteerde pictogram: wit rechthoekig paneel achter het logo,
blauwe selectie eromheen/onder de naam, één leesbare native appnaam.

## V9: native SaveAndOpenMainMenu, OS 03.81.0202

Follow UPGRADE-V9.md. Confirm 9.0 and read Version's menu route (native found
or OS timer). Test EXIT → Yes → real interactive MENU → same app immediately,
five times, then with RUN-MAT between. Also test physical MENU and No. Test
F3 after re-entry and saved chats/screen settings. User confirmed V8's white
rim is not a menu, and physical MENU there also failed; do not count that as
success. The native helper is based on gint; actual OS execution is unverified.
Check lowered rounded white panel, J at top-right and one native app name.
The J badge is visual; the calculator still determines the actual menu index.

## V10: ALPHA, wiskundetoetsen en automatisch opnieuw verbinden

De gebruiker bevestigt V9 MENU op 03.81.0202. Volg UPGRADE-V10.md: controleer
10.0, laat F3 achterwege en controleer Atom OK na openen/heropenen. Test typen
tijdens offline zijn en herstel na opnieuw voeden. Controleer alle rode letters,
vooral B/log, ALPHA aan/uit, F5-case en gewone/SHIFT sin/cos/tan, log/ln, x²/^
en pi. Controleer API-sleutelcase zonder echte sleutel in logs te zetten.

Herhaal EXIT → Ja → opnieuw openen vijfmaal: geen oud afsluitdialoog, wel een
automatisch verbonden Atom. Test RUN-MAT ertussen en behoud chats/schermranden.
De snapshot- en asynchrone UART-werking zijn host-getest; V10 zelf is hier nog
niet fysiek uitgevoerd. Achtergrondverbinding gebruikt alleen gratis HELLO.

## V11 — telefoon en wisselen naar andere apps

Zie UPGRADE-V11.md voor installatie van **beide** programma's en de apparaattest.
Controleer specifiek EXIT → Ja → RUN-MAT/SYSTEM/STAT → ChatGPT, vijf cycli,
zonder ADDRESS(W) System ERROR. Herhaal met MENU. Hosttests bewaken dat geen
quit-callback, UART of bestandsresource achterblijft bij OS-dispatch; dit is geen
fysieke OS-emulatie. Test op Android/iOS de lokale pagina, verkeerd wachtwoord,
WPA2-wachtwoord, opslaan, Terug, tienminutentimeout en opnieuw opstarten.
Wifi-status is geen API-sleutelvalidatie. Een API-test vereist eigen tegoed.


## V12 — heropenen en UART herstellen

Zie UPGRADE-V12.md. Open/sluit/heropen ChatGPT tien keer, met normale
USB-voeding op de Atom. Test ook na RUN-MAT en SYSTEM. Geen F3 nodig.
Open Instellingen terwijl de Atom uit is en sluit vervolgens USB-voeding aan:
status en instellingen moeten automatisch herstellen. Controleer na Atom-
herstart dat geen betaalde vraag opnieuw wordt verzonden. Hosttests modelleren
nu Serial_IsOpen werkelijk; V11-mocks retourneerden altijd 0 en misten de
OS-open/lokaal-gesloten regressie. Fysieke responstijden zijn niet gemeten.


## V13 — elke toets en twintig menuheropeningen

Zie UPGRADE-V13.md. Test eerst alle toetsen (ALPHA/SHIFT, letters, getallen,
pijlen, DEL, F1-F6, EXIT en MENU); controleer Atom-status en of Atom zelf
herstart. Herhaal twintigmaal EXIT/Ja/openen, inclusief na de derde keer, en
herhaal via MENU. Test ook RUN-MAT, SYSTEM en LINK: UART blijft nu OS-open
waar mogelijk. Herstart de Casio als LINK de poort niet vrij kan krijgen en
rapporteer dat gedrag; geen eigen quitcallback als omweg installeren.
Hosttests controleren 50 keyposities en 40 gesimuleerde sessies, geen echte OS-uitvoering.


## V14 — bediening en modelkeuze vóór verkoop

Controleer F3 =, SHIFT+punt en F4 eerste teken; ook met ALPHA-lock en in een
afgeschermd veld. EXIT mag vanaf geen enkel scherm de app afsluiten. MENU
moet overal bevestigen; Blijven/EXIT behoudt concept en selectie, Sluiten gaat
naar het echte menu. Herhaal twintig heropeningen en veel typen met de stabiele
GND. Controleer één centrale Diagnose en de telefooninstallatie. Kies alle
drie modelpresets; controleer met eigen API-tegoed echte vragen en modeltoegang.
De oude aangepaste modelnaam moet blijven staan na annuleren van de lijst.
Test lokaal opslaan, volle opslag en gebruikersgegevens wissen vóór overdracht.
Zie UPGRADE-V14.md en docs/V14-STARTGIDS.md. Hosttests zijn geen hardwarecertificatie.


## V15 — kopersmenu, antwoordtekens en verbinding

Volg UPGRADE-V15.md: alle vijf pijlen via EXE en rechts, terug met EXIT, MENU
bevestigen/annuleren, veel toetsen en twintig heropeningen. Header uitsluitend
Verbonden/Verbroken; test de overgang na module uit/aan. Diagnose mag geen
getallen of protocolwaarden tonen. Controleer echte antwoorden met = en
wiskundige tekens, de laadanimatie en annuleren zonder dubbele betaalde vraag.
De tests voor TX-drukte, gedeelde deel-frame-parser en herstel met cooldown
zitten in hosttests; die vervangen geen test met echte voeding/GND/draden.

## V16 — V13 als verbindingsbasis

Volg UPGRADE-V16.md. Test veel toetsen, twintig MENU/heropeningen en andere Casio-apps.
Laat Instellingen minstens 30 seconden stilstaan; gewone heartbeatberichten mogen
geen flikkeren of selectiesprongen geven. Test iedere rij met EXE/rechts, EXIT/links
terug, MENU → Blijven in een bevestiging, Atom uit/aan en de drie-miss herstelroute.
Controleer met een echte vraag het pulserende logo zonder statuslijst/knoppenbalk,
annuleren met EXIT/F1 en leesbare =-tekens zonder gewijzigde opslag.
De versie hoort alleen bij Instellingen → Hulp en versie → Over deze app.

## V17 — lichte UI en modellen uit je account

Volg UPGRADE-V17.md en werk Atom én Casio-app bij. Controleer behoud van wifi,
sleutel en chats, alle vier instellingen en Hulp-pagina's via EXE/rechts, laden,
annuleren en twintig echte menuheropeningen. Vergelijk de accountlijst met de
GET /v1/models-respons van je eigen account, rekening houdend met het beschreven
filter. Test je gekozen modellen afzonderlijk met echte vragen. Controleer een
ongeldige sleutel, geen wifi, vernieuwen/annuleren, modelbehoud en een aangepaste
modelnaam indien je account die heeft. Accountlisting is geen betaalde completion
en bewijst geen ondersteuning door Chat Completions.

## V18 — kaartstijl en tekstuitlijning

Vanaf V17 alleen de .g3a vervangen. Controleer gecentreerde teksten, selectieranden
in alle lijsten, de vier accentkleuren, kleine pictogrammen en functietoetskadertjes.
Test laadtekst bij vragen én modellen ophalen, annuleren, aangepaste schermschaal
en menuheropenen. De softwaretests vervangen geen visuele controle op een echte Casio.

## V19 — abstract ontwerp en groene/rode status

Vanaf V17/V18 alleen de .g3a vervangen. Controleer leesbaarheid van groene en rode
statustekst, Atom uit/aan op invoer/Instellingen, het abstracte begin-/laadmotief,
selectieranden, functietoetsen en aangepaste schermschaal. Herhaal de gewone vraag-,
toets- en MENU/heropenchecks. De status blijft modulecontact, geen internetgarantie.

## V20 — donker thema

Vanaf V17 t/m V19 alleen de .g3a vervangen. Controleer leesbaarheid van gewone
en gedempte tekst, groen/rood, selectieranden en primaire acties in de donkere
uitvoering. Loop invoer, antwoorden, Hulp, instellingen, laden en menuheropenen
door en controleer ook je aangepaste schaal/positie.

## V21 — gladder logo en kleurverloop

Vanaf V17 t/m V20 alleen de .g3a vervangen. Controleer het grotere beginlogo,
het mint-lila verloop, de dubbele rand, kleine bovenbalk-/antwoordlogo's en de
laadanimatie. Test ook je opgeslagen schaal/positie, annuleren en menuheropenen.
De donkere modus en statusbetekenis blijven gelijk.

## V20.1 — volledige modellenlijst en planeetring

Werk Atom én .g3a bij met behoud van configuratie/CGPT3. Controleer de symmetrische
ring en het verwijderde tekstregeltje. Vergelijk Account met de volledige GET /v1/models
voor je sleutel, inclusief niet-chatfamilies; controleer F2 Basis/Account, paginering,
lange namen en Info. Beeld/audio mogen geen chatvraag of nieuwe modelinstelling
veroorzaken. Test een echt toegankelijk tekstmodel, veel toetsen, Atom uit/aan en
twintig MENU/heropeningen plus andere Casio-apps.

## V21.1 — chatbalk en installatiepakket

Volg docs/V21.1-CONTROLE-VOOR-VERKOOP.md per apparaat. Controleer placeholder,
1/2/8-regelige berichten, lange invoer inkorten, cursor, alle hoogtes en groene/rode
bolletjes. Open PDF en HTML offline op Windows en loop de installatie met het
klantpakket door. Bewaar CGPT3 en bestaande Atom-configuratie. Softwaretests
vervangen deze echte controles niet.


## V22 — 29 oktober 2026

Actuele onderhouden chatmodellen met gewone namen, gefilterd op accountmetadata,
F4 Info met keuzehulp, F2 Vernieuw en veilige lege lijsten. Atom V20.1 blijft
ongewijzigd. Zie V22-MODELLEN.md en V22-CONTROLE-VOOR-VERKOOP.md. Geen live
API-toegang of fysieke Casio/Atom in deze bouwomgeving getest.


## V22.1 — 29 oktober 2026

Directe vaste modelkeuzes, expliciete optionele F2 Account-controle, geen
misleidende fallback na laadfout. Oude/gedeeltelijke accountgegevens worden
niet als succes getoond. Paarsblauwe planeet en compactere laad-/modelschermen.
Atom, native menu, UART, toetsen en opslag ongewijzigd. Controleer de fout- en
succesroute op echt apparaat volgens V22.1-CONTROLE-VOOR-VERKOOP.md.


## V22.2 — 29 oktober 2026

Atom-firmware en Casio-update nodig. Accountmetadata via gedecodeerde HTTP-body
in plaats van raw getStream; gefilterde JSON en begrensd ontvangen. Accountspecifieke
foutteksten. Dubbele Vraag-knop verwijderd, Bewaar bij opslagfout behouden. Geen
wijziging betaalde completion-functie, tokenlimiet, ledger, UART of native menu.
De broncode van ESP32 HTTPClient 3.3.0 bevestigt dat writeToStream chunked encoding
verwerkt. De echte foutrespons van de gebruiker en een live API-account waren
niet beschikbaar; succes op echt apparaat moet na beide updates worden gecontroleerd.


## V22.3 — geheugenarme accountcontrole

Alleen elf metadata-GETs, geen responsebody/JSON/grote accountlijst in RAM.
256-byte resultaat, Casio-buffer 4 KiB, nulresultaat expliciet gecontroleerd,
fouten wissen de hele lijst. Atom V22.3 en Casio-app vereist; betaald verzoek,
UART, opslag en native menu ongewijzigd. Test F2 op het echte apparaat na beide updates.


## V22.4 — status en antwoordtekens

Neutrale statuswoorden/achtergrond met gekleurd bolletje. Extra Unicode-gelijktekens
en native relatieglyphs; betekenis van ongeveer/ongelijk behouden, geen vervanging
van letterlijke vraagtekens. UI-tests controleren ontvangen gelijktekens tot aan
de schermpixels. Atom V22.3 en verbindings-/menu-/opslagcode ongewijzigd.


## V22.5 — doorlopende chat en directe vervolginvoer

Eén tijdlijn met vragen/antwoorden, pijlscrollen en een blijvende composer. Geen
aparte antwoordviewer of vorige/volgende chatknoppen. Invoer met 1–3 zichtbare regels,
ALPHA/hoofdletterstand behouden na versturen. Alleen 152 bytes tijdlijnmetadata,
geen samengeplakte chattekst; live antwoordbuffers blijven intact bij scrollen.
Atom, opslagformaat en native menuhelpers ongewijzigd. Test gesprekcontinuatie,
concepten, oude chats, grens van acht paren, opslagfout en heropenen op echte hardware.


## V22.6 — modelpijlen en chatopmaak

Modelkeuze: geen Vorig/Volgend, geen F3/F5-paginering; schuivende selectie en
scrollbalk. Chat: rolbadges met pictogram, extra witregel na vraag, invoerstand
rechtsonder. Historie begint op y40 zodat de extra ruimte niet ten koste gaat
van een zichtbare regel. Atom, opslag, menuhelpers en math-rendering ongewijzigd.


## V22.7 — compacte invoerstand

Geen ALPHA-balk linksboven op begin- en invoerschermen; één klein neutraal label
rechtsonder naast de invoerbalk. Labelbreedte volgt 123/abc/Abc/ABC en eventuele
SHIFT-aanduiding. Toetsen, ALPHA-lock, modelkeuze, gesprekken, Atom en menuhelpers
ongewijzigd. Bestaande editor-/masker-/menu-/chatregressies controleren de software.


## V22.8 — inline invoerbediening en zinshoofdletters

Modus vóór vaste verstuurpijl in berichtenbalk, tekstbreedte van 33 kolommen met eigen
cursor/wrapping. Losse OPTN/MENU-hint en F5-hoofdletteractie in de editor verwijderd. Lock in
Tekens via F2; automatische zinshoofdletters alleen voor gewone invoer, niet
geheime velden. Model-/API-/opslag-/menuhelpers onveranderd. Test lange invoer,
zinbegin, decimalen, cursor midden woord, lock en exacte geheime letterkast.


## V22.9 — automatische hoofdletters zonder handmatige lock

De vaste hoofdletterstand en F2-lock in Tekens zijn verwijderd. ALPHA blijft
cijfers/letters wisselen; zinshoofdletters blijven automatisch en losse hoofdletters
zijn via Tekens te kiezen. Geheimen behouden exacte letterkast. Atom V22.3,
modelkeuze, transport, opslag en native-menuhelpers zijn ongewijzigd.
Casio-build en zestien ASan/UBSan-testgroepen geslaagd binnen de sandbox met
ASAN_OPTIONS=detect_leaks=0; de afzonderlijke lekcontrole is niet uitgevoerd.
Controleer bediening en heropenen op echte hardware vóór levering.


## V30 — teruggaan, basissymbolen en accountcontrole

EXIT vanuit een chat gaat lokaal terug naar het beginscherm; opgeslagen chat
blijft bewaard en concept/opslaanfouten vragen bevestiging. Geen Atom-RPC bij
teruggaan. Bij annulering wordt de laadweergave vóór W_CANCEL verwijderd.
Veertig basissymbolen zonder alfabet/cijfers; spatie zichtbaar als SP en de
voorheen lege underscoreglyph hersteld. Open/Terug met rechts/links; Hulp alleen
als rij in Instellingen, geen F5-route.

Een gedeelde lijst van 21 GPT-4/5/6-chatnamen met aangekondigde einddatums in Info.
Atom V30 haalt accountmetadata in één GET op en verwerkt gedecodeerde body via
een incrementele parser met vast geheugen; geen grote JSON- of antwoordbuffer.
Maximaal 512 KiB gestreamde body, parser 488 bytes, uitvoer maximaal 2048 bytes,
5s netwerk-time-outs en totale controledeadline. Geen gedeeltelijke lijst bij
fout. Casio toont Account gecontroleerd of Account niet gecontroleerd.

Casio- en Atom-build geslaagd; 17 ASan/UBSan-testgroepen (lekcontrole in sandbox
uitgeschakeld). Onder meer 3500 onbekende modellen, deelchunks, foute JSON,
lege accountdoorsnede, offline EXIT, conceptbehoud en vertraagde annulering.
Betaalde POST, ledger, UART en native menuhelpers blijven ongewijzigd.
Fysieke Casio/Atom en live API-account niet in deze omgeving getest. Werk beide
onderdelen bij en volg docs/V30-CONTROLE-VOOR-VERKOOP.md vóór levering.


## V31 — bevestigde annulering en begrensde chatverzoeken

Tijdslimiet: 120 s voor chatvragen en 60 s voor accountcontrole, bewaakt tijdens DNS/connect/header/read/write en
bij publiceren; een lopende SDK-call kan beperkt uitlopen. Afzonderlijke native
socket- en Stream-time-outs, begrensde persistente DNS-callbackopslag en sluiten
van sockets alleen in de eigenaarstaak. Worker blijft bezet tot opruimen klaar is.
W_MODELS-actie 3 stopt accountcontrole, status 4 bevestigt afronden. Casio wacht
op terminale status; onbekende/verloren bevestiging blijft zichtbaar en voorkomt
nieuwe betaalde aanvragen. Expliciet opnieuw proberen herhaalt de stopopdracht.
MENU tijdens beide wachtfases, ontbrekende W_ASK-bevestiging en statusfout ruimen
werk op zonder automatisch nieuwe betaalde ID/POST. Lokale context wordt na
annulering bij de volgende vraag hersteld. Drie aanbevolen keuzes, F3 Alles/Advies;
alle 21 bestaande IDs blijven bereikbaar en de gekozen instelling blijft intact.

Beide builds en 18 ASan/UBSan-groepen geslaagd; lekcontrole uit in sandbox.
Geen fysieke apparatuur of live API-sleutel beschikbaar. Gebruik V31 als
praktijkkandidaat; volg V31-CONTROLE-VOOR-VERKOOP.md voor apparaatvrijgave.


## V32 — annuleren vóór de verstuuropdracht

EXIT/F1 worden bij een nieuwe vraag vastgelegd tijdens alle voorbereidende
UART-stappen, vóór elke verzendpoging en na ontvangst van een bevestiging.
Het annuleringsslot voorkomt een latere W_ASK. Opruimen gebruikt een aparte
route die niet door hetzelfde slot wordt geblokkeerd. Een onvolledig ontvangen
antwoord wordt niet opgeslagen; de lokale chat wordt hersteld en context wordt
bij een bewuste nieuwe vraag opnieuw opgebouwd. Zodra W_ASK al naar de driver
is doorgegeven, wordt geen belofte van kosteloze annulering gedaan.

Casio-build en 18 ASan/UBSan-groepen geslaagd. De nieuwe matrix bevat 58 vroege
annuleringen, twee na verzending, vier tijdens antwoordoverdracht en herstel
bij bewust opnieuw proberen. Dezelfde regressietest faalt op de oorspronkelijke
V31-code. Atom blijft bytegelijk V31. Geen echte hardware of live API getest.


## V32.1 — antwoordbegin en opgeruimde bediening

Een nieuw volledig ontvangen antwoord krijgt eenmalig focus op het ChatGPT-label
en de eerste zin, ook bij vervolgvragen en onopgeslagen antwoorden. Daarna
blijft handmatig scrollen behouden. Instellingen toont geen losse Open/Terug-
hints meer; de fysieke toetsen blijven werken. In de zijbalk zijn F1 Wis en
F2 Nieuw. Model kiezen toont drie aanbevelingen met daaronder Overige modellen;
rechts/EXE opent de achttien overige namen, links/EXIT gaat terug. De gekozen
model-ID verandert niet automatisch en accountfiltering blijft behouden.
Na accountcontrole verschijnt circa 900 ms een groen vinkje of rood kruis in het
laadscherm; zelf annuleren geldt niet als mislukte accountcontrole.

Casio-build en 18 ASan/UBSan-groepen geslaagd, inclusief de vroeg-annulering-
regressies van V32. Geen nieuwe live API-aanvraag of fysieke test uitgevoerd.
Atom-code en alle vier firmwarebestanden blijven bytegelijk V31.


## V32.2 — SHIFT-combinaties en derdemachtswortel

SHIFT + ( voegt nu ∛( in met een eigen lettertypeglyph. Een ingedrukte SHIFT
werkt ook als toetscombinatie; na één functie kan dezelfde SHIFT-druk niet
opnieuw inschakelen. Aantikken, gelijktijdig indrukken, beide loslaatvolgordes,
dubbel SHIFT annuleren en ALPHA-lock zijn gedekt. sin( blijft de gewone sinus;
asin( met SHIFT is de inverse sinus, dezelfde functie als sin⁻¹.

Casio-build en 18 ASan/UBSan-testgroepen geslaagd (LeakSanitizer uitgeschakeld
in deze omgeving). De echte editorfixture toont het oorspronkelijke verlies
van SHIFT+sin met de oude scanner en slaagt met de nieuwe scanner. De overige
annulerings-, navigatie- en UART-regressies blijven slagen. De eigen glyph en
het daadwerkelijke UI-frame zijn gecontroleerd. Atom-code en alle vier
firmwarebestanden blijven bytegelijk V31; geen fysieke of live API-test van
deze patch uitgevoerd. Alleen de Casio-app bijwerken als Atom V31 al aanwezig is.

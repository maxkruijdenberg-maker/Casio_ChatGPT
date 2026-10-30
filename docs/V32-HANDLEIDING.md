# ChatGPT voor Casio fx-CG50

## V32 — installatie en gebruik

Deze versie is bedoeld voor gecontroleerd praktijkgebruik. Een geslaagde softwaretest
is geen verkoopvrijgave voor een gebouwd apparaat. De maker moet de echte werking
controleren; daarvoor is een aparte controlelijst meegeleverd.

Een donkere chat-app met een paarsblauwe planeet en een meegroeiend invoerveld. Groen bolletje betekent
Verbonden; rood bolletje betekent Verbroken. De woorden en de achtergrond blijven neutraal; alleen het bolletje heeft een kleur.
De status gaat over contact met de Atom, niet over internet of je API-tegoed.

## Wat je nodig hebt

- Een Casio fx-CG50 met de aangesloten originele M5Stack Atom Lite en voeding.
- Wifi op 2,4 GHz, via een router of een andere telefoon als hotspot.
- Je eigen OpenAI-API-account, API-sleutel en betaald API-tegoed.
- Voor installeren: een Windows-computer en USB-kabels voor Casio en Atom.

Een ChatGPT-abonnement geeft geen API-tegoed. Beheer je sleutel en facturatie
zelf op https://platform.openai.com. Deel je sleutel niet met een verkoper.

## Bestaande installatie bijwerken

Heb je Atom V31 al geïnstalleerd? Vervang alleen de Casio-app met
het meegeleverde dist/ChatGPT.g3a, laat CGPT3 staan en herstart de Casio eenmaal.
Daarin staan je chats en scherminstellingen. De Atom-code en firmware in dit
pakket zijn ongewijzigd V31; opnieuw flashen is dan niet nodig.

Heb je een oudere Atom of weet je het niet? Upload eerst de meegeleverde
Atom-firmware V31 en installeer daarna de Casio-app volgens de stappen hieronder.
Laat volledig wissen uitgeschakeld om wifi, API-sleutel en model normaal te
behouden. F2 Account en bevestigd stoppen gebruiken de functies van Atom V31.

## Casio-app installeren

1. Pak de volledige ZIP uit naar een gewone map in Windows.
2. Verbind de Casio via zijn USB met Windows en kies USB Flash / USB-opslag.
3. Kopieer dist/ChatGPT.g3a naar de hoofdmap van het Casio-station. Vervang het
   bestaande bestand; bewaar geen tweede kopie zoals ChatGPT (1).g3a.
4. Laat de map CGPT3 staan als je chats wilt behouden.
5. Werp het station veilig uit en ontkoppel de Casio-USB.
6. Herstart de Casio eenmaal. Open MENU en kies ChatGPT.
7. Instellingen → Meer → Over deze app toont Versie 32.

De Atom houdt tijdens gebruik zijn eigen voeding. De Casio gebruikt zijn eigen
batterijen. De handleiding gaat uit van een correct geassembleerd apparaat.

## Atom installeren of bijwerken met Arduino IDE

1. Haal de USB-voeding van de Atom los. Ontkoppel tijdelijk de Casio TX/RX-draden
   van de Atom. Noteer de bestaande aansluitingen zodat je ze juist terugplaatst.
2. Sluit de Atom via USB op Windows aan. Installeer Arduino IDE 2.x.
3. Voeg bij Voorkeuren de extra Boardbeheer-URL toe:
   https://espressif.github.io/arduino-esp32/package_esp32_index.json
4. Installeer esp32 by Espressif Systems 3.3.0 in Boardbeheer.
5. Installeer ArduinoJson by Benoit Blanchon 7.4.2 in Bibliotheekbeheer.
6. Open atom/ChatGPTAtom/ChatGPTAtom.ino. Laat alle .h-bestanden ernaast staan.
   Alleen het .ino-bestand kopiëren is niet voldoende.
7. Kies board M5Atom voor de originele ESP32, de juiste COM-poort en snelheid 115200.
   Gebruik geen M5AtomS3. Laat Erase All Flash / volledig wissen uitgeschakeld.
8. Klik Upload. Na Hard resetting via RTS pin... is uploaden klaar; je hoeft
   niet te wachten totdat die tekst verdwijnt.
9. Sluit Serial Monitor, haal de USB los en herstel de eerder gecontroleerde
   TX/RX-aansluitingen. Geef de Atom opnieuw USB-voeding.
10. Installeer daarna de Casio-app zoals hierboven.

Wie liever de voorgebouwde firmware gebruikt, kan Python met esptool 5.0.0
installeren en vanuit de uitgepakte map uitvoeren: py tools/flash_windows.py COM5.
Vervang COM5 door de juiste poort in Apparaatbeheer. Ook hiervoor blijven TX/RX
los tijdens flashen. Gebruik geen erase-flash-opdracht.

## Wifi en API-sleutel instellen via je telefoon

1. Open ChatGPT op de Casio en druk F6 Instel.
2. Kies Wifi en account en druk EXE of pijl rechts.
3. Verbind je telefoon met het wifi-netwerk en wachtwoord op de Casio.
4. Blijf verbonden als je telefoon zegt dat dit netwerk geen internet heeft.
5. Open http://192.168.4.1 in de browser van je telefoon.
6. Vul je eigen wifi-naam en wachtwoord in en plak je volledige API-sleutel.
7. Kies Opslaan en verbinden en kijk op de Casio of verbinding lukt.
8. Ga terug naar Instellingen en kies Model.

De installatietelefoon kan niet tegelijk zelf de benodigde wifi-hotspot zijn.
Gebruik een router of een hotspot op een andere telefoon. De opgeslagen sleutel
wordt niet teruggelezen op het Casio-scherm.

## Een bericht sturen

Stuur een bericht is tijdelijke tekst in de lege chatbalk. Zodra je typt,
verdwijnt die tekst. De balk groeit omhoog bij langere berichten en krimpt
weer bij wissen. Binnen een bestaande chat toont hij maximaal drie regels tegelijk;
langere invoer blijft mogelijk en schuift mee met de cursor. Zo blijft er ruimte
voor eerdere berichten boven de balk. De tijdelijke tekst wordt nooit als jouw vraag verstuurd.

ALPHA zet letters vast aan of uit. De eerste gewone letter wordt automatisch
hoofdletter. Ook na een punt, uitroepteken of vraagteken begint een nieuwe zin
automatisch met een hoofdletter. SHIFT gebruikt een keer een gele wiskundefunctie.
F4 Tekens bevat veertig basissymbolen: reken- en leestekens, zonder alfabet of
cijfers. Bijvoorbeeld =, +, −, ×, ÷ en haakjes. De geselecteerde tegel is duidelijk
omlijnd. SP betekent spatie; de naam Spatie staat erbij wanneer je die selecteert.
Kies een teken met de pijlen en EXE; EXIT gaat terug.
Er is geen handmatige hoofdletterlock. ALPHA wisselt tussen cijfers en letters.
DEL wist de vorige letter.
Links/rechts verplaatst de cursor. In een chat bladert omhoog/omlaag door de
gespreksgeschiedenis. ALPHA-lock blijft na versturen behouden. AC kan de hele invoer na bevestiging wissen.
Het lage streepje (_) is nu zichtbaar in plaats van een leeg vak.

Druk F1 Stuur of EXE om je bericht te versturen. De bewegende paarsblauwe planeet
betekent dat de app de vraag voorbereidt of op de dienst wacht. EXIT of F1 werkt
ook tijdens het voorbereiden: bij het herstellen van de recente chatcontext,
het overzetten van je vraag en het aanvragen van een verzoeknummer.

Annuleer je vóór de verstuuropdracht naar de Atom, dan wordt die vraag niet alsnog
verstuurd. Je concept blijft beschikbaar. Is de verstuuropdracht al verzonden,
dan kan de Atom al een API-vraag gestart hebben; annuleren kan die kosten niet
ongedaan maken.

De laadplaneet verdwijnt bij annuleren. Waar nodig krijgt de Atom een stopverzoek
en wacht de app op de bevestiging dat de taak is afgerond. Een lopende netwerkactie
kan nog kort nodig hebben om terug te keren. Als dit niet bevestigd is, verschijnt Stoppen niet bevestigd en
blijft een nieuwe vraag geblokkeerd. Kies je later zelf opnieuw Stuur, dan controleert
de app eerst de vorige taak en vraagt zo nodig nogmaals om stoppen. Pas als de
Atom beschikbaar is kan je nieuwe vraag worden verstuurd.

Chatverzoeken hebben een netwerkbudget van 120 seconden, accountcontrole van
60 seconden. Een al begonnen lees- of verbindingsactie kan nog kort doorlopen
binnen zijn eigen begrensde time-out. Dit geeft geen garantie dat de dienst altijd
op tijd antwoordt of dat stoppen exact binnen een bepaald aantal seconden klaar is.
Een al verstuurde API-vraag kan nog kosten maken; stuur niet meteen opnieuw bij
een onzekere uitkomst. De app doet geen automatische nieuwe betaalde poging.

Je opgeslagen chats blijven na stoppen staan. Bij een volgende vraag herstelt
de app de recente context van de lokale chat en vervangt daarmee de oude Atom-context.

## Antwoorden en chats

Gewone en kleine, brede, hoger of lager geschreven gelijktekens worden als =
weergegeven. De app ondersteunt ook ongeveer gelijk, identiek en andere veelgebruikte
relatietekens. Bijvoorbeeld ≈ blijft ongeveer gelijk en wordt niet veranderd in =.
Echte vraagtekens blijven vraagtekens. De oorspronkelijke chattekst blijft bewaard.


Vragen en antwoorden staan samen in één doorlopende chat, met Jij en ChatGPT
als afgeronde labels met een klein pictogram. Een extra witregel scheidt de
vraag van het antwoord. De invoerstand (123/abc/Abc) staat ín de berichtenbalk,
vlak vóór het verstuurpijltje. Een kleine S toont de eenmalige SHIFT-stand. Ook
bij lange invoer blijven tekst, cursor en deze bediening van elkaar vrij.
De losse OPTN/MENU-tekst onder de balk is verwijderd. De toetsen werken nog wel.
Met omhoog en omlaag lees je terug en weer naar het laatste bericht.
De invoerbalk blijft onderaan zichtbaar; links/rechts verplaatst de invoercursor.
Er zijn geen aparte antwoordpagina's of Vorig/Volgend-knoppen meer in een chat.

Typ onderaan je vervolgvraag en druk F1 Stuur of EXE. Je blijft in hetzelfde
opgeslagen gesprek; eerder teruglezen begint geen nieuw gesprek en verandert je
concept niet. ALPHA-lock, de keuze tussen cijfers en letters, blijft na versturen
bewaard. Aan het begin van een nieuwe zin wordt een gewone letter automatisch
hoofdletter; daarna typ je kleine letters. Wifi en je API-sleutel vul je via
je telefoon in, met de exact gewenste letterkast.

F3 Chats of OPTN opent de lokale zijbalk. F2 Nieuw begint bewust een nieuw gesprek;
bij mogelijk verlies van een concept vraagt de app om bevestiging. EXIT brengt
je vanuit een chat lokaal terug naar het beginscherm, zonder laadicoon of
Atom-aanvraag. Het opgeslagen gesprek blijft onder F3 Chats staan. Bij een
ongestuurd concept vraagt de app eerst of je het wilt verwerpen. Een antwoord
dat nog niet is opgeslagen geeft een aparte waarschuwing.
Op het beginscherm sluit EXIT de app niet af en blijft je concept staan.
Alleen MENU vraagt om afsluiten.

Acht gesprekken met elk maximaal acht vraag/antwoordparen blijven lokaal bewaard.
Ook eerdere vragen zijn in de tijdlijn leesbaar. Bij acht paren meldt de app dat
het gesprek vol is; oudere berichten worden niet stilzwijgend verwijderd of in
een nieuwe chat gezet. Oude chats zijn offline leesbaar. Nieuwe antwoorden
vereisen internet en API-toegang; recente context wordt zoals voorheen begrensd
naar de Atom doorgestuurd.

Chats worden normaal automatisch opgeslagen. Mislukt dat, dan verschijnt tijdelijk
F3 Bewaar in plaats van Chats. Hiermee probeer je alleen lokaal opnieuw op te slaan;
er wordt geen nieuwe API-vraag verstuurd. OPTN opent dan nog steeds de zijbalk,
met een waarschuwing voordat een onopgeslagen antwoord wordt verlaten.

AI kan fouten maken: controleer belangrijke antwoorden.

## Modellen kiezen

Instellingen → Model begint met drie aanbevolen keuzes:

- GPT-6.1 Sol: een startkeuze voor dagelijkse vragen en uitleg.
- GPT-6 Luna: voor korte vragen en eenvoudige uitleg.
- GPT-6 Astra: voor een uitgebreidere vraag of redenering.

Dit is keuzeadvies van deze app, geen bewijs van snelheid, kwaliteit of prijs.
F3 Alles opent Alle modellen; F3 Advies gaat terug naar Aanbevolen. In Alle modellen
blijven alle 21 ondersteunde namen
uit de GPT-4-, GPT-5- en GPT-6-families bereikbaar. Het wisselen verandert je huidige
model niet. De namen, bron en aangekondigde einddatums staan in docs/V32-MODELLEN.md.

De lijst opent zonder internetaanvraag. Dat is nog geen controle van jouw API-account.
F2 Account vraagt de accountlijst op met één gestreamde GET en begrensd geheugen.
Gebruik hiervoor de Casio-app V32 en de meegeleverde, ongewijzigde Atom-firmware V31. Na een geslaagde
controle staat er Account gecontroleerd; Aanbevolen en Alles tonen vervolgens alleen
de passende namen uit jouw account. Een geslaagde controle zonder passende namen
blijft een lege, gecontroleerde lijst. Zijn alleen andere ondersteunde namen
beschikbaar dan de drie aanbevelingen, dan opent de app automatisch Alles.
Je bestaande model blijft bewaard.

Accountmetadata alleen garandeert geen geslaagde chatvraag. Ook compatibiliteit met
de chatdienst, modelrechten en tegoed moeten kloppen. De controle verstuurt geen
betaalde vraag. Mislukt de controle, dan staat er Account niet gecontroleerd en zie
je herkenbare vaste, ongecontroleerde keuzes. Een gedeeltelijke of onleesbare respons
wordt niet als volledige succesvolle controle getoond.

EXIT of F1 tijdens de controle vraagt ook de Atom om te stoppen. De laadplaneet
verdwijnt direct. Een al begonnen netwerkleesactie kan nog enkele seconden nodig
hebben om terug te keren. Als het stoppen nog niet is bevestigd, toont de app dat
de controle nog wordt afgerond. Zonder bevestiging verschijnt Stoppen niet bevestigd en blijft een nieuwe vraag
geblokkeerd. Als je later zelf opnieuw probeert, controleert de app eerst de vorige
taak. Er wordt niets automatisch betaald herhaald.

Omhoog/omlaag kiest een model; de lijst schuift mee. F4 Info geeft uitleg en een
waarschuwing als een aangekondigde einddatum bekend is. Lees met omhoog/omlaag;
links of EXIT gaat terug. EXE, rechts of F1 Kies slaat de modelkeuze op. Actief
vermeldt het ingestelde model, ook als het buiten de drie aanbevelingen valt. Een bestaande keuze wordt nooit stilzwijgend vervangen.
Controleer actuele tarieven op https://openai.com/api/pricing/.

## Terug en afsluiten

EXIT gaat vanuit een chat terug naar het beginscherm. Dit gebeurt lokaal,
zonder laadicoon; je opgeslagen gesprek blijft via F3 Chats beschikbaar.
Een ongestuurd concept en een niet-opgeslagen antwoord worden niet zonder
waarschuwing verworpen. In instellingen en Tekens gaat EXIT een scherm terug.
Op het beginscherm blijf je in de app en blijft je concept staan.

Alleen MENU vraagt om ChatGPT af te sluiten. Blijven of EXIT in dat
afsluitvenster bewaart je concept. Bij echt afsluiten wordt ongestuurde invoer
gewist; opgeslagen chats blijven staan.

## Instellingen en hulp

- Wifi en account: installatie via telefoon.
- Model: actuele chatkeuzes, uitleg en accountcontrole.
- Hulp: starten, typen/toetsen, antwoorden/kosten en verbinding controleren.
  Deze keuze staat alleen in Instellingen; er is geen aparte F-sneltoets voor Hulp.
- Meer: scherm aanpassen, appversie en wifi/sleutel wissen.

Bij een pijl naar rechts openen EXE en rechts de keuze. EXIT of links gaat
terug in lijsten, de chatzijbalk en hulpschermen. De losse F-knoppen Open en Terug
zijn verwijderd. Andere actieknoppen, zoals Stuur, Account en Info, blijven staan.
Bij Scherm aanpassen wijzigen links/rechts de geselecteerde
waarde; EXE/F1 slaat op en EXIT annuleert de wijziging.

## Als iets niet lukt

Verbroken: controleer of de Atom voeding krijgt en goed is aangesloten. Open
Instellingen → Hulp → Verbinding controleren. De app probeert opnieuw contact.
Een groene status bewijst niet dat wifi, internet en API-tegoed werken.

Wifi verbinden mislukt: controleer netwerknaam, wachtwoord en 2,4GHz-wifi.
Verbind de installatietelefoon niet tegelijk als de hotspot die je wilt gebruiken.

Geen API-toegang of tegoed: controleer je sleutel, projectrechten en facturatie
in je OpenAI-account. Een ChatGPT-abonnement is geen API-tegoed.

Model geeft een fout: kies een ander beschikbaar tekstmodel. Accountmetadata alleen garandeert geen geslaagde chatvraag. Controleer ook
je rechten en tegoed. De app probeert niet automatisch een andere betaalde route.

Stoppen niet bevestigd: de Atom heeft de afronding van de vorige taak nog niet
bevestigd. Controleer of hij voeding heeft en of de verbinding herstelt. De app
start tot die bevestiging geen nieuwe vraag. Bij een expliciete nieuwe poging
controleert de app eerst de oude taak en vraagt zo nodig nogmaals om stoppen.

Versturen niet bevestigd: de vraag kan al verstuurd zijn. Er is om stoppen gevraagd;
de app start niet automatisch een nieuwe betaalde aanvraag. Controleer de uitkomst
voordat je zelf besluit opnieuw te versturen.

Vraag lijkt vast te staan: annuleer het wachten. De meegeleverde Atom V31 bewaakt de totale tijd van
het netwerkwerk en beëindigt een stilgevallen overdracht. Stoppen kan nog kort duren
door een lopende netwerkleesactie. Een al verzonden vraag kan kosten hebben gemaakt;
de app verstuurt die niet automatisch opnieuw. Controleer de uitkomst voordat je
zelf nogmaals verstuurt. Blijft een apparaat toch herhaalbaar vastzitten, laat de
maker de exacte fout en de uitgevoerde handelingen onderzoeken.

Opslaan mislukt: gebruik Bewaar in het antwoordscherm of controleer vrije
Casio-opslag. Verstuur dezelfde vraag niet opnieuw om een opslagprobleem op te lossen.

Een System ERROR of herhaalbaar vastlopen is geen normale werking. Noteer wat
je deed en de fouttekst, en laat het apparaat door de maker controleren.

## Gegevens en overdracht

Chats staan op de Casio in CGPT3. Wifi en API-sleutel staan op de Atom zonder
flashversleuteling. Vragen en recente gesprekscontext gaan bij gebruik naar OpenAI.

Wifi en sleutel wissen onder Meer verwijdert geen chats en geen OpenAI-account.
Het herstelt ook de standaardmodelkeuze. Verwijder je chats afzonderlijk via Chats.
Wis persoonlijke gegevens vóór overdracht en trek een niet meer gebruikte
API-sleutel eventueel in via je OpenAI-account. Zet je sleutel nooit in broncode,
een openbare foto of een bericht aan een verkoper.

## Eerste proef met de handleiding

Laat iemand die de app nog niet kent de installatie en het eerste gesprek uitvoeren
met alleen deze handleiding. De maker kijkt mee zonder toetsen of stappen voor te
zeggen. Noteer waar de gebruiker vastloopt, een melding verkeerd begrijpt of hulp
nodig heeft. Controleer starten, model kiezen, een vraag en vervolgvraag, teruglezen,
EXIT naar het beginscherm en opnieuw openen via Chats. Verbeter de uitleg of de app
voordat dit als zelfstandig bruikbaar product wordt aangeboden.

## Over dit pakket

V32 bevat de bijgewerkte Casio-app V32 en dezelfde Atom-firmware V31. De voorbeeldschermen
zijn softwareweergaven met voorbeeldgegevens. Er is hier geen fysieke duurtest
of live betaalde modeltest uitgevoerd. De meegeleverde controlelijst beschrijft de
nog uit te voeren praktijkproef. Het is een onafhankelijk project.
De licentie- en bronvermeldingen zijn meegeleverd in docs.

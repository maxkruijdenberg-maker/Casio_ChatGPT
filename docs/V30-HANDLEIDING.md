# ChatGPT voor Casio fx-CG50

## V30 — installatie en gebruik

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

Werk voor V30 zowel de Atom als de Casio-app bij. Upload eerst de meegeleverde
Atom-firmware V30 en installeer daarna de Casio-app volgens de stappen hieronder.
Laat volledig wissen uitgeschakeld om wifi, API-sleutel en model normaal te
behouden. Laat CGPT3 staan: daarin staan je chats en scherminstellingen.

Met een oudere Atom blijven bestaande chats compatibel, maar F2 Account vraagt
om Atom V30 voor de vernieuwde accountcontrole en gedeelde modelkeuzes.

## Casio-app installeren

1. Pak de volledige ZIP uit naar een gewone map in Windows.
2. Verbind de Casio via zijn USB met Windows en kies USB Flash / USB-opslag.
3. Kopieer dist/ChatGPT.g3a naar de hoofdmap van het Casio-station. Vervang het
   bestaande bestand; bewaar geen tweede kopie zoals ChatGPT (1).g3a.
4. Laat de map CGPT3 staan als je chats wilt behouden.
5. Werp het station veilig uit en ontkoppel de Casio-USB.
6. Herstart de Casio eenmaal. Open MENU en kies ChatGPT.
7. Instellingen → Meer → Over deze app toont Versie 30.

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

Druk F1 Stuur of EXE om je bericht te versturen. De bewegende paarsblauwe planeet betekent
dat de vraag wordt verwerkt. EXIT of F1 annuleert het wachten. De laadplaneet
verdwijnt meteen, voordat de Atom de annulering verwerkt. Er start geen nieuwe
betaalde vraag. Een al verstuurde
API-vraag kan nog kosten maken; stuur niet meteen opnieuw bij een onzekere uitkomst.

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

Instellingen → Model toont herkenbare modelnamen uit de GPT-4-, GPT-5- en
GPT-6-families. De meegeleverde keuzehulp legt uit waarvoor de verschillende
modellen passen. De gecontroleerde selectie en bronvermelding staan in
docs/V30-MODELLEN.md. Toegang, tarieven en beschikbaarheid kunnen veranderen.

De modelkeuze opent direct met vaste keuzes, zonder internetaanvraag. Dit is
nog geen controle van jouw API-account.

F2 Account controleert welke ondersteunde modelnamen jouw account teruggeeft.
Gebruik hiervoor de meegeleverde Atom-firmware V30. De controle gebruikt één
gestreamde GET-aanvraag met begrensd geheugengebruik, in plaats van een aparte
aanvraag per model. De volledige accountlijst wordt niet tegelijk in het
werkgeheugen bewaard. Een oudere Atom vraagt bij deze functie om een update.

Na een geslaagde controle staat er Account gecontroleerd en zie je de passende
namen uit jouw account. Een geslaagde controle zonder passende namen geeft een
lege lijst; je bestaande model blijft bewaard. Modelmetadata alleen garandeert
geen geslaagde chatvraag: ook de chatrechten, internet en je tegoed moeten kloppen.

Mislukt de controle, dan staat er Account niet gecontroleerd. De uitleg en de
regel onder de lijst maken duidelijk dat je vaste, ongecontroleerde keuzes ziet.
Een onleesbare accountrespons krijgt een melding over de modellenlijst, niet
over een chatantwoord. Controleer wifi en je API-account en probeer F2 Account
opnieuw. De app toont geen gedeeltelijke controle als volledig succes.
Er is geen betaalde chatvraag verstuurd.

Omhoog/omlaag kiest een model. De lijst schuift automatisch mee; een kleine
scrollbalk geeft je positie aan. Er zijn geen Vorig/Volgend-knoppen. F4 Info
vertelt bij welke vragen het model past en geeft kort keuzeadvies. Scroll met
de pijlen om de uitleg te lezen; EXIT of links gaat terug naar dezelfde regel.
Dit appadvies is geen benchmark of prestatiegarantie. Snelheid hangt ook af van
je vraag en de dienst. Controleer actuele prijzen op https://openai.com/api/pricing/.

EXE, rechts of F1 Kies slaat de keuze op. Actief markeert je huidige model.
F2 Account controleert opnieuw. Ophalen, Info openen en kiezen versturen geen
betaalde chatvraag. De app verstuurt pas een vraag als jij Stuur kiest.
Een bestaande modelinstelling wordt nooit stilzwijgend vervangen. Kies zo nodig
zelf een beschikbare naam en test daarna een echte vraag.

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

Vraag lijkt vast te staan: annuleer het wachten. De aanvraag kan al verstuurd
zijn. De app vraagt niet automatisch opnieuw betaald; controleer de uitkomst
voordat je zelf nogmaals verstuurt.

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

## Over dit pakket

V30 bevat de bijgewerkte Casio-app en de nieuwe Atom-firmware V30. De voorbeeldschermen
zijn softwareweergaven met voorbeeldgegevens. Het is een onafhankelijk project.
De licentie- en bronvermeldingen zijn meegeleverd in docs.

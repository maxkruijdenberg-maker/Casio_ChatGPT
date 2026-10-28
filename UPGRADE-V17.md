# V17 — lichte stijl, eenvoudig menu en accountmodellen

## Upgraden vanaf V11 t/m V16

Deze keer moet je **beide onderdelen** bijwerken. De nieuwe modellenlijst vereist
Atom V17. Wifi, API-sleutel, model en chats kunnen behouden blijven.

### 1. Atom bijwerken op Windows

1. Pak de volledige ZIP uit naar een gewone map.
2. Haal de Atom-USB los en ontkoppel tijdelijk de Casio TX/RX-draden van de Atom.
3. Sluit de Atom via USB op Windows aan.
4. Open `atom\ChatGPTAtom\ChatGPTAtom.ino` in Arduino IDE 2.x.
   Houd alle `.h`-bestanden ernaast; ook de nieuwe `model_catalog.h` is nodig.
5. Gebruik **esp32 by Espressif Systems 3.3.0**, **ArduinoJson 7.4.2**,
   board **M5Atom**, juiste COM-poort en uploadsnelheid **115200**.
6. Laat **Erase All Flash / volledig wissen uitgeschakeld** om je instellingen
   te behouden. Klik Upload. `Hard resetting via RTS pin...` betekent dat upload klaar is.
7. Sluit Serial Monitor en haal de USB los. Sluit de eerder gecontroleerde draden
   terug aan en geef de Atom opnieuw USB-voeding.

Je kunt ook de voorgebouwde firmware flashen met `py tools\flash_windows.py COM5`
na installeren van esptool 5.0.0; vervang COM5 door je eigen poort. Zie README.md.
Gebruik geen erase-flash-opdracht als je je instellingen wilt houden.

### 2. Casio-app vervangen

1. Verbind de Casio via zijn USB met Windows, kies USB Flash / USB-opslag.
2. Vervang `ChatGPT.g3a` in de hoofdmap door `dist\ChatGPT.g3a` uit deze ZIP.
3. Laat `CGPT3` staan; daar staan je chats en scherminstellingen.
4. Werp veilig uit, ontkoppel de Casio-USB en herstart de Casio eenmaal.
5. Open ChatGPT. Instellingen → Meer → Over deze app toont **Versie 17.0**.

Een nog oude Atom werkt voor de bestaande functies, maar toont alleen de drie
startkeuzes voor modellen. Upload Atom V17 voor de accountlijst.

## De nieuwe bediening

Instellingen heeft vier keuzes:

- Wifi en account: telefooninstallatie voor wifi en je eigen API-sleutel.
- Model: de modellenlijst van je eigen account.
- Hulp: Aan de slag, Typen en toetsen, Antwoorden en kosten, Verbinding controleren.
- Meer: Scherm aanpassen, Over deze app, Wifi en sleutel wissen.

EXE, pijl rechts of F1 opent een keuze; EXIT, links of F6 gaat terug in lijsten.
Bij Scherm aanpassen wijzigen links/rechts de waarde en annuleert EXIT.
Alleen MENU vraagt om de app af te sluiten. Blijven bewaart je concept.

Tijdens wachten verschijnt een ChatGPT-logo op een lichte kaart met bewegend
accent. Geen technische voortgangslijst of versienummer. EXIT/F1 annuleert het
wachten. Een al verstuurde chatvraag kan nog kosten maken.

## Modellen uit je account

Open Model. Met Atom V17 wordt de accountlijst opgehaald. Dit kan de eerste keer
langer duren door wifi, beveiligde verbinding en het overdragen van de lijst.

- Omhoog/omlaag kiest een model; F3 Vorig / F5 Volgend bladert vijf regels.
- EXE, rechts of F1 Kies slaat de selectie op. De bestaande keuze blijft bij annuleren.
- F2 Ververs haalt de accountlijst opnieuw op; F4 Info geeft uitleg.
- Ophalen en kiezen verstuurt **geen chatvraag** en doet geen completion-POST.
- Bij geen verbinding blijven drie startkeuzes beschikbaar; dat is geen volledige
  accountlijst. Het onderschrift maakt het verschil zichtbaar.

De Atom gebruikt een geauthenticeerde HTTPS `GET /v1/models` en haalt alleen de
model-ID's uit de JSON. V17 toont GPT/ChatGPT- en o-reeksen, inclusief geschikte
namen van aangepaste modellen. Bekende audio-, realtime-, beeld-, embedding-,
moderatie-, instruct- en deep-researchvarianten worden uitgefilterd.

**Deze API geeft geen endpoint-capabilities mee.** Daarom is dit een lijst met
tekst-chatkandidaten, geen garantie dat iedere keuze met de gebruikte Chat
Completions API werkt. Sommige modellen vereisen bijvoorbeeld Responses.
Web-only modellen uit een ChatGPT-abonnement zijn niet automatisch API-modellen.
Een model wordt bij een vraag op accounttoegang gecontroleerd; een ongeschikte
keuze geeft een fout en wordt niet automatisch via een andere betaalde route geprobeerd.

De lijst ondersteunt maximaal 384 unieke namen van maximaal 80 tekens en 32767
bytes. Een te grote lijst wordt afgewezen, niet stilletjes afgekapt. Accountrechten,
API-tegoed en actuele prijzen verschillen; de app verzint geen prijzen.
De voorbeeldlijst in de screenshots komt uit hosttestdata.

## Behoud en controle

Geen wijziging aan pins, baudrate, voeding, slaapstand of de native menu-route.
De V13-aanpak van fysieke toetsen en verbindingsherstel blijft behouden.
Chats/opslag, telefooninstallatie, vraag-ledger en =-weergave blijven beschikbaar.
Het opslagrecord `config11` behoudt bewust zijn naam om eerdere instellingen te lezen.

Veertien ASan/UBSan-testgroepen controleren onder meer de lijstparser, filteren,
deduplicatie, paginering/keuze, overdracht in delen, annuleren, aangepaste modelnamen,
veilige grenzen, de vraagroute en 40 gesimuleerde menuheropeningen.
De Casio- en Atom-firmware worden beide gecompileerd. Er is geen fysieke hardware
of API-sleutel aanwezig in deze bouwomgeving.

Controleer op jouw apparaat: veel typen, twintigmaal MENU/heropenen en daarna
andere Casio-apps, alle vier instellingen via EXE/rechts, iedere Hulp-pagina,
nieuwe laadanimatie, = in een echt antwoord en een echte accountlijst met eigen
sleutel. Test modellen die je wilt aanbieden afzonderlijk; claim geen algemene
ondersteuning of verkoopklaarheid op basis van alleen deze softwaretests.

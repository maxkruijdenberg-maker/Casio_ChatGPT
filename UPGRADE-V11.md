# V11 installeren — telefooninstallatie en menuherstel

V11 voegt installatie via de telefoon toe en wijzigt het opruimen bij het verlaten
van ChatGPT vanwege de gemelde System ERROR bij andere pictogrammen:
`ADDRESS(W) TARGET=00000001 PC=8C09CA4` (zoals doorgegeven).
De app registreert geen eigen quit-callback meer. UART, opslag en toetsenstatus
worden synchroon vóór de native menuoverdracht vrijgegeven. De werkende native
menu-route, pictogrammen, schermvulling, ALPHA, SHIFT en chatbediening blijven.

De exacte PC kan zonder de bijbehorende OS-ROM niet naar een instructie worden
herleid. Softwaretests controleren de nieuwe levensduur van callbacks en
resources; zij bewijzen geen herstel op een fysieke Casio. Controleer dit eerst
met de onderstaande apparaattest.

## 1. Werk beide programma's bij

**Anders dan V4–V10 vereist deze update ook nieuwe Atom-firmware.**

1. Pak het volledige V11-pakket uit op Windows.
2. Zet de Casio uit. Maak de Casio TX/RX-draden tijdelijk los van de Atom.
3. Verbind de Atom via USB met de laptop.
4. Open `atom\ChatGPTAtom\ChatGPTAtom.ino` in Arduino IDE.
   Houd alle bijbehorende `.h`-bestanden in dezelfde map.
5. Board: **M5Atom**, originele ESP32-PICO-D4; ESP32-boardpakket **3.3.0**,
   ArduinoJson **7.4.2**, juiste COM-poort, uploadsnelheid **115200**.
6. Laat **Erase All Flash Before Sketch Upload op Disabled** staan om bestaande
   instellingen te behouden. Klik Upload. `Hard resetting via RTS pin...`
   betekent dat upload klaar is; dit hoeft niet uit beeld te verdwijnen.
7. Ontkoppel de Atom-USB, verbind de eerder gecontroleerde draden weer en sluit
   USB weer aan voor voeding. Aansluitingen en baudrate blijven onveranderd.
8. Sluit de Casio via zijn eigen USB aan, kies **USB Flash / USB-opslag**.
9. Vervang alleen `ChatGPT.g3a` in de hoofdmap door `dist\ChatGPT.g3a` uit V11.
   Verwijder geen `CGPT3`-map: daarin staan je lokale chats en scherminstellingen.
   Voorkom een extra bestand `ChatGPT (1).g3a`.
10. Werp het Casio-station veilig uit, ontkoppel zijn USB en herstart de Casio
    één keer. Bij een bestaande System ERROR gebruik je de normale RESTART;
    kies geen geheugen wissen/fabrieksreset.
11. Open ChatGPT → F6 Instellingen → Versie: **11.0 / Atom 11.0**.

## 2. Stel in via je telefoon

1. Open **F6 Instellingen → F2 Telefoon**.
2. Op de Casio verschijnen netwerk `ChatGPT-XXXX` en een willekeurig wachtwoord.
3. Verbind je telefoon met dat netwerk. Kies zo nodig **Verbonden blijven zonder
   internet**. De melding dat dit netwerk geen internet heeft is normaal.
4. Open in Safari/Chrome **http://192.168.4.1**. Gebruik expliciet `http://`.
   Een automatisch geopend wifi-aanmeldvenster kan ook werken; de browser is
   beschikbaar als alternatief. Dit vereist geen internet op het installatienetwerk.
5. Vul de exacte naam en het wachtwoord van je eigen **2,4GHz-wifi** in.
   Gebruik een router of een hotspot van een **andere** telefoon; de telefoon
   waarmee je installeert kan niet tegelijk zijn eigen installatienetwerk joinen
   en de benodigde hotspot voor de Atom verzorgen.
6. Plak je **volledige OpenAI Secret API key** (`sk-...` of `sk-proj-...`).
   Maak/kopieer die vooraf en deel hem niet met verkoper of andere personen.
7. Kies **Opslaan en verbinden**. De pagina bevestigt het opslaan en het
   installatienetwerk sluit na ongeveer twee seconden. De telefoon kan terug
   naar zijn gewone wifi. Laat het Casio-installatiescherm nog even open.
8. De Casio meldt **Verbonden — klaar voor gebruik** zodra wifi verbonden is.
   Dit controleert wifi, nog geen internet, API-tegoed of geldigheid van de sleutel.
   Die worden bij je eerste zelf verstuurde vraag gecontroleerd.
9. Druk F1 of EXIT voor Terug en stel een korte vraag met F1.

Je bestaande modelkeuze blijft behouden. Nieuw apparaat: standaard
`gpt-4.1-mini`. Kies desgewenst een ander model via Instellingen → Model.
Een eigen OpenAI API-account met betaald tegoed is nodig. ChatGPT Plus geeft
geen API-tegoed. Er worden bij installeren geen betaalde vragen verstuurd.

## Herstellen en opnieuw instellen

- Geen wifi-verbinding? Controleer de gegevens en 2,4GHz, open opnieuw
  **F2 Telefoon** en corrigeer ze. De pagina toont opgeslagen geheimen nooit terug.
- Verkeerde sleutel of later vervangen? Volg dezelfde installatie opnieuw.
- F1/EXIT sluit het tijdelijke netwerk; anders sluit het na tien minuten.
- Elke nieuwe installatie krijgt een nieuw wachtwoord. Er is maximaal één
  wifi-client tegelijk. De pagina is alleen beschikbaar tijdens deze installatie.
- Instellingen → **Instellingen wissen** wist de actieve wifi, sleutel en
  modelkeuze op de Atom. Lokale Casio-chats blijven bestaan en kunnen in Chats
  worden verwijderd. Wis beide vóór overdracht aan een andere gebruiker.
- De Atom bewaart gegevens lokaal in zijn flash. Dit is geen opslag die bestand
  is tegen fysieke uitlezing van het apparaat. De installatie gebruikt HTTP
  binnen het tijdelijke, met WPA2-wachtwoord beveiligde netwerk.
- Ontbreekt de Atom-update? De Casio legt bij F2 uit dat V11-firmware nodig is;
  de bestaande handmatige instellingen blijven beschikbaar.

## Apparaattest vóór verkoop

1. Open ChatGPT, EXIT → Ja, open RUN-MAT en voer `23+42` uit. Verwacht `65`
   zonder System ERROR. Open ook SYSTEM, STAT en andere aanwezige apps.
2. Open ChatGPT opnieuw en herhaal dit minstens vijf keer; ook met fysieke MENU.
3. Test telefooninstallatie met juiste én verkeerde wifi-gegevens, annuleren,
   tien minuten timeout, opnieuw instellen en Atom stroom uit/aan.
4. Controleer na stroom uit/aan dat de opgeslagen instellingen nog werken.
5. Stuur pas zelf een testvraag wanneer je API-tegoed wilt gebruiken.

Elf hosttestgroepen met ASan/UBSan en beide firmwarebuilds zijn beschikbaar.
Er is hier geen fysiek apparaat, OS-ROM of telefoon aanwezig voor deze tests.

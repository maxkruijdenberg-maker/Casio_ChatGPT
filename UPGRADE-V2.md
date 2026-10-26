# Van V1 naar ChatGPT V2 — Windows

Historische V2-handleiding. Voor de huidige V3: zie **UPGRADE-V3.md**.

Werk **zowel de Atom als ChatGPT.g3a** bij. Alleen de afbeelding of de Casio-app
vervangen geeft niet alle verbeteringen. De Atom houdt dezelfde NVS-naam,
instellingssleutels en partitietabel: een gewone upload hoort bestaande
hotspotgegevens en API-sleutel te behouden. Dit is nog niet op jouw apparaten
gevalideerd. Gebruik geen 'Erase all flash' als je instellingen wilt behouden.

## 1. Download de nieuwste versie

Download de volledige ZIP van deze repository via Code → Download ZIP, of de
V2-tag. Pak uit naar een **nieuwe map**, bijvoorbeeld `C:\CasioChatGPT-V2`.
Zo voorkom je een combinatie van oude en nieuwe bestanden.

## 2. Werk de Atom bij via Arduino IDE

1. Sluit de Casio-app af met MENU en zet de Casio uit.
2. Haal de USB-kabel uit de Atom. Maak de twee signaaldraden aan G32 en G26
   tijdelijk los; GND mag blijven zitten.
3. Sluit de Atom weer via USB op Windows aan.
4. Open **de nieuwe** `atom\ChatGPTAtom\ChatGPTAtom.ino`. Laat alle `.h`-bestanden
   erbij staan. Je hoeft geen sleutel of wachtwoord in de code te zetten.
5. Gebruik dezelfde omgeving als bij V1:
   - esp32 by Espressif Systems **3.3.0**;
   - board **M5Atom**, originele ESP32;
   - ArduinoJson **7.4.2**;
   - juiste COM-poort, uploadsnelheid **115200**.
6. Klik op het vinkje en daarna Uploaden. 'Hard resetting via RTS pin...' is
   de laatste normale melding; die hoeft niet te verdwijnen.
7. Koppel de Atom-USB los. Herstel TX via 1 kΩ naar G32, G26 via 1 kΩ naar
   Casio RX en gemeenschappelijke GND. Verander geen voedingsaansluitingen.
8. Voed de Atom weer via USB en zet de Casio op zijn eigen AAA-batterijen aan.

Alternatief: het meegeleverde flashhulpmiddel gebruikt de nieuwe bestanden uit
`dist`, controleert hun hashes en schrijft de afzonderlijke flashgebieden.
Zie README.md. Normaal hoeft de firmware niet apart met Python geflasht te worden
als de Arduino-upload al gelukt is.

## 3. Werk de Casio-app bij

1. Sluit ChatGPT af en verbind de Casio met zijn USB-kabel aan Windows.
2. Kies F1 **USB Flash**.
3. Vervang in de hoofdmap het oude `ChatGPT.g3a` door **het nieuwe bestand uit
   `dist\ChatGPT.g3a`**. Kopieer geen broncode of Atom-firmware naar de Casio.
4. Werp het Casio-station veilig uit en ontkoppel alleen de Casio-USB.
5. Open ChatGPT via MENU. Je ziet het nieuwe logo en de donkere interface.

## 4. Controleer voordat je een betaalde vraag stelt

- F3: 'Atom antwoordt op protocol v1' blijft correct. Het **transportprotocol
  is versie 1**; de **app en firmware zijn V2.0**. Een waarschuwing over Atom
  V1 betekent dat je nog oude firmware gebruikt.
- F6 Instellingen → Versie moet **2.0 / Atom 2.0** tonen nadat de verbinding
  gecontroleerd is. De kop 'Atom OK' betekent recente UART-communicatie,
  niet dat wifi of OpenAI al werkt. 'Atom ?' vraagt om een nieuwe controle.
- Controleer hotspotnaam, model en de status 'Ingesteld' bij de sleutel en
  het wachtwoord. Ze horen normaal bewaard te zijn. Ontbreekt een waarde,
  voer die alleen lokaal opnieuw in en sla op.
- Laat het model voor de eerste test op `gpt-4.1-mini` staan. Zet de 2,4GHz-
  hotspot aan en verstuur één korte vraag. Dat kan API-kosten maken.

## Nieuw in V2

- Donkere ChatGPT-stijl met mintgroene accenten, logo, welkomstscherm,
  vraagballon, antwoordweergave, laadstappen en scrollbalk.
- Eigen 92×64-menu-iconen, normaal en geselecteerd. Alleen het ChatGPT-icoon
  verandert; de rest van het Casio-systeemmenu houdt zijn eigen stijl.
- SHIFT-lettervergrendeling, F5/ALPHA-hoofdletters en eerste gewone letter
  automatisch als hoofdletter blijven behouden. Instellingen wijzigen de
  letterkast nooit automatisch.
- Antwoorden breken op woorden af; pagina's stoppen bij de laatste volle
  pagina. Langere foutmeldingen kunnen ook gescrold worden.
- Instellingen toont hotspotnaam en huidig model. De API-sleutel en het
  wachtwoord worden nooit teruggestuurd of zichtbaar gemaakt.
- Offline instellingen tonen Onbekend in plaats van een misleidende lege
  configuratie. F3 vernieuwt de verbinding. Mislukte opslag bewaart je
  invoer tijdelijk en afgeschermd; EXIT/MENU verwerpt die wijziging.
- Atom-geheugenbeheer hersteld: taakobjecten worden afgebroken voordat de
  FreeRTOS-taak stopt. Wifi en tijd worden waar mogelijk hergebruikt.
- Uploads hebben een expliciete toestand, grenzen en invalidatie. Een oude
  of onvolledige upload kan niet opnieuw worden toegepast.
- Tekst, sleutels, modellen en API-antwoorden worden beter gecontroleerd.
  API-weigering en een afgebroken antwoord krijgen een eigen afhandeling.
- Bescherming tegen dubbele POSTs per duurzaam aanvraag-ID blijft behouden.
- **Geen slaapstand en geen geldlimiet in de app.** Automatisch prepaid
  opwaarderen en eventuele budgetmeldingen beheer je in je OpenAI-account.

## Wat is wel en niet getest?

Beide programma's zijn gecompileerd. De softwaretests gebruiken sanitizers en
controleren transport, aanvraagregistratie, uploadgrenzen, Unicode, de echte
editor met hostapparaatstubs, woordomloop en de echte API-responseparser met
vaste testdocumenten. Die documenten zijn geen vervanging voor een echte API.

De afbeeldingen in `docs/design/V2-*.png` zijn uitvoer van de werkelijke
384×216-renderer in een hosttest. Het zijn geen apparaatfoto's. Er is in deze
omgeving geen fysieke Casio/Atom en geen API-sleutel gebruikt. Test V2 op jouw
apparaten volgens `docs/HARDWARETEST.md`. Windows-scripts zijn hier niet op
Windows uitgevoerd.

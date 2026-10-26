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

> Nieuwste versie: [V10 installeren](UPGRADE-V10.md).

> Nieuwste versie: [V9 installeren](UPGRADE-V9.md).

> Nieuwste versie: [V8 installeren](UPGRADE-V8.md).

> Nieuwste versie: [V7 installeren](UPGRADE-V7.md).

# V6 — direct naar MENU en blauwe pictogramselectie

**Alleen `ChatGPT.g3a` vervangen als je Atom V3 hebt.** Je hoeft de Atom niet
opnieuw te programmeren. Je API-sleutel, chats en scherminstellingen blijven.

## Installeren op Windows

1. Download de V6-ZIP en pak hem uit naar een nieuwe map.
2. Gebruik op de Casio de fysieke MENU-toets als de oude app blijft staan.
   Sluit de Casio via USB aan en kies **F1 USB Flash**.
3. Vervang het bestaande bestand in de hoofdmap door **dist/ChatGPT.g3a**.
   Plaats geen tweede kopie met '(1)' in de naam naast de oude app.
4. Laat **CGPT3** met je chats en scherminstellingen staan.
5. Werp het station veilig uit en ontkoppel de Casio-USB-kabel.
6. Open ChatGPT en laat EXE los. Bij F6 Instellingen → Versie hoort
   **6.0 / Atom 3.0** te staan.

## Afsluiten: nieuwe volgorde

- Na EXIT → F1 Ja wordt eerst gewacht tot je de toetsen loslaat.
- De UART blijft open. Er wordt vooraf geen CANCEL verstuurd en geen
  Serial_Close aangeroepen.
- De app schakelt menu-terugkeer expliciet in en roept **DisplayMainMenu**
  rechtstreeks aan. De synthetische MENU-toets uit V4/V5 is verwijderd.
- De OS-afsluitcallback ruimt de verbinding op bij het wisselen naar een
  andere app. Als het menu terugkeert naar dezelfde add-in zonder callback,
  ruimt de app alsnog op vóór de nieuwe sessie.

De bedoeling is dat je terug naar MENU gaat terwijl de verbinding tijdens
de overdracht intact blijft. De software kan niet controleren of een
specifiek fysiek schermbeeld al zichtbaar is; de OS-appwissel is het gebruikte
moment. De Atom blijft via USB gevoed: dit is geen fysieke stroomschakelaar
of slaapstand. Een al lopende API-aanvraag kan op de Atom worden afgemaakt;
alleen afsluiten of heropenen start geen nieuwe API-aanvraag.

## Pictogram

Het geselecteerde pictogram heeft een **rechte lichtblauwe rechthoek** en
**witte achtergrond**, met hetzelfde zwarte logo en dezelfde 92×64-afmetingen.
Geen zwarte omlijsting, afgeronde hoeken, schaduw of grijs verloop meer.
Casio tekent de appnaam zelf, dus geen dubbele tekst. Een fx-CG50-menuscherm
is gebruikt als visuele vergelijking; bron en beperkingen staan in
[docs/V6-REFERENCES.md](docs/V6-REFERENCES.md). De definitieve vergelijking met
jouw OS-menu moet op het apparaat gebeuren.

## Controle op de Casio

1. Open ChatGPT → F3. Controleer dat de Atom antwoordt en keer terug met EXIT.
2. Druk opnieuw EXIT → F1 Ja en laat F1 los. Je hoort het gewone Casio-menu
   te zien. Open RUN-MAT om te controleren dat het menu echt bedienbaar is.
3. Open ChatGPT opnieuw en test F3. Herhaal de route vijfmaal.
4. Test ook EXIT → Nee en de fysieke MENU-toets vanuit ChatGPT.
5. Selecteer het ChatGPT-pictogram naast een standaardpictogram: wit vlak,
   zwarte logo, rechte blauwe rand, één naam. De donkere schermranden in
   ChatGPT moeten hetzelfde blijven als in V5.
6. Controleer of je opgeslagen chats en scherminstellingen behouden zijn.

## Verificatie

Negen softwaretestgroepen met ASan/UBSan slagen. De controller loopt drie
sessies met een geopende UART, vastgehouden toetsen, gewijzigd VRAM/DMA,
EXIT → Ja, EXIT → Nee en MENU door. Sluiten vóór de gesimuleerde menu-entry
laat de test falen. Ook de callbackloze terugkeer en ontbreken van een
CANCEL/nieuwe betaalde aanvraag worden gecontroleerd. De volledige framebuffer
en aparte schermrand houden hun bestaande controles.

De echte .g3a is opnieuw gecompileerd; Atom-broncode en binaries zijn gelijk
aan V3. **Het werkelijke Casio-menu en de timing van de OS-callback zijn hier
niet uitgevoerd.** Hoststubs zijn geen bewijs dat het op jouw calculator
werkt; bovenstaande fysieke controle blijft nodig.

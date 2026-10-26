> Nieuwste versie: [V10 installeren](UPGRADE-V10.md).

> Nieuwste versie: [V9 installeren](UPGRADE-V9.md).

> Nieuwste versie: [V8 installeren](UPGRADE-V8.md).

> Nieuwste versie: [V7 installeren](UPGRADE-V7.md).

> Nieuwste versie: [V6 installeren](UPGRADE-V6.md).

# V5 — terug naar MENU, schermranden en pictogramselectie

**Met Atom V3 hoef je alleen `ChatGPT.g3a` te vervangen.** Geen nieuwe
Atom-upload of API-sleutel nodig. De Atom-bestanden zijn gelijk aan V3/V4.
Je chats en instellingen blijven bewaard.

## Installeren

1. Pak de V5-ZIP uit naar een nieuwe map op Windows.
2. Als de oude app blijft hangen, druk op de Casio op MENU. Sluit daarna de
   Casio via USB aan en kies **F1 USB Flash**.
3. Kopieer **dist/ChatGPT.g3a** naar de hoofdmap van het Casio-station.
   Vervang het bestaande bestand; maak geen tweede kopie met '(1)' in de naam.
4. Laat de map **CGPT3** met chats en scherminstellingen staan.
5. Werp het station veilig uit en ontkoppel de Casio-USB-kabel.
6. Open ChatGPT en laat EXE los. F6 Instellingen → Versie toont
   **5.0 / Atom 3.0**. Dat verschil is correct.

## Wat is aangepast?

- **EXIT → Ja:** na het sluiten van de seriële verbinding wordt de klaargezette
  MENU-toets door de volledige OS-routine **GetKey** verwerkt. V4 gebruikte
  GetKeyWait_OS; die route bracht de gebruiker niet daadwerkelijk naar MENU.
  De app begint pas een nieuwe sessie wanneer de OS-routine terugkeert.
- **Witte randen:** naast de volledige 384×216-framebuffer wordt nu ook de
  aparte Casio-rand met **DrawFrame** donker gekleurd. Alleen zoomen of meer
  pixels naar VRAM schrijven vervangt deze OS-rand niet. Bij afsluiten wordt
  de gewone witte rand hersteld. De framebuffer wordt niet buiten zijn
  afmetingen beschreven.
- **Geselecteerd pictogram:** een eigen geselecteerde bitmap tekent het hele
  blok met een neutrale Casio-stijl, omlijsting en schaduw. De normale bitmap
  blijft wit met een zwart logo. Beide bevatten geen naamtekst; Casio tekent
  de appnaam en bepaalt de systeemkleur van de tekst/focus zelf.

## Kort testen op jouw Casio

1. Open ChatGPT → EXIT → F1 Ja. Je moet het gewone Casio-menu zien en daar
   bijvoorbeeld RUN-MAT kunnen openen. Alleen een onderbroken Atom-link is
   niet voldoende.
2. Open ChatGPT opnieuw → F3: de Atom hoort weer bereikbaar te zijn.
3. Herhaal dit vijfmaal. Test ook EXIT → Nee en afsluiten met MENU.
4. Test de weergave eerst op standaard: **F6 → F4 Scherm → F3 100% → F1**.
   Controleer de onderrand, zijranden en hoekentest met F2. De rand hoort
   donker te zijn zonder dat tekst of functietoetsen worden afgeknipt.
5. Loop in het gewone menu naar en van het ChatGPT-pictogram. Controleer dat
   het hele pictogramblok verandert en dat de naam één keer zichtbaar is.
6. Controleer dat gewone Casio-apps hun normale witte rand behouden en je
   opgeslagen chats nog bereikbaar zijn.

De schaalinstelling van V4 blijft beschikbaar, inclusief de bestaande opslag
in D0A.dat/D0B.dat. Voor de schermrand is groter zoomen niet nodig.

## Verificatie en beperking

De Casio-binary is opnieuw gebouwd; negen softwaretestgroepen met ASan/UBSan
slagen. De controller is drie sessies getest, inclusief EXIT → Ja, EXIT → Nee,
MENU, vastgehouden toetsen en veranderde VRAM/DMA-toestand. De menu-teststub
accepteert nu uitsluitend GetKey voor de geïnjecteerde MENU-toets: de V4-route
faalt daarmee. Het LCD-model controleert de complete framebuffer en daarnaast
apart de randkleur bij tekenen, zoomen en afsluiten. Dit is hostsimulatie;
het daadwerkelijke Casio-OS en de menustijl zijn hier niet fysiek getest.
Er zijn geen betaalde API-aanvragen gedaan.

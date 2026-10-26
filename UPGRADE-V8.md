> Nieuwste versie: [V10 installeren](UPGRADE-V10.md).

> Nieuwste versie: [V9 installeren](UPGRADE-V9.md).

# V8 — sluiten én opnieuw openen, wit paneel in het pictogram

**Met Atom V3 hoef je alleen ChatGPT.g3a te vervangen.** Geen Atom-upload of
nieuwe API-sleutel nodig. Chats en scherminstellingen blijven behouden.

## Installeren

1. Download de V8-ZIP en pak hem uit naar een nieuwe map op Windows.
2. Sluit de Casio via USB aan en kies **F1 USB Flash**.
3. Vervang het bestaande bestand in de hoofdmap door **dist/ChatGPT.g3a**.
   Gebruik precies die naam en plaats geen tweede '(1)'-kopie naast de oude app.
4. Laat de map **CGPT3** met chats en scherminstellingen staan.
5. Werp het station veilig uit en ontkoppel de Casio-USB-kabel.
6. Open ChatGPT en laat EXE los. Controleer bij F6 Instellingen → Versie dat
   **8.0 / Atom 3.0** staat. Zie je 7.0, dan draait de nieuwe app nog niet.

## Afsluiten en opnieuw openen

V7 verliet main na de menu-aanroep. Op jouw Casio sloot dat wel af, maar een
tweede start viel met een kort laadscherm terug naar MENU. V8 gebruikt daarom
de native **GetKey**-menuafhandeling met een MENU-matrixtoets (kolom 4, rij 9).
Een koude start en een hervatting vanuit deze native afhandeling resetten
beide de sessiestatus. V8 roept DisplayMainMenu niet meer direct aan.

Voor beide overgangen worden eerst de toetsen losgelaten en oude bewaarde
OS-toetsgebeurtenissen weggehaald. Dat is nodig omdat de editor rechtstreeks
PRGM-toetsen leest, buiten de normale GetKey-verwerking om. Een oude F1/EXE
mag niet opnieuw een keuze in het menu of in de app veroorzaken.

De applicatie tekent geen ChatGPT-frame terwijl de native GetKey-menufunctie
actief is. Er wordt vooraf geen CANCEL verstuurd en de UART sluit via de
OS-afsluitcallback of bij terugkeer uit de menuafhandeling. De via USB gevoede
Atom blijft fysiek aan; er is geen slaapstand toegevoegd.

Dit beschrijft de nieuwe implementatie. De oorzaak van het exacte OS-gedrag
is hier niet met een fysiek apparaat vastgesteld. Zie docs/V8-NATIVE-MENU.md.

## Pictogram

- **Normaal:** witte achtergrond, zwart logo.
- **Geselecteerd:** een witte rechthoek achter het zwarte logo, met de blauwe
  selectie eromheen en in het gebied voor de appnaam.
- Het 32×32-logo blijft volledig boven de naamruimte. De witte rechthoek
  loopt van (4,2) tot (87,39); onderaan blijft plaats voor de native appnaam.
- Casio tekent ChatGPT zelf op zijn normale lettergrootte. Er is geen tweede
  naam in de bitmap getekend. De volledige pictogrammaat blijft 92×64.
- De werkende donkere randen van het geopende ChatGPT-scherm blijven.

## Controleer deze volgorde

1. Controleer dat de versie **8.0** is. Test F3 met je aangesloten Atom.
2. Ga terug naar invoer, kies EXIT → F1 Ja en laat F1 los.
3. Het Casio-menu moet blijven staan. Open ChatGPT opnieuw en laat EXE los.
   Er hoort weer een bruikbaar invoerscherm te verschijnen, geen terugval.
4. Herhaal sluiten en openen vijfmaal, ook éénmaal met RUN-MAT ertussen.
5. Test ook EXIT → Nee en de fysieke MENU-toets vanuit ChatGPT.
6. Controleer F3 na opnieuw openen en behoud van je opgeslagen chats.
7. Loop naar en van het pictogram. Controleer de witte rechthoek achter het
   logo, blauw daarbuiten en de volledig leesbare naam eronder.

## Teststatus

De daadwerkelijke .g3a is opnieuw gebouwd. Negen softwaretestgroepen met
ASan/UBSan slagen. De lifecycle-test modelleert drie keer de blokkerende native
GetKey-menuovergang, met zowel OS-afsluitcallback als opruimen na hervatting,
oude toetsgebeurtenissen, vastgehouden toetsen, een ander VRAM-adres en een
geërfd beperkt DMA-venster. Geen nieuwe API-aanvraag of CANCEL tijdens alleen
sluiten/heropenen. De volledige framebuffer en aparte rand blijven getest.

De verpakkingscontrole controleert beide ingebedde RGB565-pictogrammen, de
native naamvelden en de vrije naamruimte. Atom-broncode en binaries blijven
bytegelijk aan V3. **De echte Casio-OS-dispatcher is hier niet uitgevoerd.**
Groene hosttests bewijzen de fysieke menu-/herstartwerking niet; de bovenstaande
controle op jouw apparaat blijft nodig.

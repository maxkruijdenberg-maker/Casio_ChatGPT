> Opvolgende correctie: [UPGRADE-V20.1.md](UPGRADE-V20.1.md). Voor de ongefilterde accountlijst moet ook de Atom worden bijgewerkt.

# V21 — verfijnd logo, donkere modus behouden

## Installeren vanaf V17 t/m V20

1. Pak de ZIP uit.
2. Verbind de Casio met Windows en kies USB Flash / USB-opslag.
3. Vervang `ChatGPT.g3a` in de hoofdmap door `dist\ChatGPT.g3a` uit deze ZIP.
4. Laat `CGPT3` staan: daarin staan je chats en scherminstellingen.
5. Werp veilig uit, ontkoppel de Casio-USB en herstart de Casio eenmaal.
6. Instellingen → Meer → Over deze app toont **Versie 21.0**.

De Atom hoeft vanaf V17 niet opnieuw geüpload te worden. Broncode en alle vier
firmware-/bootbestanden zijn identiek aan V17. Wifi, API-sleutel en model blijven behouden.
Vanaf V16 of ouder: volg eerst de Atom-update in UPGRADE-V17.md en installeer dan de Casio-app uit V21.

## Het nieuwe logo

Het grote logo heeft een mint-lila kleurverloop en een subtiele dubbele rand,
passend bij de donkere achtergrond. Het beginlogo is iets groter, met ruimte
voor de gecentreerde tekst. Tijdens laden blijven de accenten bewegen.
Ook de kleine logo's in de bovenbalk en bij antwoorden hebben gladdere randen.

Hiervoor worden native rastermaten uit het bestaande SVG gebruikt: 16, 24, 40,
48, 56 en 64 pixels, met 4-bit dekking in plaats van alleen aan/uit-pixels.
De renderer mengt de rand met de aanwezige achtergrond en houdt rekening met
je schermschaal en verschuiving. Het gaat om de echte Casio-renderer, niet alleen een preview.

Het Casio-menu-icoon houdt bewust de bestaande afmetingen, witte basis, zwart
logo en OS-selectie. Er zijn geen OS-, timer-, verbinding- of opslagwijzigingen.
De donkere modus en groene/rode statusbadges blijven; er is geen extra instelling.

## Controle

Veertien ASan/UBSan-testgroepen en de Casio-build, inclusief 40 gesimuleerde
menuheropeningen, statuskleuren, modellen, opslag en de volledige vraagroute.
Screenshots en GIF zijn hostrenders. Er is hier geen fysieke Casio of API-sleutel.
Controleer op jouw apparaat het beginlogo, de kleine logo's, laden, annuleren
en jouw ingestelde schermschaal. Bron en licentie van het logo staan in docs/LOGO-SOURCE.md.

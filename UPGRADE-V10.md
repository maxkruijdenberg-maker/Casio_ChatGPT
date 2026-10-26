# V10 — ALPHA-lock, wiskundetoetsen en automatisch verbinden

**Vervang alleen ChatGPT.g3a als je Atom V3 hebt.** Geen nieuwe Atom-upload of
API-sleutel nodig. Je chats en scherminstellingen blijven behouden. Het door
jou goedgekeurde V9-pictogram blijft hetzelfde.

## Installeren

1. Pak de V10-ZIP uit naar een nieuwe map op Windows.
2. Sluit de Casio via USB aan en kies **F1 USB Flash**.
3. Vervang het bestaande bestand in de hoofdmap door **dist/ChatGPT.g3a**.
   Laat geen tweede '(1)'-kopie naast de oude app staan.
4. Laat de map **CGPT3** en de bestanden daarin staan.
5. Werp het station veilig uit en ontkoppel de Casio-USB-kabel.
6. Open ChatGPT. F6 Instellingen → Versie hoort **10.0 / Atom 3.0** te tonen.

## Nieuwe toetsen in het invoerscherm

| Toets | Functie |
|---|---|
| ALPHA | Lettervergrendeling aan; opnieuw ALPHA zet die uit |
| F5 | Wisselt ABC/abc; de eerste gewone letter van een nieuwe vraag is standaard hoofdletter |
| SHIFT | Activeert de gele functie voor één toets; opnieuw SHIFT annuleert |
| sin / cos / tan | `sin(` / `cos(` / `tan(` in cijferstand |
| SHIFT + sin / cos / tan | `asin(` / `acos(` / `atan(`, oftewel sin⁻¹/cos⁻¹/tan⁻¹ |
| log / ln | `log(` / `ln(` |
| SHIFT + log / ln | `10^(` / `e^(` |
| x² / ^ | `^2` / `^(` |
| SHIFT + x² / ^ | `sqrt(` / `root(` |
| a b/c | `/` voor een breuk |
| ×10ˣ | `*10^(` |
| SHIFT + ×10ˣ / punt | `π` / `=` |
| SHIFT + + / − / × / ÷ | `[` / `]` / `{` / `}` |
| SHIFT + (−) | `Ans` als tekstnaam |
| SHIFT + 1 / 2 | `List ` / `Mat ` als tekstprefix |
| F4 | Overige tekens; EXE voegt een teken in |

Bij ALPHA-lock volgen alle 26 letters de rode labels. **B zit op log**;
zonder ALPHA typ je daar `log(`. Met ALPHA typ je B/b. F5 wisselt de letterkast.
SHIFT verandert de lettervergrendeling niet: SHIFT+sin voegt eenmalig `asin(`
in, waarna de ALPHA-stand weer geldt. Om gewone sin/cos/tan te typen zet je
ALPHA uit. De UI toont ALPHA-lock en een S als SHIFT klaarstaat.

Functies worden als leesbare tekst voor ChatGPT ingevoerd, niet als native
Casio-2D-vakken en niet lokaal uitgerekend. Sluit haakjes met de )-toets.
`root(` kun je bijvoorbeeld gebruiken als `root(3,8)` voor een derdemachtswortel.
`Ans`, List en Mat halen geen waarden uit RUN-MAT op. Speciale Casio-menu's
zoals S⇔D-conversie of de native VARS-lijst zijn geen onderdeel van de editor.

In de antwoordweergave behoudt SHIFT zijn bestaande functie: de hele vraag
bekijken. De bovenstaande wijziging gaat over invoervelden. API-sleutel en
wachtwoord houden exact de ingevoerde letterkast; gebruik daar F5 voor ABC/abc.

## Automatisch verbinding met de Atom

Bij iedere start of hervatting opent de app de UART en stuurt hij een gratis
**W_HELLO**. Je hoeft daarvoor niet meer op F3 Test te drukken. De status gaat
van **Atom...** naar **Atom OK** zodra de Atom antwoordt.

Dit is niet-blokkerend: je kunt intussen letters typen. Bij geen antwoord wordt
na 600 ms Offline gemeld. Tijdens wachten op toetsen in het invoerscherm
probeert de app het ongeveer iedere 3 seconden opnieuw. Met een online Atom
volgt iedere 15 seconden een gratis controle. De probe leest geen toetsen,
verstuurd geen vraag en start geen wifi-/OpenAI-aanvraag. F3 blijft beschikbaar
voor een uitgebreide handmatige UART-diagnose.

Als je een vraag verstuurt of instellingen gebruikt, krijgt de gewone RPC-
afhandeling voorrang en stopt de achtergrondprobe. Achterstallige HELLO-antwoorden
mogen niet als antwoord op een andere opdracht worden gebruikt. Je hoeft de
API-sleutel niet opnieuw in te voeren. De Atom moet wel via USB gevoed blijven.

## Flits van het afsluitvenster

De V9-menuovergang werkt volgens jouw bevestiging. Bij terugkeer herstelt
Casio echter eerst het scherm dat vóór MENU bewaard was: het Ja/Nee-dialoog.
V10 tekent daarom vlak vóór de native menu-aanroep het gewone lege
invoerscherm. Dezelfde renderer wordt gebruikt als bij normale invoer.
Casio bewaart zo geen oud afsluitdialoog meer. De native menufunctie, timer-
fallback en de werkende donkere schermranden blijven.

## Controle op jouw Casio

1. Controleer versie 10.0 en open ChatGPT zonder op F3 te drukken. Met gevoede
   Atom hoort **Atom OK** te verschijnen.
2. Druk ALPHA en typ via de rode labels bijvoorbeeld `Abc`. Test ook B op log.
   Druk nogmaals ALPHA en test sin, cos, tan, log en ln.
3. Test SHIFT+sin, SHIFT+x² en SHIFT+×10ˣ; op het scherm verschijnen `asin(`,
   `sqrt(` en `π`. SHIFT hoort voor één toets te werken.
4. Sluit via EXIT → Ja en open opnieuw. Er hoort geen oud Ja/Nee-scherm voorbij
   te komen; verbinding met de Atom moet vanzelf terugkomen.
5. Test dit vijfmaal, ook met RUN-MAT ertussen. Controleer de opgeslagen chats.
6. Je kunt de Atom tijdelijk van USB halen: typen moet mogelijk blijven. Na
   opnieuw voeden hoort de verbinding in het invoerscherm vanzelf te herstellen.

## Verificatie en beperking

Alle tien ASan/UBSan-softwaretestgroepen slagen. Tests dekken alle 26 letters,
B op log, F5-case, primaire en eenmalige SHIFT-functies, UTF-8, volledige token-
invoer of geen wijziging bij een vol veld, exact gemaskeerde sleutelcase en
cursorbewerking. Met een vertraagd HELLO-antwoord blijft getypte tekst intact.
Offline retries, herstel en voorrang voor een gewone RPC zijn getest, zonder
nieuwe API-opdrachten.

De zes gesimuleerde native/fallback-menuhervattingen gebruiken geen F3: iedere
sessie stuurt zelf HELLO en wordt online. De vóór MENU bewaarde framebuffer
wordt volledig vergeleken met het gewone invoerscherm, zodat een bewaard
bevestigingsscherm de test laat falen. De daadwerkelijke .g3a is opnieuw gebouwd;
Atom-broncode en binaries blijven gelijk aan V3.

**V10 is hier niet op een fysieke Casio uitgevoerd.** De oorspronkelijke
V9-menuovergang is door jou bevestigd; de nieuwe toetsen, automatische verbinding
en de uiteindelijke afwezigheid van een schermflits vragen bovenstaande controle.

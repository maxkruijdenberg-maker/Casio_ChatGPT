# V6: bronnen en afbakening

## Menuoverdracht

libfxcg declareert DisplayMainMenu als directe OS-functie (syscall 0x1E6A):
https://github.com/Jonimoose/libfxcg/blob/daa76d9e37758ab5b9b0f614b7e0aa6acfbe2e18/libfxcg/syscalls/DisplayMainMenu.S

De keyboard-header documenteert de menu-terugkeervlag en biedt een expliciete
enable-functie. V6 gebruikt die vóór de directe menu-aanroep:
https://github.com/Jonimoose/libfxcg/blob/daa76d9e37758ab5b9b0f614b7e0aa6acfbe2e18/include/fxcg/keyboard.h

De system-header beschrijft SetQuitHandler als callback bij het wisselen van
app vanuit het hoofdmenu. Daarom blijft UART open bij de menu-aanroep, en
zit sluiten in deze callback plus een idempotente fallback na terugkeer:
https://github.com/Jonimoose/libfxcg/blob/daa76d9e37758ab5b9b0f614b7e0aa6acfbe2e18/include/fxcg/system.h

Dit bewijst de beschikbare SDK-interface, niet de uitvoering op een echte
fx-CG50 met jouw OS. De eerdere V5-hoststub veranderde een geïnjecteerde MENU
automatisch in een menuovergang. V6 doet dat niet meer. Het echte OS kan hier
niet worden uitgevoerd: de onderzochte emulator vereist een eigen flashdump,
die niet aanwezig is. Er is geen fysieke-emulatietest geclaimd.

## Vergelijking geselecteerd pictogram

Een afbeelding van het echte OS-menu in CG50 Studio toont een lichtblauwe
selectie rond het gehele icon-/naamblok:
https://github.com/hexbinoct/casio-cg50/blob/b321ae95308a172d94ac3742b2a5e6c59e937744/docs/images/cg50-studio.jpeg

Het oude Utilities-voorbeeld in /tmp/casio-utilities heeft een grijze,
afgeronde selectie en is daarom geen geschikte fx-CG50-stijlreferentie voor
deze vraag. Het NOTES-add-in-voorbeeld heeft ook een eigen selectieontwerp;
dat is evenmin een standaard Casio-selectie.

V6 maakt een eigen bitmap: dezelfde 92×64-afmetingen, een rechte 3-pixelrand
in lichtblauw RGB(82,198,247), een wit interieur en het bestaande zwarte logo.
Het logo is niet vergroot. Er zijn geen pixels uit de screenshot gekopieerd.
De kleur is een visuele benadering op basis van een gecomprimeerde afbeelding;
pixelidentieke overeenstemming met alle Casio-OS-versies is niet vastgesteld.
Naam en systeemfocus blijven door Casio getekend. Zie tools/make_assets.py.

# V8: native MENU-afhandeling en beperkingen van hosttests

## Aanleiding

V6 liet de app volgens de gebruiker na de menu-aanroep opnieuw tekenen.
V7 verwijderde de lus na DisplayMainMenu en liet main terugkeren. Dat liet
het menu staan, maar opnieuw openen gaf een kort laadscherm en terugval.
De V7-hosttest riep main drie keer aan en modelleerde daarmee geen bewaarde
OS-continuatie. Die test kan dit werkelijke apparaatgedrag niet uitsluiten.
De startcode blijkt gewoon aanwezig in het V7-linkerresultaat (crt0.o op
0x00300000); het probleem is niet aangetoond als ontbrekende CRT-startcode.

## Nieuwe route

V8 gebruikt geen directe DisplayMainMenu en keert niet uit main terug.
De app laat GetKey de normale MENU-toets afhandelen. Daarvoor worden expliciet
menu-terugkeer ingeschakeld en een matrixevent (4,9,0) aangeboden via
Keyboard_PutKeycode. Dit verschilt van V4/V5, die een reeds vertaald
KEY_CTRL_MENU met (-1,-1) aanboden; dat kan een gewone retourwaarde zijn.
De voortzetting van de appsessie zit achter de native GetKey-afhandeling.

De editor leest toetsen met PRGM_GetKey. Aan de OS-grenzen worden na een
stabiele toetsen-los-periode maximaal 32 oude OS-keyevents geconsumeerd met
GetKeyWait_OS in niet-blokkerende modus, met MENU-afhandeling uitgeschakeld
voor die oude events (menu=1). Pas daarna wordt de echte menu-matrixtoets
geïnjecteerd. Zo worden oude editor-/bevestigingstoetsen niet als menukeuze
hergebruikt. SHIFT-lock in de eigen editor blijft ongewijzigd.

De UART blijft open tot de OS-afsluitcallback of de terugkeer uit GetKey.
Daarna worden sessiestatus, toetsstatus, parser en schermpointer opnieuw
geïnitialiseerd. De callback en het opruimen na terugkeer zijn idempotent.

## Gebruikte interfaces en vergelijking

SDK keyboard-header: GetKey, GetKeyWait_OS, matrix-/keycode-interface en
EnableGetkeyToMainFunctionReturn:
https://github.com/Jonimoose/libfxcg/blob/daa76d9e37758ab5b9b0f614b7e0aa6acfbe2e18/include/fxcg/keyboard.h

De matrixpositie MENU = kolom 4, rij 9 wordt ook gebruikt door de tijdgestuurde
key-poller van dit fx-CG50-project; dit is een vergelijking, geen OS-specificatie:
https://github.com/james8464/notes-addin-casio-fx-cg50/blob/bcf9a6183c312bfa9ceac0407590bc5e4c1dab18/source/apps/notes/notes_app.cc

Quit-handlercontract:
https://github.com/Jonimoose/libfxcg/blob/daa76d9e37758ab5b9b0f614b7e0aa6acfbe2e18/include/fxcg/system.h

Het native fx-CG50-menuscherm gebruikt een geselecteerd blok met een gekleurd
logo-paneel boven de appnaam. V8 geeft het eigen logo een wit paneel binnen
de blauwe selectie. Afbeelding ter vergelijking:
https://github.com/hexbinoct/casio-cg50/blob/b321ae95308a172d94ac3742b2a5e6c59e937744/docs/images/cg50-studio.jpeg

## Wat de tests wel en niet aantonen

De hoststub controleert het gebruikte matrixevent, het leegmaken van oude
events, menu-enable, behoud van UART tot menu-entry en drie hervattingen.
De stub geeft niet automatisch elke GetKeyWait-poll of een vertaald MENU-code
het effect van een echte menuovergang. DisplayMainMenu wordt niet gebruikt.
Deze controles bewijzen de applicatievolgorde onder dit SDK-contract.

Er is geen fysieke Casio of eigen OS-flashdump beschikbaar. Daarom is niet
vastgesteld dat elke aanname over native MENU op dit apparaat klopt. De
screenshot is een vergelijking, geen uitvoering van ChatGPT in de emulator.
V8 moet fysiek worden gecontroleerd op sluiten én opnieuw openen. De eerdere
V7-regressielog is historische testinformatie en geen bewijs voor OS-gedrag.

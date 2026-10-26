# V9: onderbouwde native menuovergang

## Feiten uit de apparaatmelding

- OS-versie: door gebruiker opgegeven als 0.3.81.0202 (hier geschreven 03.81.0202).
- V8: EXIT → Ja herstelt de witte rand/statusbalk, maar laat de bevestiging staan.
  Ook de fysieke MENU-toets op dat scherm opent volgens de gebruiker geen menu.
- V7: terugkeer uit main liet MENU staan, maar heropenen gaf een kort laadscherm
  en terugval naar MENU. De eerdere hoststubs waren hiervoor geen OS-bewijs.

## Gevonden bestaande implementatie: gint

Bron op vaste commit (spiegel van Lephe's gint):
https://github.com/peltlasvulpine/gint-mirror/blob/2bc6f7ebcb500b423ee9906e11664466ae913424/src/kernel/osmenu.c

API-documentatie met exact de heropenbeperking van een teruggekeerde main:
https://github.com/peltlasvulpine/gint-mirror/blob/2bc6f7ebcb500b423ee9906e11664466ae913424/include/gint/gint.h

Gint legt uit dat Casio een add-in die terugkeert uit main probeert te hervatten,
waarna hij opnieuw afsluit. De bibliotheek roept in plaats daarvan het menu aan
en hervat de context na opnieuw selecteren.

Voor CG biedt gint twee native routes:

1. Haal de syscalltabel op via 0x8002007C. Lees daarin de pointer van syscall
   **0x1E58 SwitchToMainMenu**. Herken binnen 150 instructies het patroon:
   `mov.l @(disp,pc),rX; mov #3,rY; bsr target; mov.b rY,@rX; bra ...; nop`.
   Het branchdoel is **SaveAndOpenMainMenu**. Gint roept dit rechtstreeks aan.
   De bron schrijft de ontdekking toe aan **Dr-Carlos** en verwijst naar:
   https://www.cemetech.net/forum/viewtopic.php?t=18944
   Die forumpagina is hier niet zelfstandig opgehaald; de gelezen bron is gint.
2. Als dit patroon niet herkenbaar is: een OS-timer injecteert `(4,9,0)` en
   stopt/verwijdert zichzelf. De native code wacht in
   `GetKeyWait_OS(&col,&row,0,1,0,&key)`, dus met MENU-afhandeling ingeschakeld.
   Dat verschilt van vooraf injecteren gevolgd door GetKey, de V8-route.

Gint wist eerst de echte keybuffer via syscall **0x12C7**:
https://github.com/peltlasvulpine/gint-mirror/blob/2bc6f7ebcb500b423ee9906e11664466ae913424/src/kernel/syscalls.S

De README staat gebruik, wijzigen en delen voor ieder doel toe:
https://github.com/peltlasvulpine/gint-mirror/blob/2bc6f7ebcb500b423ee9906e11664466ae913424/README.md
Attributie: **Lephe/Lephenixnoir, gint**, en **Dr-Carlos** voor de native functieherkenning.

## Aanpassing in deze app

Onze libfxcg-app draait al onder het native OS; er is geen gint-driverwereld
te herstellen en er is geen tweede framebuffer om naar OS-VRAM te kopiëren.
We nemen de native aanpak over, niet de hele gint-unikernel.

- `casio/native_menu.h`: begrensde, alleen-lezen patroonherkenning, met guards
  voor tabel/entry/branchdoel, uitlijning, 150-instructielimiet, getekende
  12-bit BSR-displacement en afwijzing van verschillende passende doelen.
- `casio/native_menu_device.h`: directe ROM-reads en aanroep van het gevonden
  doel. Geen hardgecodeerd functieadres voor één OS-versie; alleen de bestaande
  syscall-interface en het herkende codepatroon worden gebruikt.
- `casio/native_syscalls.S`: de gewone syscall-stub voor ClearKeyBuffer 0x12C7.
- Timerfallback: 20 ms in plaats van gint's 0 ms; het scherm wordt voorbereid
  vóór Timer_Start, waarna meteen GetKeyWait_OS wordt aangeroepen. Zo krijgt
  de key-wait gelegenheid actief te worden vóór de timerinjectie.
- De timer wordt altijd gestopt/verwijderd, ook bij fouten en in de quit-handler.
  Een mislukte start houdt de UART en de appsessie intact en toont een melding.
- Bij terugkeer uit de native functie/key-wait start een nieuwe lokale UI-sessie;
  main retourneert niet. De OS-quitcallback of opruimen na terugkeer sluit UART.
- F6 → Versie meldt of de native functie is gevonden of de timerroute nodig is.

Dit leest ROM maar past geen OS-code, mapping, RAM-vlag of firmwarebestand aan.

## Pictogramsjabloon en J

Vergelijking met het echte CG50-menu:
https://github.com/hexbinoct/casio-cg50/blob/b321ae95308a172d94ac3742b2a5e6c59e937744/docs/images/cg50-studio.jpeg

Aanvullend onderzocht: Lephe's CG50-template (README en XCF-lagen) in:
https://github.com/Fadouse/khicas-opt/blob/ebcbfa3c5f497ef5f278dbda4c1eee7b939be193/tools/fxcg50-icon-template.zip
De XCF-laagposities geven een **82×38-paneel op (5,5)**. Het bijbehorende
README bevestigt dat Casio de G3A-naam zelf onderin tekent. We genereren onze
eigen witte, afgeronde variant op deze geometrie, met een zwart 30px-logo,
blauwe selectie daarbuiten en de door de gebruiker gevraagde compacte J-badge.
Er zijn geen pixels uit de menuscreenshot of XCF naar de app gekopieerd.

De KhiCAS-toolnotities adviseren zelf geen sneltoetsbadge te tekenen omdat
het OS de positie bepaalt. Hier is J wel toegevoegd op expliciet verzoek;
het is een visuele badge en stelt de eigenlijke Casio-sneltoets niet in.
De bitmap bevat geen appnaam, zodat OS-fontmaat en naam niet dubbel worden.
Afgeronde hoeken/kleuren/badge zijn Casio-stijl; exacte pixelgelijkheid op de
fysieke calculator is niet bewezen.

## Validatie en open beperking

Tien ASan/UBSan-hosttestgroepen slagen. De pure resolver tests dekt ook alle
SH-registerparen en ongeldige/dubbelzinnige patronen. De echte appcontroller
gebruikt in hoststubs een nagebootst ROM met dat patroon en doorloopt drie
native en drie fallback-hervattingen. De fallbackstub eist dat de timer pas
binnen GetKeyWait_OS injecteert; vooraf injecteren zoals V8 laat de test falen.
Timerfouten en onderbroken wachten laten geen timer achter en sluiten UART niet.

De SH4-build bevat de nieuwe ClearKeyBuffer-stub, Timer-functies en correcte
startcode. Het nieuwe Linux Casio-buildscript en het Windows-script nemen ook
het assemblybestand mee. Windows is hier niet uitgevoerd.

**Er is nog geen daadwerkelijke OS 03.81.0202-code of fysieke calculator hier
uitgevoerd.** Het ROM-patroon en de timerroute komen uit bestaande gint-code;
het host-ROM is synthetisch. Daarom bewijzen de tests de adapterlogica, niet
dat deze OS-versie het patroon bevat of het menu op het apparaat correct toont.
De emulator vereist een eigen flashdump die hier niet beschikbaar is. De
hardwarecontrole in UPGRADE-V9.md blijft nodig.

# Gebruikte versies en bronnen

- Arduino CLI 1.3.1 Linux x86_64. Officiële release plus SHA-256-controle:
  https://github.com/arduino/arduino-cli/releases/tag/v1.3.1
- ESP32 Arduino core 3.3.0, board-ID `esp32:esp32:m5stack_atom`, boardnaam
  `M5Atom`, originele ESP32. Officiële core en tool-assets gedownload via de
  Espressif-index, alle SHA-256-waarden gecontroleerd.
  https://github.com/espressif/arduino-esp32/releases/tag/3.3.0
- esp-x32 2411, xtensa-esp-elf GCC 14.2.0_20241119.
- esp32-arduino-libs idf-release_v5.5-b66b5448-v1.
- esptool 5.0.0. Arduino-ctags 5.8-arduino11, SHA-512 gecontroleerd.
- ArduinoJson 7.4.2, commit 733bc4ee82630c88c0a619a883cd3a206efae977:
  https://github.com/bblanchon/ArduinoJson/tree/v7.4.2
- Casio: officiële PrizmSDK 0.6-headers, libfxcg/libc en big-endian libgcc uit
  PrizmSDK-win-0.6.zip; SDK-compiler voor Windows is GCC 10.1.0.
  https://github.com/Jonimoose/libfxcg/releases/tag/v0.6
- Linux-compilatie van Casio-main.c: Debian gcc-14-sh4-linux-gnu
  14.2.0-19cross1 en binutils-sh4-linux-gnu 2.44-3. Gedownload via door APT
  geverifieerde Debian Trixie-indexen. Freestanding, SH4A no-FPU, big-endian,
  Hitachi-conventie, leading underscores; daarna gelinkt met SDK 0.6-libraries.
  De binary heeft de originele SDK-startup op adres 0x00300000; linkerkaart
  inbegrepen. Voor de definitieve test is echt Casio-hardware nodig.
- mkg3a git 502381ee402d8435126be3ac651ad9b42db26d33 (GitLab taricorp/mkg3a).
  Gebouwd met BMP-ondersteuning; eigen iconen 92x64 pixels.
  https://gitlab.com/taricorp/mkg3a
- DejaVu Sans Mono 13px-raster, 177 expliciete glyphs (V2); generator inbegrepen.
  Zie FONT-LICENSE.txt. Geen automatische aanname over Casio-codepagina's.
- OpenAI API-specificatie opgehaald van de officiële repository tijdens bouw:
  https://github.com/openai/openai-openapi/blob/master/openapi.yaml
  API-referentie: https://platform.openai.com/docs/api-reference/chat/create
  Modelnaam standaard `gpt-4.1-mini`, wijzigbaar. Beschikbaarheid voor een
  concreet account is niet getest: de Atom doet zelf een geauthenticeerde
  modelcontrole voordat een betaalde completion start.
- TLS-ankers: originele Mozilla-certificaten uit Debian ca-certificates,
  zonder uitschakeling van verificatie of vervanging van checksums.

Bron- en licentievermeldingen van libfxcg, mkg3a, ArduinoJson en het lettertype
staan in deze map. De ESP32-core en vooraf gebouwde Espressif-bibliotheken
behouden hun upstreamlicenties: https://github.com/espressif/arduino-esp32 en
https://github.com/espressif/esp-idf . De volledige toepassing en de benodigde
Arduino-bronbestanden zijn meegeleverd zodat je de firmware opnieuw kunt bouwen.

`dist/sha256.json` bevat hashes van de geleverde binaire bestanden. Een hash
bevestigt bestandsgelijkheid; hij bewijst geen fysieke werking.

## V2.0 changes

Runtime toolchain versions are unchanged. Firmware and add-in report V2.0;
wire framing remains protocol v1 with additive W_GET_CONFIG/W_DEVICE commands.
The shared Unicode helper is in atom/ChatGPTAtom/utf8.h. The font now contains
177 glyphs (middle dot added); tools/make_font.py no longer overwrites the
menu icons. tools/make_assets.py builds those from assets/openai.svg using
CairoSVG and Pillow on the build machine. Their versions are recorded in
`docs/logs/v2-assets.txt`; neither is needed to flash prebuilt files.

The C++ host API tests use the same ArduinoJson 7.4.2 headers as the firmware.
GCC emits a may-be-uninitialized warning for an upstream ArduinoJson iterator
in the sanitized host build; tests finish successfully without sanitizer
errors. The ESP32 target build also completes successfully.

## V3.0 changes

Toolchain versions and the original Atom pinout remain unchanged. Both programs
report 3.0. Wire framing is still v1; W_CONTEXT and upload targets 5/6 restore
recent local history without starting HTTP or consuming a paid request ID.
The tested ChatContext template is shared by firmware and host tests.

Casio persistence uses the existing SDK Bfile syscalls and a streamed dual-bank
CRC32 format; a complete conversation is never buffered in application RAM.
Only ChatGPT chats are included. The native OS renders menu names; the two
92x64 icon bitmaps now have a white background, black logo and no baked-in text.
Display refresh explicitly covers rows 0..215 with status-area mode 3.
These OS/device behaviors still require physical validation.

W_DEVICE also reports a non-secret boot identifier. The add-in checks it
before sending a new question and reloads recent stored context if the Atom
has restarted. The host UI tests cover both unchanged and changed boot IDs.
The eight-group test run is recorded in docs/logs/tests.log.

## V4.0 Casio-only fixes

Atom source and firmware files are unchanged from v3.0.0. Casio reports 4.0;
Atom correctly reports 3.0. The native SDK GetKey/GetKeyWait path handles color
initialization and menu switching. Each entry/resume has an explicit session
reset and held-key release barrier. Full frames use Bdisp_DefineDMARange
(0,383,0,215) and Bdisp_PutDisp_DD instead of Bdisp_PutDisp_DD_stripe.

The renderer supports bounded 80–120% zoom and ±24px pan. Display preferences
use D0A.dat/D0B.dat in CGPT3; chat records are unchanged. The viewport geometry,
preference storage and full app lifecycle are covered by sanitized host tests,
including an LCD model and changed VRAM pointers. Physical OS/LCD confirmation
is still required. See UPGRADE-V4.md and docs/logs/tests.log.

## V5.0 Casio-only fixes after device feedback

V4 closed UART but did not leave the add-in on the user's calculator. Replace
the matrix-level GetKeyWait_OS handoff with the full GetKey dispatcher for
the translated MENU queue. Keep the explicit session reset on resume. Host
stubs now reject the old handoff rather than assuming every poll opens MENU.

DrawFrame(C_BG) fills the OS-managed outer frame separately from VRAM; cleanup
restores white. This covers the border that viewport zoom cannot reach.
The selected menu bitmap now supplies a complete raised neutral tile; neither
bitmap contains baked-in label text. The OS controls its text/focus colors.

Atom source/binaries remain identical to v3.0.0. Nine sanitized test groups
pass and the SH4 add-in is built. Native menu behavior, border rendering and
selection style require physical confirmation. See UPGRADE-V5.md.

## V6.0: direct native menu and deferred UART cleanup

V5 still did not leave the app on the user's device; its host dispatcher stub
was not evidence of an OS menu transition. V6 removes injected MENU entirely
and calls EnableGetkeyToMainFunctionReturn and DisplayMainMenu directly.
UART stays open until the OS app-switch callback (or cleanup after native
menu return). No pre-menu W_CANCEL or Serial_Close. The USB-powered Atom is
not physically switched off. Lifecycle fixtures now exercise real RPC-opened
UART sessions and assert cleanup ordering, callback and callbackless paths.

The V5 working DrawFrame behavior remains. Selected icon: white interior,
rectangular light-blue border, no black shadow/rounding/gradient. See
docs/V6-REFERENCES.md for the fx-CG50 menu screenshot used for comparison.
Nine sanitized host test groups pass; SH4 binary built. Physical native-menu
behavior and exact OS selection appearance remain unverified. Atom stays V3.

## V7.0: return to OS instead of internally reopening

Device feedback: V6 briefly restored the white frame, then rendered ChatGPT
again. V6's main loop treated any DisplayMainMenu return as an add-in relaunch.
Remove that loop and return from main to the OS caller, matching V1's exit
structure. Keep per-launch resets and UART cleanup after the menu call.

Lifecycle fixtures now call main three separate times. DisplayMainMenu returns
immediately; tests assert no further app framebuffer transfer and a normal
main return. Running these fixtures against V6 fails (editor re-entry without
new OS launch); see docs/logs/v7-menu-regression.log. All nine sanitized test
groups pass with V7; actual SH4 binary built. Device confirmation still needed.

Selected icon is fully light blue, rather than blue outline on white. Shrink
menu logo from 44px to 32px, keep it in the upper area and leave the bottom
for OS-drawn app name at native font size. No bitmap label. Preserve DrawFrame
and Atom V3 unchanged. See UPGRADE-V7.md.

## V8.0: native MENU dispatch and white logo panel

User confirmed V7 exits but re-opening shows loading then MENU. CRT startup
was present; returning main was insufficient. Use GetKey with a real matrix
MENU (4,9,0), explicit menu enable, and bounded nonblocking stale OS-event
draining at session boundaries. Remove direct DisplayMainMenu. Remain in the
app's main continuation, and reset sessions only after native GetKey returns.
UART deferred cleanup, dark outer frame and Atom V3 are retained.

Selected icon: white rectangle behind black 32px logo, blue outside including
the native label region. Same 92x64 dimensions; no baked label. Nine sanitized
host groups pass and SH4 add-in is built. Native dispatcher execution is
unverified on hardware; see docs/V8-NATIVE-MENU.md and UPGRADE-V8.md.

## V9.0: native menu resolver and proven timer fallback structure

User OS 03.81.0202: V8 stays on a white-rimmed confirmation even on physical
MENU. Investigated gint's native osmenu.c and its documented main-return
re-entry limitation. Use its SaveAndOpenMainMenu discovery from syscall
0x1E58 (bounded, aligned, signed-branch checked, no OS writes), or its timer +
GetKeyWait_OS(0,1,0) structure. Add the real ClearKeyBuffer syscall 0x12C7.
The timer injects during the key-wait and frees itself; failure paths restore
the app and keep UART. Version dialog reports native target found / OS timer.

Icon panel follows Lephe template geometry (5,5,82,38), rounded white with
black logo lowered and requested J badge. Native name and dark screen frame
retained. Ten sanitized host test groups pass, including resolver and six
menu sessions; actual SH4 add-in built. The actual 03.81.0202 OS has not been
executed here. See docs/V9-NATIVE-MENU.md and UPGRADE-V9.md. Atom stays V3.

## V10.0: ALPHA lock, one-shot math SHIFT, async UART and clean menu snapshot

User confirmed V9 native MENU works on 03.81.0202, but re-entry briefly restored
the closing dialog and needed F3 to reconnect. Paint the ordinary blank editor
through the shared renderer before native MENU saves VRAM. Preserve the working
V9 native helper and icon assets. Six-session tests compare the saved framebuffer
against the normal startup editor, with no manual F3 in any session.

ALPHA toggles persistent letters; F5 toggles case. SHIFT is one-shot (or cancelled
by pressing again) and inserts yellow-label math text, including inverse trig,
roots, powers and pi. Primary trig/log/powers work in numeric mode. All red-label
letters including B/log are exercised. Multi-character function tokens insert
atomically under byte limits and UTF-8 cursor boundaries.

Startup and idle input use a nonblocking W_HELLO-only UART probe. 600ms response
window, offline retry 3s, online heartbeat 15s. It never reads keys, allocates an
API request or sends question/config text. Foreground RPC preempts the probe.
Delayed response typing, offline recovery and stale-reply preemption are tested.
Ten sanitized host groups pass, SH4 add-in built, Atom remains byte-identical V3.
New V10 behavior still needs hardware confirmation; see UPGRADE-V10.md.

## V11.0

- Tijdelijke WPA2-wifi-installatie met willekeurig wachtwoord op de Casio,
  Nederlandse telefoonpagina, lokale gezamenlijke NVS-configuratiecommit,
  CSRF-token, timeout, status en geen betaalde setup-aanvragen.
- Atom-firmware nu V11, Casio V11; bestaande instellingen migreren zonder wissen.
- Geen add-in-quitcallback meer bij OS-dispatch; UART en bestanden vóór menu
  sluiten. Aanpassing naar aanleiding van ADDRESS(W) TARGET=00000001 PC=8C09CA4.
- Bestaande iconen, menu-resolver, bediening, chats en schermgeometrie behouden.
- Elf softwaretestgroepen; hardwareherstel en telefoonnetwerk nog fysiek testen.


## V11.0.1 — menuafbeeldingen

Beide menutegels wit; zichtbare basis- en taalnamen bevatten alleen een spatie.
Interne identiteit @CHATGPT behouden. Uitvoerbare Casio-code en alle Atom-binaries
identiek aan v11.0.0. OS-overlays en werkelijk menugedrag nog op hardware testen.


## V12.0 — automatisch seriële verbinding herstellen

- Normaal menu-icoon en naam van de oorspronkelijke V11 terug.
- V11-fout opgelost die een reeds geopende OS-poort definitief afwees;
  controleer ook een extern gesloten poort bij lokaal geopende vlag.
- Poort opnieuw initialiseren na schrijffout of heartbeat-timeout. Achtergrond-
  retry elke 750 ms offline / controle elke 10 s online; 500 ms heartbeat-timeout.
- Automatisch herstel binnen Instellingen; read-only metadata maximaal 2x300 ms.
- Belangrijke opdrachten behouden 4x800 ms en identieke requestbytes/sequence.
- Atom blijft V11, bron en binaries ongewijzigd. Geen extra betaald API-verkeer.
- Uitgebreide UART-mocks en regressietests, elf groepen; fysieke verificatie nog nodig.


## V13.0 — toetsen en blijvende OS-UART

- Read-only SH7305 KEYSC-polling vervangt PRGM_GetKey_OS in app-bediening.
- OS-poort blijft open door MENU en herselectie; geen eigen callback/ISR achtergelaten.
- Achtergrond-RX verwerken vóór het afhandelen van toetsen.
- Drie gemiste controles vóór offline/driver-reset; geen sluiten bij losse timeout.
- Metadata 3x1200 ms, heartbeat 1200 ms; belangrijke opdrachten nog 4x800 ms.
- Atom V11 bron/binaries ongewijzigd. Normale iconen behouden.
- Twaalf testgroepen, 50 fysieke keys, trage ACK tijdens typen, 40 sessies.
- Fysieke oorzaak per-toets-uitval en OS-gedrag buiten app nog niet bewezen.


## V14.0 — verkoopgerichte bediening

- EXIT sluit niet af; alleen MENU opent een globale bevestiging met Blijven.
- F3 en SHIFT+punt voeren = in; eerste symbool in de tekenkiezer is =.
- Eén Diagnose in Instellingen; antwoord F3 toont de vraag, bij opslagfout Bewaar.
- Drie modelpresets met naam, uitleg en indicatieve relatieve kosten; geen vrije invoer.
- Verfijnde donker/mint UI, Hulp, kopersgids en verticale TikTok-interfacepreview.
- Atom V11, V13-keyboard/heartbeat/menu-driver blijven behouden.
- Nieuwe integratietests voor menu/drafts/geheimen/modellen, twaalf groepen en 40 sessies.
- Fysieke V14, API-toegang en actuele prijzen nog niet door deze omgeving geverifieerd.


## V15.0 — eenvoudige kopersinterface en stabieler berichtverkeer

- Verbonden/Verbroken rechtsboven; klantentaal zonder technische diagnosewaarden.
- Vijf instellingenkaarten, met EXE/rechts openen en telefooninstallatie als hoofdroute.
- Geen ruimtemeter of aparte =-knop; F3 opent Chats.
- Display-only antwoordnormalisatie voor =-varianten en veelvoorkomende wiskundetekens.
- Groot geanimeerd laadlogo zonder technische voortgangsteksten.
- Gedeelde RX-parser, TX-backpressure afwachten, cache van openbare instellingen.
- Geen reset bij korte timeouts; herstel bij langdurige stilte met cooldown.
- Achtergrondcontroles op meer schermen, afgeschermd tijdens foreground-RPC/modal.
- Atom V11 en fysieke toetsdecoder/native-menu-route blijven ongewijzigd.
- Dertien testgroepen en 40 menucycli; fysieke V15-duurtest/API-test nog nodig.

## V16.0 — V13-verbinding met opgeruimde interface

- Herstelde V13 UART-retries, aparte heartbeat-parser en drie-miss herstel.
- Geen V15 herstel-cooldown, TX-wachtlus of achtergrondverkeer in ieder dialoogvenster.
- Alleen bij gewijzigde verbindingsstatus het invoer-/instellingenmenu hertekenen.
- V15-instellingen behouden; appversie uitsluitend onder Hulp en versie.
- Pulserend laadlogo zonder voortgangstekst/knoppenbalk; antwoorden met leesbare =.
- Geneste bevestiging wordt hersteld na MENU → Blijven; sessiecache wordt gewist bij heropenen.
- Atom-firmware V11 ongewijzigd; dertien hosttestgroepen en SH4-build.
- Geen fysieke duurtest of betaalde API-test uitgevoerd in deze omgeving.

## V17.0 — lichte koperinterface en accountmodellen

- Lichte stijl met teal en mintselectie, vier hoofdinstellingen.
- Aparte Hulp met taakgerichte uitleg en verbindingscontrole; geavanceerd onder Meer.
- Nieuwe laadkaart met bewegend accent, zonder ontwikkelstatus of versie.
- Nieuwe Atom V17: asynchrone GET /v1/models met TLS, JSON-filter op ID's en begrensde lijst.
- W_MODELS (17): starten/status/ophalen; W_DEVICE major 17, capabilitybit 16.
- Accountgebonden tekst-chatkandidaten; endpointondersteuning is niet in /models beschikbaar.
- Maximaal 384 unieke ID's/32767 bytes; geen stille afkapping of automatische completion-test.
- Modelnamen met dubbele punten toegestaan voor aangepaste modellen; paden blijven ongeldig.
- Config11/opslag/V13-toetsen/native menu blijven; Atom-update deze keer vereist.
- Veertien hosttestgroepen en beide builds; echte hardware en live API nog te testen.

## V18.0 — kleurrijke kaartstijl en gecentreerde teksten

- Gedeelde kaartstijl met zachte schaduw en gekleurde selectierand.
- Teal/blauw/paars/goud voor de vier instellingen, met kleine pictogrammen.
- Gecentreerde tekst onder logo's op start- en laadscherm; aparte zin voor modellen ophalen.
- Consistente kaders rond functieknoppen en kleurrijke accenten in Chats/Hulp.
- Alleen visuele wijzigingen; Atom-broncode en firmware bytegelijk aan V17.
- Casio-build, veertien hosttestgroepen en 40 gesimuleerde menuheropeningen.

## V19.0 — abstract motief en expliciete statuskleuren

- Abstract orbitmotief op begin- en laadscherm, met rustige lila/mintaccenten.
- Minder schaduwen en knopkaders; primaire actie en geselecteerde rij blijven zichtbaar.
- Verbonden: groene tekst/stip/badge. Verbroken: rode tekst/stip/badge.
- Statusbetekenis, transport en bediening ongewijzigd. Atom blijft bytegelijk aan V17.
- Integer-renderer voor abstracte vormen; geen nieuwe timer, ISR of hardwareconfiguratie.
- Veertien softwaretestgroepen met beide statuskleuren en 40 gesimuleerde menucycli.

## V20.0 — donkere uitvoering

- Volledig afgestemde donkere palette: diepblauw, mint, lila en heldere tekst.
- Gekleurde donkere selectiekaarten en groene/rode statusbadges.
- Verfijnde teksthiërarchie en dubbele accentbeweging rond het laadlogo.
- Geen nieuwe bediening of instellingen; Atom bytegelijk aan V17.
- Casio-build en veertien hosttestgroepen met 40 gesimuleerde menucycli.

## V21.0 — verfijnd logo in donkere modus

- Native SVG-rastermaten met 4-bit antialiasing voor grote en kleine logo's.
- Mint-lila kleurverloop en dubbele rand op begin-/laadscherm.
- Schermschaal/bounds-aware RGB565 blending; geen nieuwe timers of hardwareconfiguratie.
- Casio-menu-iconen en volledige Atom-broncode/binaries blijven gelijk.
- Casio-build en veertien hosttestgroepen, inclusief 40 gesimuleerde menucycli.

## V20.1 — symmetrische planeetring en volledige OpenAI-accountlijst

- Eén gecentreerde schuine ellipsring, met render-symmetrietest. Donkere modus blijft.
- “Een vraag is genoeg om te beginnen” verwijderd.
- Alle accountmodel-ID's, zonder familie-/modelfilter; typen zijn alleen UI-uitleg.
- Nieuwe Atom 20.1 (capabilitybit 32); chatkeuze blokkeert bekende niet-chatfamilies met uitleg.
- 26 ingebouwde SDK-keuzes, waaronder GPT-6 Astra/Sol/Terra/Luna; vaste officiële bronrevisie vastgelegd.
- Lijst maximaal 1024 IDs/65535 bytes, modelnaam/config/upload maximaal 255 tekens.
- Casio-lijstbuffer op OS-heap, vrijgeven vóór MENU; SDK-RAM-regio ongewijzigd.
- Beide builds, veertien ASan/UBSan-groepen en 40 gesimuleerde menuheropeningen.

## V21.1 — chatbalk en klantpakket

- Normale chatbalk met niet-verstuurde placeholder; groeit/krimpt tot acht zichtbare regels.
- Cursor/scrollpositie herstelt correct wanneer een lang concept wordt ingekort.
- Rond groen/rood statusbolletje; hoogtes en marges afgestemd.
- PDF en offline HTML uit één handleidingbron; aparte fysieke vrijgavelijst voor de verkoper.
- Klant-ZIP met alleen actuele bestanden en licenties; geen oude versiegidsen of ontwikkellogs.
- Casio-build en veertien softwaretestgroepen; Atom bytegelijk aan V20.1.
- Fysieke montage, duurtest en live API-toegang niet gecertificeerd vanuit deze omgeving.


## V22 — 29 oktober 2026

Actuele onderhouden chatmodellen met gewone namen, gefilterd op accountmetadata,
F4 Info met keuzehulp, F2 Vernieuw en veilige lege lijsten. Atom V20.1 blijft
ongewijzigd. Zie V22-MODELLEN.md en V22-CONTROLE-VOOR-VERKOOP.md. Geen live
API-toegang of fysieke Casio/Atom in deze bouwomgeving getest.


## V22.1 — 29 oktober 2026

Directe vaste modelkeuzes, expliciete optionele F2 Account-controle, geen
misleidende fallback na laadfout. Oude/gedeeltelijke accountgegevens worden
niet als succes getoond. Paarsblauwe planeet en compactere laad-/modelschermen.
Atom, native menu, UART, toetsen en opslag ongewijzigd. Controleer de fout- en
succesroute op echt apparaat volgens V22.1-CONTROLE-VOOR-VERKOOP.md.


## V22.2 — 29 oktober 2026

Atom-firmware en Casio-update nodig. Accountmetadata via gedecodeerde HTTP-body
in plaats van raw getStream; gefilterde JSON en begrensd ontvangen. Accountspecifieke
foutteksten. Dubbele Vraag-knop verwijderd, Bewaar bij opslagfout behouden. Geen
wijziging betaalde completion-functie, tokenlimiet, ledger, UART of native menu.
De broncode van ESP32 HTTPClient 3.3.0 bevestigt dat writeToStream chunked encoding
verwerkt. De echte foutrespons van de gebruiker en een live API-account waren
niet beschikbaar; succes op echt apparaat moet na beide updates worden gecontroleerd.


## V22.3 — geheugenarme accountcontrole

Alleen elf metadata-GETs, geen responsebody/JSON/grote accountlijst in RAM.
256-byte resultaat, Casio-buffer 4 KiB, nulresultaat expliciet gecontroleerd,
fouten wissen de hele lijst. Atom V22.3 en Casio-app vereist; betaald verzoek,
UART, opslag en native menu ongewijzigd. Test F2 op het echte apparaat na beide updates.

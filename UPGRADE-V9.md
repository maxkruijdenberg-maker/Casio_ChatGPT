> Nieuwste versie: [V10 installeren](UPGRADE-V10.md).

# V9 — native OS-menu en pictogram volgens de CG50-template

**Vervang alleen ChatGPT.g3a als je Atom V3 hebt.** Geen nieuwe Atom-upload,
API-sleutel of schermkalibratie nodig. Chats en instellingen blijven.

## Installeren op Windows

1. Download de V9-ZIP en pak die uit naar een nieuwe map.
2. Sluit de Casio via USB aan en kies **F1 USB Flash**.
3. Vervang in de hoofdmap het bestaande bestand door **dist/ChatGPT.g3a**.
   Laat geen tweede '(1)'-kopie naast de oude app staan.
4. Laat **CGPT3** en de bestanden daarin staan.
5. Werp het station veilig uit en ontkoppel de Casio-USB-kabel.
6. Open ChatGPT en laat EXE los. F6 Instellingen → Versie moet
   **9.0 / Atom 3.0** tonen. Het versiedialoog toont ook de menu-route:
   **native functie gevonden** of **OS-timer**.

## Wat is nu anders aan afsluiten?

V8 bleef volgens jouw melding ook met de fysieke MENU-toets op het
bevestigingsscherm staan. Alleen een witte rand terugzetten is geen menu.
V9 gebruikt daarom de native menu-aanpak van de bestaande bibliotheek gint:

- Eerst de echte OS-functie **SaveAndOpenMainMenu** zoeken via het herkenbare
  codepatroon in de OS-menuwrapper. De lookup leest alleen ROM, heeft grenzen
  en schrijft geen OS-code of globale OS-adressen over.
- De gevonden functie bewaart de appcontext, opent MENU en keert terug als je
  dezelfde app weer selecteert. Daarna initialiseert ChatGPT zijn sessie opnieuw.
- Bij een onbekend patroon wordt de andere gint-route gebruikt: een OS-timer
  biedt MENU aan terwijl **GetKeyWait_OS** actief is. V8 zette de toets vooraf
  klaar en gebruikte GetKey; dat is een andere volgorde en andere aanroep.
- De echte native keybuffer wordt nu via syscall 0x12C7 gewist bij de grenzen.
- Er wordt niet uit main teruggekeerd. Gint documenteert expliciet dat Casio
  een teruggekeerde app probeert te hervatten en die dan opnieuw afsluit,
  precies het soort heropenprobleem dat je voor V7 meldde.

De UART sluit niet vóór de menu-aanroep; dat gebeurt bij de OS-appwissel of
na terugkeer uit het native menu. De USB-gevoede Atom blijft fysiek aan.
Bij fouten zoals geen vrije OS-timer wordt een melding in de normale app
getoond, met behoud van de verbinding en zonder achtergebleven timer.

## Pictogram

Het beeldvlak is verplaatst naar **(5,5), 82×38 pixels**, de geometrie uit
Lephe's CG50-pictogramsjabloon. Het witte vlak heeft afgeronde hoeken; het
zwarte logo staat lager. De gevraagde **J** staat als compacte badge rechtsboven.
Bij selectie blijft het logo op wit, met blauw eromheen en achter de appnaam.
Casio tekent de naam ChatGPT zelf op de normale lettergrootte, zonder dubbele
bitmaptekst. De donkere schermranden in de geopende app blijven behouden.

De J is op jouw verzoek getekend voor je huidige menupositie. De echte
sneltoets en positie worden door Casio bepaald; de badge verplaatst de app niet.
Het ontwerp gebruikt de standaard geometrie, maar pixelidentieke weergave
op jouw echte OS-menu is hier niet vastgesteld.

## Controle op jouw Casio (OS 03.81.0202)

1. Controleer eerst versie **9.0**. Lees in het versiedialoog de **menu-route**.
2. Test F3 met de aangesloten Atom en keer terug met EXIT.
3. Kies EXIT → F1 Ja en laat F1 los. MENU hoort nu echt zichtbaar én bedienbaar
   te zijn. Open RUN-MAT om dat te bevestigen.
4. Open ChatGPT opnieuw; er hoort een bruikbaar invoerscherm te komen.
5. Herhaal vijfmaal, ook direct heropenen zonder RUN-MAT ertussen.
6. Test Nee en de fysieke MENU-toets vanuit ChatGPT.
7. Controleer behoud van chats, schermranden, het witte afgeronde pictogramvlak,
   de J en de leesbare appnaam.

Als de menuovergang alsnog niet klopt, zijn de **versie 9.0**, de gemelde
**menu-route** en het schermgedrag nodig om gericht verder te onderzoeken.

## Verificatie

Tien softwaretestgroepen met ASan/UBSan slagen. De nieuwe ROM-herkenning is
getest op alle registerparen, voorwaartse/achterwaartse branches, grenzen,
verkeerde/oneven adressen, leesfouten en ontbrekende/dubbelzinnige patronen.
De appcontroller doorloopt drie native menu-hervattingen en drie timer-
hervattingen. De timertest laat injectie uitsluitend tijdens de key-wait toe.
Ook timerallocatie-, start- en onderbrekingsfouten houden de UART open en
laten geen timer achter. Bestaande UI/opslag/API-parsertests blijven slagen.

De daadwerkelijke .g3a is opnieuw gebouwd, met de aanvullende syscall-stub.
Atom-broncode en binaries zijn ongewijzigd V3. Bronnen en beperkingen:
**docs/V9-NATIVE-MENU.md**. De echte OS 03.81.0202 is hier niet uitgevoerd;
de fysieke controle hierboven blijft noodzakelijk. Er zijn geen betaalde
API-aanvragen gedaan.

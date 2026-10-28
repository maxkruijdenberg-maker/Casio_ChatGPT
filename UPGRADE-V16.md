> Nieuwste versie: [UPGRADE-V17.md](UPGRADE-V17.md). V17 vereist ook nieuwe Atom-firmware.

# V16 — V13 als basis, een rustigere interface

## Installeren

Vanaf V11, V12, V13, V14 of V15:

1. Pak de volledige V16-ZIP uit.
2. Verbind de Casio met Windows en kies USB Flash / USB-opslag.
3. Vervang `ChatGPT.g3a` in de hoofdmap met `dist/ChatGPT.g3a` uit deze ZIP.
   Bewaar geen tweede exemplaar zoals `ChatGPT (1).g3a`.
4. Laat de map `CGPT3` staan: daarin staan je chats en scherminstellingen.
5. Werp veilig uit, ontkoppel de Casio-USB en herstart de Casio eenmaal.
6. Laat de Atom via zijn eigen USB gevoed. Open ChatGPT.
7. Controleer Instellingen → Hulp en versie → Over deze app: **Versie 16.0**.

De Atom hoeft vanaf V11 niet opnieuw geüpload te worden. De broncode en vier
firmwarebestanden zijn gelijk aan de oorspronkelijke V11. Wifi, sleutel en
model blijven op de Atom; opgeslagen chats blijven op de Casio.
Van vóór V11: volg de Atom-installatie in UPGRADE-V11.md met de Casio-app uit V16.

## Wat teruggaat naar V13

De fysieke toetsen, UART-opencontrole, aparte achtergrondparser en retries van
V13 worden opnieuw gebruikt. Een enkele gemiste controle sluit de poort niet;
na drie gemiste controles volgt één reset, daarna automatisch opnieuw proberen.
De V15-wachttijd van 15 seconden met herstel-cooldown en de gedeelde parser zijn
verwijderd. Ook het extra afwachten van een drukke verzendbuffer uit V15 vervalt.
Een schrijffout wordt weer volgens V13 hersteld met dezelfde requestbytes.

De seriële poort blijft bij gezonde verbinding open tijdens MENU en heropenen,
zoals in V13. De achtergrondcontrole is beperkt tot invoer en het hoofdmenu van
Instellingen, niet ieder dialoogvenster. Alleen een echte verandering van
Verbonden/Verbroken veroorzaakt daar een hertekening; een gewone controle niet.
Dit is geen claim dat losse bedrading of voeding softwarematig is opgelost.

## Wat van de nieuwere interface blijft

- Donker ontwerp met mintgroen, volledige schermvulling en normale menu-iconen.
- Vijf instellingen: Wifi en account, Model, Verbinding, Scherm en bediening,
  Hulp en versie. EXE/rechts/F1 opent; EXIT/links gaat terug.
- Alleen MENU vraagt om afsluiten. EXIT gaat terug binnen de app en blijft op het
  beginscherm staan. Blijven bewaart je concept en selectie.
- Modellen kiezen, eigen API-account via telefoon, lokale chats en klantentaal
  bij de verbindingscontrole. Die controle test geen internet of API-tegoed.
- Alleen Instellingen → Hulp en versie → Over deze app toont de appversie.

## Wachten en antwoorden

Na Stuur verschijnt alleen een groot, rustig pulserend ChatGPT-logo onder de
bovenbalk. Geen statusregels, ronddraaiende stippen of knoppenbalk tijdens wachten.
EXIT of F1 annuleert nog steeds; MENU opent de afsluitbevestiging.
Een fout toont uiteraard een begrijpelijke foutmelding in plaats van eindeloos laden.

Gewone `=` en de typografische varianten `＝`/`﹦` worden leesbaar getoond.
Bijvoorbeeld `𝑥 − 2 ＝ 4` wordt `x - 2 = 4`. Dit is alleen weergave: de oorspronkelijke
tekst blijft in chats en gesprekscontext. `≠`, `≤` en `≥` behouden hun betekenis.
Het is geen volledige LaTeX-renderer. Zelf typen kan met SHIFT + punt of F4 Tekens;
er is geen aparte =-knop. F3 opent Chats.

## Gecontroleerd en nog te controleren

Dertien ASan/UBSan-testgroepen, de SH4-build en 40 gesimuleerde MENU/heropeningen
controleren o.a. V13-herstel, toetsgebruik, schermvulling, modellen, telefoonsetup,
opslag, antwoordtekens en annuleren zonder automatische betaalde herhaling.
Twee heartbeat-intervallen in een stilstaand instellingenmenu veroorzaken in de
hosttest geen hertekening. MENU → Blijven herstelt ook een onderliggende bevestiging.

Er is hier geen fysieke Casio/Atom of API-sleutel. Controleer op jouw apparaat:

1. Veel typen en twintig keer afsluiten/heropenen; daarna ook andere Casio-apps openen.
2. Alle vijf instellingen met EXE én pijl rechts; terug met EXIT/links.
3. Minstens 30 seconden in Instellingen blijven: geen flikkeren of selectiesprongen.
4. Atom uit/aan: Verbroken en daarna Verbonden op invoer of Instellingen.
5. `23+42` en `Los 2x+5=17 op`: laadlogo, leesbare = en bewaarde antwoorden.
6. Annuleren en MENU → Blijven, ook vanuit een andere bevestiging.

Bewaar een kopie van de vorige werkende .g3a om zo nodig terug te gaan.

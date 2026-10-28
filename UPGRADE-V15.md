> Nieuwste versie: [UPGRADE-V16.md](UPGRADE-V16.md). Dit document beschrijft V15.

# V15 — eenvoudiger voor kopers

## Installeren vanaf V11, V12, V13 of V14

1. Pak het volledige V15-pakket uit.
2. Sluit de Casio op Windows aan en kies USB Flash / USB-opslag.
3. Vervang alleen `ChatGPT.g3a` in de hoofdmap door `dist\ChatGPT.g3a` uit V15.
   Laat geen tweede `ChatGPT (1).g3a` staan.
4. Laat `CGPT3` staan: hierin staan je chats en scherminstellingen.
5. Werp veilig uit, ontkoppel de Casio-USB en herstart één keer.
6. Laat de Atom via zijn eigen USB gevoed. Open ChatGPT.
7. Instellingen → Hulp en apparaat → Over deze app toont **Versie 15.0**.

**De Atom hoeft vanaf V11 niet opnieuw geüpload te worden.** Zijn broncode en
alle vier firmware-/bootbestanden zijn identiek aan V11. Wifi, sleutel en model
blijven behouden. Ook native menu-resolver, toetsdecoder en opslagformaat blijven.
Van vóór V11: gebruik de Atom-installatiestappen in UPGRADE-V11.md met de Casio-app uit V15.

## Wat een koper ziet

- Rechtsboven uitsluitend **Verbonden** of **Verbroken**. Dit is het laatst
  gecontroleerde contact tussen Casio en module, niet een wifi-/internetgarantie.
- Instellingen heeft vijf overzichtelijke kaarten. Bij elke pijl werken
  **EXE, pijl rechts en F1 Open**. EXIT of pijl links gaat terug.
- Wifi en account opent direct de installatie via telefoon, in plaats van losse
  technische wifi-/sleutelvelden op de rekenmachine.
- Verbinding doet een gratis lokale controle. Het resultaat is **Verbinding goed**,
  **Verbinding wisselt** of **Geen verbinding**, met een korte uitleg en Opnieuw.
  Er worden geen protocollen, meetwaarden, tellers of pininstellingen getoond.
- Over deze app en wissen staan onder Hulp en apparaat. Scherminstellingen blijven
  apart beschikbaar. Modellen kiezen blijft zoals in V14, met relatieve kosten.
- Geen Ruimte 100% of vergelijkbare invoermeter. F3 opent Chats, niet een =-knop.
- Tijdens versturen staat een groot ChatGPT-logo met bewegende stippen. Geen
  technische wifi-/klok-/API-statuslijst. EXIT of F1 Annuleer stopt het wachten;
  een reeds verstuurde vraag kan nog kosten maken. MENU blijft om afsluiten vragen.

## = in ontvangen antwoorden

Gewone ASCII `=` was al onderdeel van het lettertype. V15 voegt een expliciete
antwoordweergave toe voor typografische varianten zoals `＝` en `﹦`, Unicode-min,
veelvoorkomende wiskundige lettervormen en LaTeX-mathafbakening `\(...\)`/`\[...\]`.
Daardoor wordt bijvoorbeeld `𝑥 − 2 ＝ 4` leesbaar als `x - 2 = 4`.

Dit verandert alleen het beeld: de originele antwoordtekst blijft ongewijzigd
in de chatbestanden en in de context voor een vervolgvraag. `≠`, `≤`, `≥` en
andere relaties worden niet tot `=` omgezet. Het is geen volledige LaTeX-renderer
of bewijs dat iedere mogelijke Unicode-formule wordt ondersteund.
SHIFT gevolgd door punt en F4 Tekens blijven beschikbaar om zelf `=` te typen.

## Gerichte verbeteringen aan de verbinding

Dit is een seriële kabelverbinding; er is geen meetbare 'wifi-sterkte' van de
Casio naar de Atom. De gebruikerscontrole beoordeelt geldige antwoorden,
herhalingen en fouten, en toont alleen een begrijpelijke conclusie.

- Dezelfde ontvangstparser blijft actief wanneer een achtergrondcontrole plaats
  maakt voor bijvoorbeeld Instellingen. Een half ontvangen antwoord wordt niet
  weggegooid; geldige late berichten veroorzaken geen kunstmatige framefout.
- Kortstondige drukte in de verzendbuffer wordt maximaal 400 ms afgewacht met
  dezelfde berichtbytes, voordat een echt schrijffout-herstel wordt geprobeerd.
- Een gemist antwoord sluit de poort niet. Na drie mislukte controles verandert
  de status; de poort blijft open en de app controleert verder.
- Bij aanhoudende stilte kan na 15 seconden één driverherstel plaatsvinden,
  met minstens 30 seconden tussen zulke herstelacties. Geen heropenlus bij toetsen.
- Controles werken ook op andere schermen. Tijdens een gewone opdracht of
  afsluitbevestiging wordt geen achtergrondbericht tussendoor gestuurd.
- Instellingen hergebruikt openbare reeds opgehaalde gegevens; openen van het
  menu veroorzaakt niet steeds een nieuwe reeks instellingenverzoeken.

Er wordt voor herstel nooit automatisch een nieuwe betaalde vraag gestart.
De request-ledger en atomaire instellingenopslag op de Atom zijn ongewijzigd.
Software kan een slechte GND, losse draad, uitvallende voeding of OS-ingreep
niet uitsluiten. Een permanent stabiele fysieke verbinding is hier niet bewezen.

## Verkoopcontrole op echte hardware

De build en dertien ASan/UBSan-testgroepen slagen, waaronder antwoordtekens,
EXE/rechts voor de instellingen, alle modelkeuzes, een drukke zendbuffer zonder
poortsluiting, een half ontvangen heartbeat bij schermwisseling, langdurig herstel
zonder heropenlus, laadframes/annuleren en 40 menuheropeningen.

Controleer vóór verkoop op jouw Casio:

1. Veel typen, Instellingen openen/sluiten en twintigmaal MENU/heropenen.
2. Alle vijf instellingen via EXE én pijl rechts; terug met EXIT.
3. Module uit/aan: Verbroken en daarna automatisch Verbonden; ook op andere schermen.
4. Verbindingscontrole met goede en ontbrekende aansluiting, zonder misleidende waarden.
5. Een echte API-vraag per aangeboden model, inclusief `23+42` en `2x+5=17`.
6. Leesbare =-tekens, bewaarde chats, telefooninstallatie en de laadanimatie.
7. Annuleren en MENU → Blijven zonder conceptverlies of dubbele betaalde vraag.

Er is hier geen fysieke Casio/Atom of API-sleutel. De softwaretests en preview
zijn geen fysieke duurtest of verkoopcertificatie. Zie docs/V15-STARTGIDS.md en
docs/V15-TIKTOK.md voor de kopersinformatie en demonstratie.

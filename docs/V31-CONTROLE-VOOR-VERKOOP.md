# V31 — praktijkcontrole en vrijgave per apparaat

V31 is een kandidaat voor gecontroleerd praktijkgebruik. Een geslaagde build en
hosttests zijn geen fysieke duurtest, live modeltest of verkoopvrijgave. Vul deze
lijst per gebouwd apparaat in en leg onopgeloste fouten vast. De maker beslist
op basis van de echte uitkomsten of dit specifieke apparaat geleverd kan worden.

Apparaatnummer: __________  Datum: __________  Controleur: __________
Casio-OS-versie: __________  Casio-app: 31  Atom-firmware: 31

## Pakket en bestaande gegevens

- [ ] Gebruik Casio-app en Atom-firmware uit hetzelfde V31-pakket; hashes kloppen.
- [ ] Open START-HIER.html, de PDF en de HTML-handleiding offline op Windows.
- [ ] Controleer onderdelen, montage, de bestaande aansluitingen en beide voedingen.
- [ ] Werk zonder volledig wissen bij; wifi, API-sleutel, modelkeuze en CGPT3 zijn behouden.
- [ ] Instellingen → Meer → Over deze app toont Versie 31.
- [ ] Een oudere Atom geeft bij de nieuwe accountfunctie een begrijpelijke update-instructie.

## Eerste gebruik en dagelijks bedienen

- [ ] Twintigmaal openen en met MENU afsluiten; daarna ook andere Casio-apps openen.
- [ ] MENU → Blijven bewaart concept en verbinding; EXIT op het beginscherm sluit de app niet.
- [ ] EXIT vanuit een chat opent lokaal het beginscherm, ook zonder Atom; geen laadplaneet.
- [ ] De verlaten chat kan met alle opgeslagen berichten via F3 Chats worden heropend.
- [ ] EXIT met concept of onopgeslagen antwoord vraagt bevestiging; Nee bewaart beide.
- [ ] Lang typen, DEL vasthouden, cursor verplaatsen en cijfers/letters wisselen zonder blijvend contactverlies.
- [ ] ALPHA, eenmalige SHIFT, automatische zinshoofdletters en wiskundefuncties werken zoals beschreven.
- [ ] Invoer met één tot acht regels groeit en krimpt; drie zichtbare invoerregels in een chat.
- [ ] Tekst en cursor overlappen niet de invoerstand of verstuurpijl; placeholder wordt nooit verstuurd.
- [ ] Tekens toont veertig herkenbare basissymbolen; SP/spatie en het lage streepje (_) zijn zichtbaar.
- [ ] Openen met rechts/EXE, terug met links/EXIT; Hulp staat alleen als keuze in Instellingen.
- [ ] Opgeslagen gesprekken zijn offline leesbaar; omhoog/omlaag leest dezelfde tijdlijn.
- [ ] Een vervolgvraag blijft in hetzelfde gesprek; de grens van acht paren is duidelijk.
- [ ] Jij/ChatGPT-labels, regelafstand, gelijktekens en relatieglyphs zijn leesbaar bij de gekozen scherminstelling.

## Telefoon, account en modelkeuze

- [ ] Telefooninstallatie met een eigen testaccount; geen sleutel in foto, broncode of log.
- [ ] Wifi-naam, wachtwoord en sleutel behouden hun exacte letterkast; opgeslagen geheimen worden niet teruggelezen.
- [ ] Aanbevolen opent met Sol, Luna en Astra; F3 Alles maakt alle 21 ondersteunde namen bereikbaar; F3 Advies gaat terug.
- [ ] Wisselen van weergave verandert de bestaande modelinstelling niet.
- [ ] F4 Info toont uitleg en eventuele bekende einddatum; bestaande namen/bron passen bij V31-MODELLEN.md.
- [ ] Accountcontrole slaagt met het echte testaccount en toont een duidelijke uitslag en passend aantal.
- [ ] Geen van de drie aanbevelingen beschikbaar: andere gevonden modellen blijven via Alles bereikbaar.
- [ ] Geen passende namen: gecontroleerde lijst blijft leeg en de bestaande modelkeuze blijft bewaard.
- [ ] Ongeldige sleutel, geen internet en onvolledige respons geven een fout, geen gedeeltelijk schijnsucces.
- [ ] Annuleer accountcontrole tijdens verbinden en tijdens ontvangen: de laadplaneet verdwijnt, stoppen wordt bevestigd of blijft zichtbaar als nog niet bevestigd.
- [ ] Na bevestigd stoppen kan een nieuwe handeling starten; geen verborgen controle houdt het apparaat nog bezet.

## Echte antwoorden, annuleren en herstel

Gebruik alleen een eigen testaccount en noteer hieronder welke modellen echt
getest zijn. Een naam in accountmetadata is nog geen geslaagde chatvraag.

- [ ] Een korte echte vraag en minstens twee vervolgvragen met ieder model dat je aan kopers aanbeveelt.
- [ ] Controleer een antwoord met =, bewaar het, heropen het en verwijder de testchat bewust.
- [ ] Test een voor dit account niet bruikbaar model: begrijpelijke melding, geen automatische overstap naar een ander betaald model.
- [ ] Annuleer een vraag tijdens voorbereiden, tijdens wachten op de dienst en tijdens ontvangen.
- [ ] Bij annuleren verdwijnt de laadplaneet; de Casio wacht op de taakafronding en start geen automatische nieuwe betaalde vraag.
- [ ] Stoppen niet bevestigd bij verloren verbinding blijft herkenbaar; een nieuwe vraag blijft geblokkeerd tot de Atom vrij is.
- [ ] Een expliciete nieuwe poging vraagt zo nodig opnieuw om stoppen en kan geen tweede betaalde vraag starten terwijl de oude nog loopt.
- [ ] MENU tijdens een RPC én tussen statuscontroles vraagt om afsluiten en laat de oude taak stoppen.
- [ ] Verloren bevestiging van versturen geeft Versturen niet bevestigd; er volgt geen automatische tweede POST.
- [ ] Na stoppen kan een opgeslagen chat worden vervolgd met de juiste recente context; geen oude vraag of halve restauratie lekt door.
- [ ] Laat de netwerkverbinding tijdens een antwoord wegvallen; het apparaat herstelt of geeft een fout en kan daarna opnieuw gebruikt worden.
- [ ] Een stilgevallen overdracht blijft niet onbeperkt bezig; test het netwerkbudget van 120s voor chat en 60s voor account, met eventuele korte afronding van een reeds begonnen SDK-actie.
- [ ] Annuleren of time-out verstuurt dezelfde vraag niet automatisch opnieuw. Noteer request-ID's uit een testopstelling en controleer waar mogelijk de API-gebruiksregistratie; alleen kosten tellen bewijst dit niet volledig.
- [ ] Atom uit/aan in invoer en Instellingen: status wordt rood en herstelt automatisch naar groen.
- [ ] Groen betekent alleen Casio–Atom-contact; wifi- of API-fouten blijven begrijpelijk afzonderlijk zichtbaar.
- [ ] Lokale opslagfout: F3 Bewaar probeert alleen lokaal opnieuw, zonder dezelfde vraag opnieuw te betalen.
- [ ] Minstens één gebruikssessie van een uur met veel typen, chatwissels, instellingen, annuleren en heropenen.

Echt geteste modellen en vragen: __________________________________________
Onderbrekingen, hersteltijd en resterende fouten: _________________________

## Proef met een nieuwe gebruiker

Laat iemand die de app nog niet kent alleen met de meegeleverde handleiding
werken. De maker observeert en noteert problemen zonder stappen voor te zeggen.

- [ ] Pakket uitpakken, Atom/Casio bijwerken en opnieuw openen zonder mondelinge hulp.
- [ ] Eigen wifi en API-sleutel via telefoon instellen; betaald API-gebruik correct begrepen.
- [ ] Model kiezen, een vraag en vervolgvraag sturen, teruglezen en de chat heropenen.
- [ ] Annuleren en de statusmeldingen begrijpen; geen onbedoelde herhaalde vraag.
- [ ] Alle plaatsen waar hulp nodig was zijn vastgelegd en afgehandeld vóór zelfstandig gebruik wordt beloofd.

Gebruiker kon zelfstandig afronden: ja / nee
Onduidelijkheden en verbeteringen: _______________________________________

## Voor overdracht

- [ ] Testchats verwijderd; test-wifi/sleutel gewist en testsleutel ingetrokken.
- [ ] Koper gebruikt een eigen API-account en tegoed; geen gedeelde verkoperssleutel.
- [ ] Handleiding, limieten, daadwerkelijk geteste modellen en resterende fouten zijn meegeleverd.
- [ ] Geen algemene belofte dat ieder model, ieder netwerk of iedere Casio-OS-versie werkt.
- [ ] Supportafspraken en de uitkomst van deze controle zijn vastgelegd.

Uitkomst en opmerkingen: _________________________________________________
Vrijgegeven door maker: ____________________  Datum: ____________________

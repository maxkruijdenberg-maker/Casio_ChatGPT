# V32 — modellen en accountcontrole

V32 biedt 21 herkenbare GPT-chatkeuzes uit de GPT-6-, GPT-5- en GPT-4-families.
De lijst bevat algemene tekstmodellen die volgens de gecontroleerde bron op
30 oktober 2026 nog niet zijn uitgeschakeld. De Casio en Atom gebruiken dezelfde
lijst in atom/ChatGPTAtom/model_choices.h.

## Beschikbare namen

| Familie | Keuzes |
|---|---|
| GPT-6 | GPT-6 Astra, GPT-6.1 Sol, GPT-6.1 Terra, GPT-6 Sol, GPT-6 Luna |
| GPT-5.6 | GPT-5.6 Sol, GPT-5.6 Terra, GPT-5.6 Luna |
| GPT-5.5 en GPT-5.4 | GPT-5.5, GPT-5.4, GPT-5.4 Mini, GPT-5.4 Nano |
| Oudere GPT-5 | GPT-5.2, GPT-5.1, GPT-5, GPT-5 Mini, GPT-5 Nano |
| GPT-4 | GPT-4.1, GPT-4.1 Mini, GPT-4o, GPT-4o Mini |

Dit is de selectie voor tekstchat in deze app, niet een catalogus van alle
OpenAI-diensten. Datumkopieën, previews, fine-tunes, specialistische pro/codex-routes,
beeld-, audio-, video-, embedding- en moderatiemodellen staan er niet tussen.
Niet iedere SDK-naam is nog bruikbaar. Het oude GPT-4, GPT-4 Turbo en GPT-4.1 Nano
hebben in de bron een uitschakeldatum van 23 oktober 2026 en zijn daarom weggelaten.
GPT-5.1 Mini is volgens diezelfde bron geen ondersteunde modelnaam. Er wordt geen
GPT-5.3-basismodel verzonnen: de bron noemt alleen de inmiddels uitgefaseerde
GPT-5.3 Chat Latest-variant.

## Modellen met aangekondigd einde

Een aangekondigde toekomstige uitschakeling betekent niet dat het model nu al
uitgeschakeld is. Deze bekende oudere keuzes zijn daarom nog beschikbaar, met
een waarschuwing onder Info:

- GPT-5, GPT-5 Mini en GPT-5 Nano: aangekondigd einde 11 december 2026.
- GPT-4.1, GPT-4.1 Mini, GPT-4o en GPT-4o Mini: aangekondigd einde 14 januari 2027.
- GPT-5.1 en GPT-5.4 Nano: aangekondigd einde 1 april 2027.

Gebruik voor langdurig gebruik bij voorkeur een nieuwer model. Beschikbaarheid en
deze data kunnen wijzigen. Een bestaande modelinstelling wordt niet automatisch
vervangen; kies zelf een ander model als je eerdere keuze niet meer werkt.

## Aanbevolen en alle modellen

De modelkeuze begint met de drie aanbevolen namen GPT-6.1 Sol, GPT-6 Luna en
GPT-6 Astra. Die geven een kleine startlijst voor dagelijkse uitleg, korte vragen
en uitgebreidere redeneringen. Dit is advies van de app, geen onafhankelijke
benchmark en geen garantie voor snelheid, kwaliteit of kosten.

F3 Alles opent Alle modellen; F3 Advies gaat terug naar Aanbevolen. Alle 21 namen
in de tabel blijven beschikbaar. De weergave verandert je ingestelde model niet. Na accountcontrole wordt
in beide weergaven dezelfde doorsnede met je account gebruikt. Een aanbevolen model
dat jouw account niet teruggeeft wordt niet als beschikbaar toegevoegd. Als geen
van de drie aanbevelingen beschikbaar is maar andere ondersteunde namen wel,
wordt automatisch Alles getoond.

Omhoog/omlaag selecteert een regel. Rechts, EXE of F1 Kies bewaart de keuze.
Links of EXIT gaat terug. F4 Info beschrijft de toepassing en een eventuele
bekende einddatum. Tarieven zijn niet live bevestigd en worden niet als vaste
prijs getoond. Controleer https://openai.com/api/pricing/.

De lijst opent zonder internetaanvraag. F2 Account controleert welke namen jouw
account teruggeeft. Een geslaagde controle toont de uitslag en het gevonden aantal.
Een fout laat herkenbare ongecontroleerde vaste keuzes zien. Een naam in
accountmetadata bewijst nog geen geslaagde chatvraag met deze app.

## Accountcontrole en stoppen in Atom V31

De meegeleverde Atom blijft V31. Deze firmware gebruikt één metadata-aanvraag naar
GET /v1/models in plaats van een afzonderlijke aanvraag per model. De binnenkomende
JSON wordt stukje voor stukje verwerkt. Alleen de overeenkomende namen uit de
vaste keuzelijst blijven bewaard; de grote accountrespons wordt niet in één keer
in het geheugen gezet. Als alle 21 namen voorkomen, bevat het resultaat slechts
225 bytes inclusief afsluitende nul. De Casio reserveert 4 KiB voor het resultaat.

Een onvolledige, ongeldige of mislukte respons geeft geen gedeeltelijke lijst als
volledige succesvolle controle. Een geslaagde lege doorsnede blijft expliciet een
lege, gecontroleerde lijst. Wifi, TLS en de dienst kunnen nog steeds vertraging
veroorzaken; één aanvraag is geen garantie voor een bepaalde wachttijd.

EXIT of F1 vraagt nu de Atom om de accountcontrole te stoppen. De Atom beëindigt
zijn netwerkwerk en geeft de taak pas vrij na de afronding. De Casio wacht op de
bevestiging; kan dat niet meteen, dan blijft het stoppen zichtbaar. Een lopende
netwerkleesactie kan nog enkele seconden duren. De app verbergt geen lopende
controle achter een klaar-status. Zonder bevestiging staat er Stoppen niet bevestigd
en blijft een nieuwe vraag geblokkeerd. Een expliciete nieuwe poging controleert
eerst de vorige taak en vraagt zo nodig nogmaals om stoppen. Er start geen
vervangende betaalde aanvraag.

Accountcontrole heeft een netwerkbudget van 60 seconden. Een begonnen SDK-actie
kan nog kort doorlopen binnen zijn eigen begrensde time-out. Ook bij een gewone
chatvraag wacht de app bij annuleren op de taakafronding; het netwerkbudget van die
route is 120 seconden. Dit zijn grenzen voor herstel, geen snelheidsbelofte voor
een model en geen garantie op een exacte annuleertijd.

Deze controle start geen betaalde chatvraag. Een naam in accountmetadata garandeert
nog geen werkende chatvraag met jouw sleutel, projectrechten en tegoed.

## Bron en grenzen van de controle

De modelnamen en de datums hierboven zijn gecontroleerd op 30 oktober 2026 met de
lokaal bewaarde officiële SDK-bron die op 29 oktober 2026 werd opgehaald:

https://github.com/openai/openai-python/blob/f552fcffcf774520e946121704b5fbbf7ba986e8/src/openai/types/shared/chat_model.py

SHA-256 bronbestand: d2a74c51d4ee5cf1f782fc753c3db15d49cb19f8c3d04cadc4b76775b6ba4167

Dit is geen nieuwe live broncontrole van 30 oktober. De eerdere officiële modellen-
en prijswebpagina's waren vanuit de bouwomgeving niet op te halen. Het blok
Deprecated values in de SDK onderscheidt toekomstige uitschakelingen, reeds
verstreken uitschakelingen en een ongeldige naam. Alleen voorkomen in dat blok is
op zichzelf geen bewijs dat een model al is uitgeschakeld.

SDK-vermelding en accountmetadata bewijzen geen succesvolle completion. De Atom
blijft voor chatvragen Chat Completions, max_completion_tokens, platte tekst en
dezelfde registratie tegen dubbele betaalde verzoeken gebruiken. De app kiest
niet automatisch een ander model of een andere betaalde route. In de bouwomgeving
is geen echte API-sleutel gebruikt en geen live betaalde vraag getest.

## Bijwerken

De modelnamen en de Atom-code/firmware in V32 zijn ongewijzigd ten opzichte van V31.
De aanpassing zit in de Casio-app: EXIT en F1 werken ook tijdens de voorbereiding
van een chatvraag. Een annulering vóór de verstuuropdracht laat geen nieuwe vraag
volgen. Een opdracht die al naar de Atom is verzonden kan wel al een betaalde
API-aanvraag hebben gestart; stoppen kan kosten niet terugdraaien.

Heb je Atom V31 al geïnstalleerd, vervang dan alleen ChatGPT.g3a, laat CGPT3 staan
en herstart de Casio eenmaal. Heb je een oudere Atom, werk die eerst bij met de
meegeleverde V31-firmware zonder volledig wissen. Accountcontrole, aanbevelingen,
Info, bekende modeldatums en de grenzen van de broncontrole blijven behouden.

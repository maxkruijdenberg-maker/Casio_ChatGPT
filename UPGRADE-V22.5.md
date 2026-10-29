# V22.5 — één doorlopende chat

Je hoeft geen aparte antwoordpagina meer te verlaten om verder te typen.
De invoerbalk staat onderaan hetzelfde gesprek. Omhoog/omlaag leest eerdere
vragen en antwoorden. Links/rechts verplaatst de invoercursor. Stuur met F1 of EXE.
ALPHA-lock en je hoofdletterstand blijven na versturen behouden.

Vorig, Volgend en Verder zijn uit de chat verdwenen. De vaste functiebalk is
Stuur / Nieuw / Chats / Tekens / ABC/abc / Instel. Bij een opslagprobleem staat
Bewaar tijdelijk op F3. OPTN opent de zijbalk. Alleen Nieuw begint bewust een
nieuw gesprek; bladeren door historie en vervolgvragen doen dat niet.

De invoer toont in een bestaande chat maximaal drie regels tegelijk en schuift
bij langere tekst mee met de cursor. De historie blijft erboven zichtbaar.
Het beginscherm zonder gesprek behoudt de eerdere meegroeiende invoer.

## Bijwerken

Atom V22.3 al geïnstalleerd? Vervang alleen dist/ChatGPT.g3a op de Casio,
laat CGPT3 staan en herstart de Casio. Over deze app toont Versie 22.5.
Voor een oudere Atom: volg de volledige installatie zonder volledig wissen.
De Atom-code en firmware zijn bytegelijk aan V22.3 en V22.4.

## Behoud en grenzen

Bestaande gesprekken blijven leesbaar, zonder conversie van CGPT3. Acht gesprekken
met elk maximaal acht paren blijven de opslaggrens. Geen stilzwijgend verwijderen
van oude berichten of automatisch openen van een nieuwe chat. De bestaande
begrenzing van recente API-context blijft behouden. Een volle chat meldt dit.

Alleen rij-aantallen en offsets van de actieve tijdlijn worden gecachet (152 bytes).
Er wordt geen grote tekst met alle antwoorden aaneengeplakt. Alleen de zichtbare
paren worden voor tekenen herlezen. De oorspronkelijke vraag/antwoordbuffers blijven
beschermd tijdens scrollen. Onopgeslagen antwoorden blijven zichtbaar en zijn
zoals voorheen beschermd tegen een tweede betaalde vraag of onbedoeld verlies.

Geen wijziging aan de betaalde Atom-vraagroute, modelcontrole, UART of native
menuhelpers. Tests controleren twee vervolgvragen in dezelfde slot, de echte
upload-/contextflow met UART-stubs, offline scrollen met conceptbehoud, ALPHA-lock,
opslagherstel, historiegrenzen en veertig native menuovergangen. Geen live API
of fysieke Casio in de bouwomgeving getest.

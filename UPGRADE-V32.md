# V32 — annuleren voordat een vraag wordt verstuurd

V32 herstelt een fout in de Casio-app V31. EXIT of F1 kon tijdens de eerste
laadfase worden gelezen zonder dat de voorbereiding stopte. De app kon daarna
toch een vraag versturen. De Atom-firmware blijft ongewijzigd V31.

- EXIT en F1 worden nu ook verwerkt tijdens het herstellen van de recente
  chatcontext, het overzetten van de vraag en het aanvragen van een verzoeknummer.
- Een annulering vóór de verstuuropdracht naar de Atom onderbreekt de hele
  voorbereiding. Daarna volgt geen W_ASK-verstuuropdracht voor die vraag.
- Ook de overgang tussen de voorbereiding en het versturen wordt gecontroleerd.
  Een snelle bevestiging mag een al waargenomen annulering niet overslaan.
- Het concept blijft beschikbaar. Een volgende vraag begint pas als je die zelf
  bewust verstuurt; afgebroken werk wordt niet automatisch hervat.
- Zodra de verstuuropdracht al naar de Atom is verzonden, kan een API-aanvraag
  gestart zijn. Dan blijft de bestaande stopafhandeling van V31 gelden. De app
  wacht op taakafronding of toont Stoppen niet bevestigd. Een al gestarte vraag
  kan kosten hebben gemaakt; annuleren maakt die kosten niet ongedaan.
- De drie aanbevelingen, alle 21 modellen, Info, lokale chats, toetsenindeling en
  schermopmaak blijven behouden. Er is geen nieuwe live modelbroncontrole gedaan.

De regressieproeven gebruiken vertraagde host-UART-antwoorden en controleren dat
na vroege annulering geen W_ASK-verstuuropdracht volgt en geen nieuw antwoord wordt
opgeslagen. Dat zijn softwareproeven, geen fysieke apparaat- of live API-test.
De definitieve testuitkomsten staan in de bouw- en testlogs van deze release.

## Bijwerken

Atom V31 al geïnstalleerd? Vervang alleen dist/ChatGPT.g3a in de hoofdmap van de
Casio, laat CGPT3 staan en herstart eenmaal. Instellingen → Meer → Over deze app
toont Versie 32. De meegeleverde Atom-code en vier firmwarebestanden zijn dezelfde
als in V31; opnieuw flashen is dan niet nodig.

Heb je een oudere Atom of weet je het niet? Upload eerst de meegeleverde
Atom-firmware V31 volgens de handleiding. Laat Erase All Flash / volledig wissen
uitgeschakeld om wifi, API-sleutel en model normaal te behouden. Installeer daarna
de Casio-app V32. De bestaande modelinstelling wordt niet automatisch vervangen.

## Download en gebruik

Pak ChatGPT-V32-installatie.zip volledig uit en open START-HIER.html.
De handleidingen werken offline: docs/Handleiding-V32.pdf en
docs/Handleiding-V32.html. Het GitHub-versielabel is v32.0.0.

V32 blijft een kandidaat voor praktijkcontrole. Voer de controles in
docs/V32-CONTROLE-VOOR-VERKOOP.md uit op echte hardware en met een eigen API-testaccount
voordat een apparaat voor verkoop wordt vrijgegeven. De bouwomgeving heeft geen
fysieke Casio/Atom of live betaald API-account getest.

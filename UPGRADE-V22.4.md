# V22.4 — neutrale status en leesbare wiskundetekens

- Verbonden en Verbroken staan in dezelfde neutrale tekstkleur.
- Het gekleurde statusvlak is verwijderd; alleen het bolletje is groen of rood.
- Gewoon, breed, klein, super- en subscript = wordt leesbaar weergegeven.
- Nieuwe glyphs voor ≈ ≃ ≅ ≔ ≕ ≡ ≝ ≜ ≟ ≢ ≉; geen vraagtekenachtig vervangteken.
- Dubbele/drievoudige gelijktekens worden als == en === weergegeven.
- Wiskundige betekenis blijft behouden: ≈, ≠, ≤ en ≥ worden geen =.
- Werkelijke vraagtekens blijven staan; originele antwoorden en chatbestanden
  worden niet herschreven. Dit is weergaveverbetering, geen wijziging van AI-antwoorden.

## Bijwerken

Atom V22.3 al geïnstalleerd? Kopieer alleen dist/ChatGPT.g3a naar de hoofdmap van
de Casio, vervang de oude versie, laat CGPT3 staan en herstart de Casio eenmaal.
Instellingen → Meer → Over deze app toont Versie 22.4.

De meegeleverde Atom-firmware is bytegelijk aan V22.3. Heb je een oudere Atom,
volg dan de volledige installatie in de handleiding, zonder volledig wissen.

De geheugenarme accountcontrole, betaalde vraagroute, tokenbudget, toetsen,
opslag en native MENU zijn ongewijzigd. De exacte antwoordtekst van de melding
van de gebruiker was niet beschikbaar; gewone = werkte al in de vorige tests.
Nu worden aanvullende Unicode-schrijfwijzen en relatieglyphs ondersteund.
Tests controleren de echte pixels van ontvangen en weergegeven gelijktekens,
neutraliteit van statustekst, conversie en betekenisbehoud. Geen fysieke Casio
of live API-account in de bouwomgeving getest.

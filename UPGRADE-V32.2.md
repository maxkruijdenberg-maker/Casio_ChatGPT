# V32.2 — eenmalige SHIFT en derdemachtswortel

V32.2 herstelt de verwerking van SHIFT samen met een functietoets. Na één
verschoven functie zet dezelfde nog ingedrukte SHIFT-toets de stand niet opnieuw
aan. Daardoor wordt een volgende gewone functie niet onverwacht de inverse.

- In cijferstand 123 voegt sin sin( in. SHIFT + sin voegt asin( in: arcsinus,
  dezelfde inverse sinus als sin⁻¹ op het gele Casio-label.
- SHIFT + ( voegt ∛( in voor de derdemachtswortel. Het bijbehorende teken wordt
  door het lettertype weergegeven; bijvoorbeeld ∛(8) als tekst voor je vraag.
- SHIFT aantikken blijft één gele functie klaarzetten. Nogmaals SHIFT annuleert
  die klaarstaande stand.
- SHIFT samen met een functie vasthouden werkt bij beide loslaatvolgordes en
  bij gelijktijdig indrukken. Dezelfde SHIFT-druk wordt na de functie verbruikt.
- ALPHA blijft wisselen tussen letters en cijfers. De functies zijn vraagtekst
  voor ChatGPT, geen lokale berekening of native 2D-Casio-formule.
- Antwoorden blijven bij de eerste zin openen. Overige modellen, de korte groene
  of rode accountuitslag, F1 Wis/F2 Nieuw in Chats en vroege annulering blijven behouden.

## Bijwerken

Heb je Atom V31 al geïnstalleerd? Vervang alleen dist/ChatGPT.g3a in de hoofdmap
van de Casio. Laat CGPT3 staan en herstart eenmaal. Instellingen → Meer → Over deze app
toont Versie 32.2. Opnieuw flashen van de Atom is dan niet nodig.

Heb je een oudere Atom, upload dan eerst de meegeleverde Atom-firmware V31 volgens
de handleiding. Laat Erase All Flash / volledig wissen uitgeschakeld om wifi,
API-sleutel en model normaal te behouden. Installeer daarna de Casio-app V32.2.
De Atom-code en vier firmwarebestanden blijven gelijk aan V31.

## Pakket en controle

Pak ChatGPT-V32.2-installatie.zip volledig uit en open START-HIER.html. De
handleidingen werken offline: docs/Handleiding-V32.2.pdf en
docs/Handleiding-V32.2.html. Het GitHub-versielabel is v32.2.0.

Controleer op je apparaat vooral SHIFT aantikken, vasthouden, beide loslaatvolgordes,
gelijktijdig indrukken en de volgende gewone functietoets. Test ook ∛( en een
gewone en inverse sinus. De controles staan in docs/V32.2-CONTROLE-VOOR-VERKOOP.md.
Deze nieuwe patch is in de bouwomgeving niet op echte Casio/Atom-hardware of met
een live API getest.

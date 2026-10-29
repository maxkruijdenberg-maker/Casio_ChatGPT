# V22.8 — opgeruimde berichtenbalk en logische hoofdletters

- OPTN: chats / MENU: afsluiten en de andere losse navigatiehint onder de berichtenbalk zijn weg.
- 123 / abc / Abc / ABC staat ín de balk, direct vóór het verstuurpijltje.
- De pijl blijft zichtbaar tijdens typen. Alleen F1 Stuur of EXE verstuurt;
  het Casio-scherm is geen touchscreen.
- Een vaste ruimte rechts houdt de tekst en de cursor vrij van modus en pijl.
- De losse hoofdletterknop F5 is verwijderd en doet niets meer in de editor.
- Nieuwe gewone tekst begint na . ! ? automatisch met een hoofdletter.
- Hoofdletterlock zit onder F4 Tekens → F2 ABC aan/uit; het scherm toont de stand.
  EXIT of F6 Terug sluit Tekens. Bij lock aan schakelt de invoer naar letters.
- Geheime velden behouden de exact ingevoerde letterkast; automatische zinshoofdletters
  worden daar niet toegepast. Een handmatig gekozen lock blijft een bewuste keuze.

## Bijwerken

Atom V22.3 al geïnstalleerd? Vervang alleen dist/ChatGPT.g3a, laat CGPT3 staan
en herstart de Casio. Over deze app toont Versie 22.8. Voor een oudere Atom:
volledige installatie volgen, zonder volledig wissen. De firmware is bytegelijk
aan V22.3. Chats, menu, verbinding, modellen, gelijktekens en betaald API-verkeer
blijven ongewijzigd. Alleen nieuw getypte letters worden automatisch aangepast.

De berichtenbalk gebruikt nu 33 tekstkolommen; bestaande vragen worden niet
afgekapt of herschreven. Langere invoer blijft automatisch afbreken en schuiven.
De chat toont nog maximaal drie invoerregels tegelijk. Bestaande chatopslag en
contextlimieten veranderen niet.

Zestien testgroepen controleren nieuwe zinnen, decimalen, cursorpositie, secretcase,
lock aan/uit, F5 zonder werking, regelgrenzen en de bestaande chat-/menuovergangen.
Voorbeelden zijn echte UI-hostrenders. Geen fysieke Casio of live API-account getest.

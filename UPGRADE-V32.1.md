# V32.1 — nieuwe antwoorden vanaf de eerste zin

V32.1 past de leespositie van nieuw ontvangen antwoorden aan. Een antwoord opent
bij het ChatGPT-label en de eerste zin, zodat je direct van boven naar beneden
kunt lezen. Dat geldt voor het eerste antwoord en voor antwoorden op vervolgvragen.
De navigatiehints en enkele keuzeschermen zijn daarnaast opgeruimd.

- Met omlaag lees je verder in het antwoord; omhoog toont eerdere regels en berichten.
- Het typen van een vervolgvraag laat de weergave niet automatisch naar de laatste zin springen.
- De doorlopende chat en bestaande opgeslagen berichten blijven behouden.
- De vroege annuleringscontrole uit V32 blijft actief. Er wordt bij deze schermwijziging
  geen extra API-aanvraag verstuurd en de bestaande modelkeuze verandert niet.
- De onderste Open/Terug-hints zijn verwijderd. Rechts/EXE opent een keuze en links/EXIT gaat terug.
- De zijbalk Chats gebruikt F1 Wis en F2 Nieuw. Buiten de zijbalk blijven de gewone chatknoppen werken.
- Model kiezen toont de drie GPT-6-aanbevelingen en daaronder Overige modellen →.
  Rechts/EXE op die vierde regel opent de achttien andere namen zonder duplicaten.
  Links/EXIT gaat eerst terug naar de hoofdmodellen en daarna naar Instellingen.
  De groepsregel is geen model; F1 Kies en F4 Info gelden alleen voor echte modelregels.
  Na accountcontrole tonen beide lijsten alleen de passende accountnamen.
- Na accountcontrole verschijnt kort een groen vinkje bij succes, ook bij nul passende modellen.
  Bij een fout verschijnt kort een rood kruis en daarna de reden. Annuleren toont geen rood kruis.
- Modelnamen, gegevensopslag en de Atom-firmware blijven ongewijzigd. De accountcontrole en betaalde API-aanvragen gebruiken dezelfde bestaande route.

## Bijwerken

Heb je Atom V31 al geïnstalleerd? Vervang alleen dist/ChatGPT.g3a in de hoofdmap
van de Casio. Laat CGPT3 staan en herstart de Casio eenmaal. Instellingen → Meer →
Over deze app toont Versie 32.1. Opnieuw flashen van de Atom is dan niet nodig.

Heb je een oudere Atom, upload dan eerst de meegeleverde Atom-firmware V31 volgens
de handleiding. Laat Erase All Flash / volledig wissen uitgeschakeld om wifi,
API-sleutel en model normaal te behouden. Installeer daarna de Casio-app V32.1.

## Pakket en controle

Pak ChatGPT-V32.1-installatie.zip volledig uit en open START-HIER.html. De
handleidingen werken offline: docs/Handleiding-V32.1.pdf en
docs/Handleiding-V32.1.html. Het GitHub-versielabel is v32.1.0.

Controleer op je apparaat vooral een lang eerste antwoord, een lang antwoord op
een vervolgvraag, omhoog/omlaag lezen en typen van een concept met meerdere regels.
De controles staan in docs/V32.1-CONTROLE-VOOR-VERKOOP.md. De nieuwe patch is niet
in deze bouwomgeving op echte Casio/Atom-hardware of met een live API getest.

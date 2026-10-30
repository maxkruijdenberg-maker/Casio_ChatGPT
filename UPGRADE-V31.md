# V31 — begrensd netwerkwerk en duidelijk stoppen

V31 werkt de foutafhandeling van V30 bij. Het uiterlijk en de bestaande chats
blijven behouden. Dit pakket is een kandidaat voor praktijkcontrole; de softwarebuild
is geen verklaring dat een gebouwd apparaat klaar is voor verkoop.

- Chatverzoeken bewaken nu de totale tijd van het netwerkwerk. Een verbinding
  die open blijft zonder gegevens te leveren kan de antwoordtaak niet onbeperkt
  bezet houden. Annulering wordt ook aan de netwerkgrens gecontroleerd.
- De laadplaneet verdwijnt bij annuleren. De Casio wacht op de bevestiging dat de
  Atom de taak heeft afgerond. Zonder bevestiging staat er Stoppen niet bevestigd
  en blijft een nieuwe vraag geblokkeerd. Een expliciete nieuwe poging controleert
  eerst de oude taak en vraagt zo nodig nogmaals om stoppen.
- Chatverzoeken hebben een netwerkbudget van 120 seconden en accountcontrole
  60 seconden. Een lopende SDK-lees- of verbindingsactie kan nog kort doorlopen
  binnen zijn eigen time-out. Er is geen garantie op een exacte annuleertijd.
  Een eerdere API-vraag kan kosten hebben gemaakt; er wordt niet automatisch
  opnieuw betaald gevraagd.
- Accountcontrole annuleren vraagt nu ook de Atom om te stoppen. De Casio wacht
  op bevestiging en houdt een nog lopende afronding zichtbaar. De taak wordt niet
  voortijdig als vrij opgegeven terwijl hij nog netwerkwerk doet.
- Model kiezen begint met GPT-6.1 Sol, GPT-6 Luna en GPT-6 Astra als appadvies.
  F3 Alles opent Alle modellen; F3 Advies keert terug naar Aanbevolen. Alle 21
  ondersteunde namen blijven bereikbaar. De huidige
  modelinstelling wordt niet gewijzigd door het wisselen van weergave.
- Na accountcontrole worden alleen passende accountnamen getoond. Zijn de drie
  aanbevelingen niet beschikbaar maar andere ondersteunde namen wel, dan opent
  de app automatisch Alles. Een lege accountdoorsnede blijft expliciet leeg.
- Accountmetadata blijft een toegangscontrole, geen bewijs dat een betaalde
  chatvraag met ieder model zal slagen. De modelbron en bekende einddatums zijn
  niet opnieuw live geverifieerd; zie docs/V31-MODELLEN.md.
- Bij stoppen wordt de oude Atom-context opgeruimd. Een volgende vraag herstelt
  de recente context uit de opgeslagen lokale chat; opgeslagen chats blijven staan.
- EXIT naar het beginscherm, pijlnavigatie, basissymbolen, automatische hoofdletters,
  chatopslag en bescherming tegen dubbele betaalde aanvragen blijven behouden.

## Bijwerken

Werk beide onderdelen bij: upload de meegeleverde Atom-firmware V31 en vervang
het bestaande ChatGPT.g3a met dist/ChatGPT.g3a. Laat volledig wissen uitgeschakeld,
bewaar CGPT3 en herstart de Casio eenmaal. De app toont Versie 31 onder
Instellingen → Meer → Over deze app. De bestaande modelkeuze wordt niet stilzwijgend
vervangen. Wifi en API-sleutel blijven bij een normale update behouden.

Voor de nieuwe account-stopfunctie is Atom V31 nodig. Gebruik Casio en Atom uit
hetzelfde pakket; de oude firmware heeft deze volledige stopafhandeling niet.

## Wat nog in de praktijk moet gebeuren

De bouwomgeving heeft geen fysieke Casio/Atom of live betaald API-account gebruikt.
Voer de controle in docs/V31-CONTROLE-VOOR-VERKOOP.md uit: heropenen en afsluiten,
lang typen, offline herstel, echte vragen en vervolgvragen, stoppen tijdens
netwerkwerk en een proefinstallatie door iemand die de app nog niet kent.
Documenteer de uitkomst per apparaat en per werkelijk getest model.

De PDF en offline handleiding staan in docs/Handleiding-V31.pdf en
docs/Handleiding-V31.html. Open na uitpakken START-HIER.html.

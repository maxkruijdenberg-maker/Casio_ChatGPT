# V30 — logisch teruggaan en eenvoudige tekens

- EXIT gaat vanuit een chat terug naar het beginscherm. Het opgeslagen gesprek
  blijft via F3 Chats beschikbaar. Teruggaan gebeurt lokaal, zonder laadicoon
  en zonder een aanvraag naar de Atom.
- Een ongestuurd concept vraagt eerst bevestiging voordat het wordt verworpen.
  Een antwoord dat niet opgeslagen is geeft een aparte waarschuwing. Kies Nee
  om in je gesprek te blijven en verder te werken.
- Op het beginscherm behoudt EXIT je concept en blijf je binnen de app.
  Alleen MENU vraagt om de app af te sluiten.
- Bij annuleren verdwijnt de laadplaneet direct, voordat de Atom de annulering
  verwerkt. Er wordt geen nieuwe betaalde vraag gestart. Een eerdere aanvraag
  kan al verstuurd zijn en nog kosten maken.
- Tekens bevat veertig basissymbolen voor rekenen en leestekens, zonder alfabet
  of cijfers. Een duidelijke rand markeert je keuze. SP is de spatiekeuze;
  bij selectie verschijnt Spatie, zodat er geen onherkenbaar leeg vak meer is.
- Lijsten openen met EXE of pijl rechts; pijl links of EXIT gaat terug. De
  losse F-knoppen Open en Terug zijn verwijderd. Hulp staat alleen in Instellingen.
- De modelkeuze is uitgebreid met herkenbare modelnamen en Info. F2 Account
  gebruikt in Atom V30 één gestreamde GET met begrensd geheugengebruik, in plaats
  van een aparte aanvraag per model. Succes toont Account gecontroleerd; een fout
  toont Account niet gecontroleerd en herkenbare vaste keuzes. Metadata alleen
  garandeert nog geen geslaagde betaalde chatvraag.
- ALPHA, automatische zinshoofdletters en doorlopende chat blijven behouden.
  Er is geen handmatige hoofdletterlock.

Het eerder onzichtbare lage streepje (_) is hersteld in het lettertype. De spatie heeft een zichtbaar SP-vakje.

## Bijwerken

Werk voor V30 beide onderdelen bij. Upload de meegeleverde Atom-firmware V30
volgens de handleiding en vervang dist/ChatGPT.g3a in de hoofdmap van de Casio.
Laat volledig wissen uitgeschakeld om wifi en API-sleutel normaal te behouden.
Laat CGPT3 staan en herstart de Casio eenmaal. Instellingen → Meer → Over deze app
toont Versie 30.

Met een oudere Atom blijven bestaande chats compatibel, maar de vernieuwde
F2-accountcontrole vraagt om Atom V30. Wifi en API-sleutel blijven via je telefoon
instelbaar.

Gespreksopslag, contextlimieten, UART en de bestaande beveiliging tegen dubbele
betaalde aanvragen blijven behouden. Modelkeuze en accountcontrole zijn vernieuwd.

Voor installatie en bediening: docs/Handleiding-V30.pdf of docs/Handleiding-V30.html.
Controles op het echte apparaat staan in docs/V30-CONTROLE-VOOR-VERKOOP.md.
Softwaretests vervangen geen fysieke toets-, verbindings- of duurtest.

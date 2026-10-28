# V16 — wat gebruiksvriendelijk hier betekent

Een koper moet de gewone taken zonder technische kennis kunnen uitvoeren:
instellen, een vraag sturen, een antwoord lezen, teruggaan en afsluiten.
De keuzes volgen bekende bruikbaarheidsprincipes: zichtbare systeemstatus,
herkenbare taal, consistente bediening, controle door de gebruiker en foutpreventie.
Deze principes zijn onder meer beschreven in Nielsen Norman Group,
[10 Usability Heuristics for User Interface Design](https://www.nngroup.com/articles/ten-usability-heuristics/).
De website kon in deze bouwomgeving niet worden opgehaald; er is geen recente
inhoudscontrole of gebruikersonderzoek met kopers uitgevoerd.

| Doel | Toepassing in V16 |
| --- | --- |
| Weten waar je aan toe bent | Bovenbalk met Verbonden/Verbroken; pulserend logo tijdens wachten |
| Herkennen in plaats van onthouden | Vijf benoemde instellingen, modelkeuzes en één verbindingscontrole |
| Vaste bediening | EXE/rechts opent een rij met >; EXIT/links terug in lijsten; MENU afsluiten |
| Rust en leesbaarheid | Geen versie, ruimtepercentage of technische voortgang op het hoofdscherm |
| Controle houden | Blijven bewaart concept/selectie; wachten is te annuleren; wissen vraagt bevestiging |
| Fouten voorkomen | Geen automatische nieuwe betaalde vraag bij herstel; geen geheimen op het scherm |
| Begrijpelijke uitleg | Diagnose in klantentaal; contact met Atom onderscheiden van internet/API-tegoed |

Scherm aanpassen is een bewuste uitzondering: links/rechts wijzigt de geselecteerde
waarde; EXE/F1 slaat op en EXIT annuleert. In de teksteditor bewegen de pijlen de
cursor. Die aanwijzingen staan op de betreffende schermen.

De keuze voor een volledig rustige laadpagina verbergt de annuleerknop; daarom
staat EXIT/F1 uitgelegd in de gebruikersgids. De toetsen blijven actief.
Het logo blijft tijdens een gewone opdracht bewegen; bestandsopslag en een
blokkerende OS-seriële aanroep kunnen de animatie kort stilzetten.

Technische beperking: de status is het laatst gecontroleerde lokale contact met
de Atom. In een dialoog zonder achtergrondcontrole kan die status verouderen.
Controleer opnieuw via Instellingen → Verbinding. Er is geen gefingeerde
signaalsterkte en een groen label bewijst geen internet of voldoende API-tegoed.

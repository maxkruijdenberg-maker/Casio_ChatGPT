# V19 — abstract, rustig en herkenbaar

## Installeren vanaf V17 of V18

1. Pak de V19-ZIP uit.
2. Verbind de Casio met Windows en kies USB Flash / USB-opslag.
3. Vervang `ChatGPT.g3a` in de hoofdmap door `dist\ChatGPT.g3a` uit deze ZIP.
4. Laat `CGPT3` staan; daarin staan je chats en scherminstellingen.
5. Werp veilig uit, ontkoppel de Casio-USB en herstart de Casio eenmaal.
6. Open ChatGPT. Instellingen → Meer → Over deze app toont **Versie 19.0**.

De Atom hoeft vanaf V17 niet opnieuw geüpload te worden. Alle Atom-broncode en
vier firmware-/bootbestanden zijn identiek aan V17. Wifi, API-sleutel en model blijven behouden.
Vanaf V16 of ouder: volg de Atom-update in UPGRADE-V17.md en gebruik daarna de Casio-app uit V19.

## Wat verandert

- Het logo krijgt een abstract orbitmotief met zachte lila- en mintkleuren.
- Het beginscherm zet je vraag centraal, met een korte uitnodiging en een duidelijke Stuur-actie.
- Tijdens laden beweegt een accent rond het logo. De tekst legt uit wat er gebeurt,
  zonder een niet-meetbare resterende tijd te beloven. EXIT/F1 annuleert het wachten.
- Verbonden heeft groene tekst, een groen stipje en een lichte groene badge.
  Verbroken heeft rode tekst, een rood stipje en een lichte rode badge.
- De woorden blijven altijd staan: de betekenis hangt niet alleen van kleur af.
- Gekleurde focusranden blijven op de geselecteerde keuze; de overige kaarten zijn rustiger.
- Minder schaduwen en losse kadertjes onderaan. De primaire actie blijft duidelijk zichtbaar.

De ontwerpkeuzes richten zich op wat een koper dagelijks nodig heeft: snel typen,
leesbare antwoorden, herkenbare acties, chats terugvinden en hulp als iets niet lukt.
Er is geen gebruikersonderzoek uitgevoerd dat voorkeuren van een leeftijdsgroep bewijst.

## Wat hetzelfde blijft

ALPHA voor letters, eenmalige SHIFT voor wiskundefuncties, F1 Stuur, F3 Chats,
F6 Instel. EXIT gaat terug binnen de app; alleen MENU vraagt om afsluiten.
De versie staat uitsluitend bij Over deze app. De modellenlijst en haar
API-beperkingen zijn gelijk aan V17/V18. De =-weergave en lokale chatopslag blijven.

De verbinding gebruikt dezelfde controle en herstelregels. Groen betekent het
laatst gecontroleerde contact met de Atom; dit zegt niets over internet of API-tegoed.
De rode badge verandert geen voeding, pins, baudrate of achtergrondcontroles.

## Gecontroleerd

Veertien ASan/UBSan-testgroepen en de Casio SH4-build, inclusief 40 gesimuleerde
menuheropeningen, beide statuskleuren in de echte renderer, modellen, opslag en vragen.
Screenshots en GIF zijn hostrenders; de accountmodellen daarin zijn voorbeelddata.
Er is hier geen fysieke Casio/Atom of live API-sleutel.

Controleer op jouw Casio: leesbaarheid van groen/rood, Atom uit/aan, verschillende
schermen en je ingestelde schaal, veel typen, een echte vraag en heropenen via MENU.

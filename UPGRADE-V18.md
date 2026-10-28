# V18 — kleurrijkere afwerking

## Installeren vanaf V17

1. Pak de V18-ZIP uit.
2. Verbind de Casio met Windows en kies USB Flash / USB-opslag.
3. Vervang `ChatGPT.g3a` in de hoofdmap door `dist\ChatGPT.g3a` uit deze ZIP.
4. Laat de map `CGPT3` staan; daarin staan je chats en scherminstellingen.
5. Werp veilig uit, ontkoppel de Casio-USB en herstart de Casio eenmaal.
6. Open ChatGPT. Instellingen → Meer → Over deze app toont **Versie 18.0**.

**De Atom hoeft vanaf V17 niet opnieuw geüpload te worden.** Alle Atom-broncode
én firmwarebestanden zijn gelijk aan V17. Wifi, API-sleutel en model blijven behouden.

Vanaf V16 of ouder: volg eerst de Atom-update in UPGRADE-V17.md. Gebruik daarbij
de sketchmap of binaries uit deze ZIP, met volledig wissen uitgeschakeld.
Installeer vervolgens de Casio-app uit V18 volgens bovenstaande stappen.

## De vernieuwde opmaak

- Een vaste kleur per instellingenkeuze: teal, blauw, paars en goud.
- Een gekleurde selectierand met lichte achtergrond, in plaats van alleen een streepje.
- Afgeronde kaarten met een subtiele schaduw en herkenbare kleine pictogrammen.
- Een gecentreerd logo en uitnodigende tekst op het lege invoerscherm.
- Tijdens laden tekst onder het logo, passend bij een vraag of het ophalen van modellen.
- De laadpagina vermeldt EXIT/F1 voor annuleren. De teksten claimen geen voortgang
  of resterende tijd die de app niet kan meten.
- Knoppen onderaan hebben eigen kaders; de primaire actie en Terug vallen op.
- Chats en Hulp sluiten aan op dezelfde kaartstijl.

De versie staat uitsluitend in Instellingen. Er zijn geen nieuwe instellingen
of gewijzigde sneltoetsen. EXIT blijft binnen de app; MENU vraagt om afsluiten.
De modellenlijst en de beperkingen van account-/API-ondersteuning zijn zoals in V17.

## Controle

Veertien ASan/UBSan-testgroepen en de Casio-build slagen; de tests omvatten
40 gesimuleerde menuheropeningen, toetsen, antwoorden, modellen en opslag.
De screenshots en GIF gebruiken de echte UI-renderer met hostapparaatstubs,
niet een fysieke Casio of live account.

Controleer op jouw apparaat het beginscherm, de vier kleuren/selectieranden,
het laden bij een vraag en bij modellen, iedere Hulp-pagina, veel typen en
heropenen. Test ook jouw gekozen schermschaal en verschuiving.

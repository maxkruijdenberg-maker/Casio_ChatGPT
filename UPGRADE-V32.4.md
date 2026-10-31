# V32.4 — wifi-controle en controleerbare modelgegevens

Werk voor deze versie zowel de Atom-firmware als de Casio-app bij. Ook Atom V31
moet voor de nieuwe functies naar V32.4. Laat Erase All Flash uitgeschakeld en
bewaar CGPT3 op de Casio. De instellingenopslag en het chatformaat zijn behouden.

1. Pak ChatGPT-V32.4-installatie.zip volledig uit en open START-HIER.html.
2. Flash de meegeleverde Atom-firmware V32.4 volgens de handleiding.
3. Vervang dist/ChatGPT.g3a in de hoofdmap van de Casio en herstart eenmaal.
4. Open Instellingen → Hulp → Verbinding controleren. Je ziet Casio/Atom en wifi apart.
5. Open na een echte vraag Instellingen → Hulp → Gebruikt model voor de
   aangevraagde ID en de model-ID uit het laatste afgeronde API-antwoord op de Atom.

De wifi-test controleert de opgeslagen hotspot en signaalsterkte, geen internet
of API-tegoed. Ze start geen AI-aanvraag. Een al verbonden juiste hotspot blijft
normaal verbonden. EXIT/F1 annuleert; een nieuwe vraag blijft geblokkeerd zolang
stoppen nog niet bevestigd is.

De modelbevestiging staat los van gegenereerde antwoordtekst. Een API-versie-ID
kan afwijken van een alias. Ontbrekende modelmetadata wordt als Niet meegeleverd
getoond. Ze is niet per oud chatbericht opgeslagen en kan na herstart, nieuwe chat
of contextwissel ontbreken. De aanvraag gebruikt de ingestelde model-ID zonder
automatisch naar een ander model uit te wijken.

De functies van V32.3 blijven behouden. Hosttests en firmwarebuilds zijn uitgevoerd;
deze patch is niet op echte Casio/Atom-hardware of met live API-aanvragen getest.

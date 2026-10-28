**Nieuwste versie: [V15](UPGRADE-V15.md), eenvoudig kopersmenu, antwoordweergave en laadlogo.**

# V14 — duidelijke bediening en modelkeuze

Vanaf V11, V12 of V13 vervang je **alleen de Casio-app**. Atom blijft V11.
De door jou stabiel bevonden KEYSC/UART-aanpak van V13 blijft behouden.

## Installeren

1. Pak het V14-pakket volledig uit.
2. Sluit de Casio op Windows aan en kies USB Flash / USB-opslag.
3. Vervang `ChatGPT.g3a` in de hoofdmap door `dist\ChatGPT.g3a` uit V14.
   Laat geen tweede `ChatGPT (1).g3a` staan.
4. Laat `CGPT3` staan: chats en scherminstellingen blijven behouden.
5. Werp veilig uit, ontkoppel Casio-USB en herstart één keer.
6. Laat de Atom via zijn eigen USB gevoed. Open ChatGPT.
7. Instellingen → Versie toont **14.0 / Atom 11.0**.

**Atom niet opnieuw uploaden vanaf V11.** Broncode en alle vier Atom-/bootbestanden
zijn ongewijzigd. Ook de ruwe toetsdecoder, native menu-resolver, iconen en het
UART-herstelbeleid van V13 blijven behouden. Wifi en sleutel staan op de Atom.

## Nieuw in de bediening

| Toets / plaats | Actie |
|---|---|
| F3 in een invoerveld | Voegt direct een echt `=` toe, ook met ALPHA-lock |
| SHIFT, daarna punt | Voegt eveneens `=` toe |
| AC/ON | Invoer wissen na bevestiging; gebruikt de juiste fysieke toets |
| F4 Tekens | Tekenkiezer begint nu met `=` |
| EXIT tijdens een antwoord | Terug naar het vraag-/invoerscherm |
| EXIT op het vraag-/beginscherm | Blijft daar; concept blijft bewaard |
| EXIT in instellingen of een keuzescherm | Eén scherm terug, nooit de app afsluiten |
| MENU | Vraagt vanuit de app om afsluiten |
| F1 Sluiten in die vraag | Naar het echte Casio-menu |
| F2 Blijven of EXIT in die vraag | Doorgaan op hetzelfde scherm met je invoer |
| F6 Instel → F3 Diagnose | Eén centrale verbindingsdiagnose, zonder betaalde vraag |
| F6 Instel → F5 Hulp | Korte ingebouwde startgids |
| F3 bij een opgeslagen antwoord | Toont de volledige oorspronkelijke vraag |

In een nog niet opgeslagen antwoord blijft F3 **Bewaar**. Antwoorden worden bij
normale ontvangst al automatisch lokaal opgeslagen; mislukte opslag wordt gemeld.
EXIT tijdens wachten annuleert de lopende aanvraag, maar sluit de app niet.
Een al verstuurde API-aanvraag kan nog kosten maken; MENU maakt die niet onbetaald.
Niet-verstuurde invoer blijft binnen de app behouden, maar wordt bij werkelijk
sluiten en opnieuw starten niet als concept opgeslagen. Opgeslagen chats blijven.

## Model kiezen

Open F6 Instel → selecteer **Model** → F1 of EXE. Kies met de pijlen en bevestig.
Er hoeft geen modelnaam meer te worden ingetypt. F4 Uitleg geeft toelichting.

| Keuze | Echte modelnaam | Relatieve kosten per token |
|---|---|---|
| Voordelig | `gpt-4.1-nano` | Laag |
| Dagelijks | `gpt-4.1-mini` | Middel |
| Uitgebreid | `gpt-4.1` | Hoger |

Dagelijks is de startkeuze in de lijst bij een onbekend eerder ingesteld model.
Een bestaande aangepaste modelnaam blijft werken en wordt niet automatisch
overschreven: alleen een bevestigde nieuwe keuze wordt op de Atom opgeslagen.
MENU → Blijven behoudt ook de nog niet opgeslagen selectie in de lijst.

Dit zijn **indicaties**, geen actuele prijsopgave of vast bedrag per vraag.
Prijzen veranderen en de werkelijke kosten hangen af van invoer, antwoord en
meegestuurde chatcontext. De officiële prijspagina was tijdens deze build niet
bereikbaar (HTTP 403), dus er worden geen ongecontroleerde bedragen getoond.
De namen komen voor in de officiële OpenAI Python SDK ChatModel-lijst:
https://github.com/openai/openai-python/blob/77fd116d2326bc828bb23d4fd7960142972293a4/src/openai/types/shared/chat_model.py

Deze drie modellen gebruiken de Chat Completions-route van de bestaande Atom.
Werkelijke toegang voor jouw API-account is niet hier getest. De Atom controleert
bij de eerste vraag modeltoegang via een GET vóór de betaalde POST. Een keuze
opslaan of uitleg openen verstuurt geen betaalde vraag.

## Voor een nieuwe koper

Iedere koper gebruikt een **eigen OpenAI API-sleutel met eigen betaald API-tegoed**.
ChatGPT Plus bevat geen API-tegoed. De verkoper hoeft de sleutel niet te kennen.
Zie docs/V14-STARTGIDS.md voor de korte handleiding en docs/V14-TIKTOK.md voor
het demonstratieplan. De telefooninstallatie is hetzelfde als in V11.

## Controle vóór verkoop

De SH4-build en twaalf ASan/UBSan-testgroepen zijn gecontroleerd. De nieuwe tests
dekken `=` via drie routes, MENU/annuleren vanuit invoer en modelkeuze, geheime
invoer bewaren zonder onthullen, EXIT op het beginscherm, alle drie modelkeuzes
via echte transportchunks zonder ASK/ALLOC, en de centrale Diagnose-knop.
De bestaande 40 menu-sessies, toetsen en UART-regressietests blijven slagen.

Nog op echte hardware controleren: alle toetsen, `=`, MENU → Blijven/Sluiten,
EXIT, 20 heropeningen, telefooninstallatie, bewaren van chats, elke aangeboden
modelkeuze met eigen account en ten minste één vraag/antwoord per keuze.
Er is hier geen fysieke Casio/Atom of API-sleutel om dat te bewijzen. De bevestigde
stabiliteit van je eerdere versie is geen hardwaretest van deze nieuwe build.

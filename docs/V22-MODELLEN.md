# V22 — modellen, bron en selectiebeleid

Controle: 29 oktober 2026. Vaste officiële bron:
https://github.com/openai/openai-python/blob/f552fcffcf774520e946121704b5fbbf7ba986e8/src/openai/types/shared/chat_model.py

SHA-256 bronbestand: d2a74c51d4ee5cf1f782fc753c3db15d49cb19f8c3d04cadc4b76775b6ba4167

De officiële ChatModel-enumeratie bevat ook historische namen. V22 controleert
daarom eveneens het blok Deprecated values in dezelfde bron. Alle keuzes in
casio/models.h zijn aanwezig in ChatModel en ontbreken in de uitfaseringslijst.
De 5.3-chat-latest-reeks en verscheidene oude nano/o-modellen zijn inmiddels
uitgeschakeld; andere GPT-4/5-keuzes hebben een aangekondigde einddatum.
Die staan niet meer in de V22-keuzelijst, ook niet als een account ze nog retourneert.

## Elf onderhouden tekst-chatkeuzes

GPT-6 Astra, GPT-6.1 Sol, GPT-6.1 Terra, GPT-6 Sol, GPT-6 Luna,
GPT-5.6 Sol, GPT-5.6 Terra, GPT-5.6 Luna, GPT-5.5, GPT-5.4, GPT-5.4 Mini.

Dit is een bewuste selectie van huidige algemene tekst-chat-aliassen. Geen
uitputtende catalogus van iedere OpenAI-dienst. Datum-snapshots, fine-tunes,
pro/codex/specialistische routes, media, embedding en moderatie worden niet
als chatkeuze aangeboden. Nieuwe onbekende IDs worden niet automatisch goedgekeurd.
Werk de gecontroleerde lijst bij als OpenAI deze namen wijzigt of uitfaseert.

De keuzehulp is appadvies bij het kiezen van een model, geen onafhankelijke
benchmark. Geen exacte snelheid, kwaliteitsrangorde of prijs is afgeleid uit
alleen de naam of GET /v1/models. De officiële modellen- en prijswebpagina's
retourneerden HTTP 403 vanuit deze omgeving, ook na de aanvullende netwerkpoging.
Daarom zijn tarieven expliciet niet bevestigd. De officiële SDK-bron is wel
opgehaald en op bovenstaande revisie en hash gecontroleerd.

## Accountcontrole en compatibiliteit

De bestaande Atom V20.1 blijft alle accountmetadata ophalen via GET /v1/models.
De Casio valideert de volledige transportlijst en toont vervolgens uitsluitend
de doorsnede met de elf onderhouden namen, in dezelfde vaste volgorde als de
keuzelijst. Identieke exacte aliases zijn vereist. Een datum-ID is geen bewijs
dat de bijbehorende algemene alias voor jouw account beschikbaar is.

Geen passende ID resulteert in een lege lijst, met behoud van het opgeslagen
model. Bij geen geladen accountlijst zijn de ingebouwde keuzes herkenbaar als
ongecontroleerd. Een oudere Atom vereist de meegeleverde firmware voor ophalen.

GET metadata of de SDK-enumeratie bewijst geen succesvolle betaalde completion.
De bestaande Atom controleert vóór POST de modeltoegang met GET /v1/models/{id}.
Vragen gebruiken ongewijzigd Chat Completions, max_completion_tokens, platte tekst
en dezelfde ledger tegen dubbele betaalde verzoeken. Geen automatische overstap
naar een andere endpoint of een ander model. Info/vernieuwen/kiezen zijn geen POST.

## Bijwerken en controles

Atom-broncode, firmware, native menu, UART, toetsen en opslag blijven gelijk
aan de vorige release. De bestaande modelinstelling wordt niet stilzwijgend
vervangen. Kies na bijwerken zelf een actueel model.

Hosttests controleren de accountdoorsnede, volgorde, info, alle elf keuzes via
het echte uploadprotocol, veilig lege lijsten, snapshots/retired/media/unknown
uitsluiten, refresh/cancel en behoud van een bestaande instelling. Er is geen
API-sleutel in deze omgeving gebruikt en geen live betaalde vraag getest.

# V22.3 — accountcontrole zonder grote geheugenbuffer

De geheugenfout in V22.2 kwam doordat de volledige accountrespons werd bewaard.
V22.3 vraagt alleen metadata op voor de elf modellen die de app aanbiedt. Alleen
HTTP-status en toegankelijke namen worden gebruikt; JSON-bodies worden niet
opgeslagen. De resultaatbuffer is maximaal 256 bytes. TLS houdt wel werkgeheugen
nodig. De Casio-buffer is verlaagd van 64 naar 4 KiB.

200: naam toegankelijk. 404: deze naam overslaan. Andere HTTP-/netwerkfouten:
hele controle afbreken, geen gedeeltelijke lijst publiceren. Als geen enkel model
toegankelijk is blijft de gecontroleerde lijst leeg. Geen betaalde chatvraag,
automatische modelwissel of wijziging aan de bestaande completion-route.

## Beide onderdelen bijwerken

1. Pak de ZIP volledig uit en open START-HIER.html.
2. Upload atom/ChatGPTAtom/ChatGPTAtom.ino met alle bijbehorende .h-bestanden.
3. Laat Erase All Flash / volledig wissen uitgeschakeld.
4. Vervang dist/ChatGPT.g3a op de Casio, laat CGPT3 staan en herstart de Casio.
5. Test Instellingen → Model → F2 Account. Dit controleert elf namen afzonderlijk
   en kan daardoor langer duren. De app meldt het als de Atom nog moet worden bijgewerkt.

Wifi, API-sleutel, chats, paarsblauwe planeet, Info en MENU-bediening blijven
behouden. Vraag blijft verwijderd; Bewaar verschijnt alleen bij opslagfout.

Tests controleren alle/een deel/geen modellen, fouten onderweg, time-outs en
het begrensde resultaat zonder gedeeltelijke publicatie. Echte hardware en
het API-account van de gebruiker zijn niet in de bouwomgeving getest.

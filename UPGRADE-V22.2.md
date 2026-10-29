# V22.2 — accountlijst lezen en antwoordscherm opruimen

## Werk beide onderdelen bij

Dit is een Atom-firmwarefix plus Casio-update. Alleen het .g3a-bestand vervangen
is onvoldoende. Upload atom/ChatGPTAtom/ChatGPTAtom.ino met alle bijbehorende .h's,
of flash de meegeleverde binaries. Laat Erase All Flash uitgeschakeld. Kopieer
daarna dist/ChatGPT.g3a naar de hoofdmap van de Casio en herstart die eenmaal.
Laat CGPT3 staan. Zie de volledige handleiding voor aansluitingen en uploadstappen.

## Wat is aangepast?

F2 Account kon Antwoord niet leesbaar tonen. Dat is in deze route een fout bij
het lezen van de modellenlijst, niet bij een chatantwoord. De oude Atom gaf de
ruwe HTTP-stream aan de JSON-reader. Bij chunked overdracht bevat die stream ook
HTTP-blokheaders. HTTPClient::writeToStream verwijdert die voordat JSON wordt
gelezen. De vernieuwde Atom gebruikt die route nu ook voor de accountlijst.

De buffer heeft dezelfde expliciete 512-KiB-bovengrens als de oude HTTP-limiet,
wordt alleen gevuld naarmate bytes binnenkomen en stopt veilig bij te weinig
geheugen. Size-, geheugen- en transportfouten worden vóór JSON-verwerking getest.
TLS/HTTP-resources en de ontvangen body worden vóór de gesorteerde resultaatlijst
vrijgegeven. Alleen IDs blijven in het gefilterde JSON-document staan. De volle
modelcatalogus behoudt zijn eerdere grenzen en wordt niet stilzwijgend afgekapt.

Onleesbare accountgegevens krijgen een contextuele accountmelding. De bestaande
vaste lijst blijft beschikbaar met ongecontroleerde toegang; geen oude/halve
accountlijst als succes. Er wordt geen betaalde chatvraag gestart.

De dubbele Vraag-knop is verwijderd uit het antwoordscherm. SHIFT kan nog de
volledige vraag tonen. F3 Bewaar verschijnt uitsluitend bij een opslagprobleem.

## Wat blijft behouden?

Paarsblauwe planeet, modelkeuzes en Info, telefooninstallatie, eigen API-account,
chats, ALPHA/SHIFT en native MENU. De betaalde vraagroute, tokenlimiet, ledger,
UART-pinnen en snelheid veranderen niet. Geen automatische herhaling van POST.

Softwaretests en beide builds moeten slagen. De concrete foutrespons van jouw
API-account was hier niet beschikbaar; er is geen live betaald API-account of
fysieke Casio/Atom getest. Controleer F2 Account opnieuw na beide updates.

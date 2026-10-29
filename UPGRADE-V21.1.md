# V21.1 — chatbalk, statusbolletjes en handleidingen

## Wat verandert

- Groen rond bolletje bij Verbonden; rood rond bolletje bij Verbroken.
- Lege chatbalk met Stuur een bericht, zoals een normale chatinterface.
- De aanduiding is geen echte invoer en wordt niet meegestuurd.
- De balk groeit omhoog tot acht zichtbare regels en krimpt bij wissen.
- Hoogtes van logo, kop, invoer, cursor, hints en functietoetsen zijn op elkaar afgestemd.
- Installatie- en gebruikershandleiding in PDF, offline HTML en bewerkbare tekst.
- Een afzonderlijk klantpakket zonder oude versiegidsen en ontwikkellogs.

## Welke bestanden bijwerken?

Heb je de Atom-update uit V20.1 al geïnstalleerd? Vervang alleen ChatGPT.g3a door
het bestand uit dist, laat CGPT3 staan en herstart de Casio eenmaal.
De Atom-broncode en alle Atom-binaries blijven identiek aan V20.1.

Heb je die Atom-update nog niet gedaan of weet je het niet? Volg de complete
installatie in docs/Handleiding-V21.1.pdf of docs/Handleiding-V21.1.html.
Gebruik alle headers naast de .ino, M5Atom, esp32 core 3.3.0, ArduinoJson 7.4.2
en snelheid 115200. Laat volledig wissen uitgeschakeld om wifi en sleutel te behouden.

Instellingen → Meer → Over deze app toont Versie 21.1. De bestaande V21-download
blijft ongewijzigd; V21.1 bevat ook de volledige modellenlijst uit V20.1.

## Voor levering

De klant kan beginnen bij START-HIER.md. Het overzichtelijke pakket staat in
dist/ChatGPT-V21.1-installatie.zip. De volledige repository bevat daarnaast
historische documentatie en softwaretests.

Softwarechecks en beide meegeleverde firmwareversies zijn gecontroleerd. De
Casio-app wordt nieuw gebouwd; de Atom is bytegelijk aan de gebouwde V20.1-versie.
Veertien ASan/UBSan-groepen controleren ook lege invoer, veldhoogtes, lange tekst,
krimpen na scrollen, de cursorpositie en beide statuskleuren.

Fysieke duurtest, echte wifi/API-toegang en montage zijn hier niet te testen.
Volg docs/V21.1-CONTROLE-VOOR-VERKOOP.md per apparaat voordat je levering vrijgeeft.
Er wordt geen fysieke verkoopklaarheid of algemene modelcompatibiliteit geclaimd.

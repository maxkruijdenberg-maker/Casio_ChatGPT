# ChatGPT voor Casio — V40

Pak het pakket volledig uit en open START-HIER.html of docs/Handleiding-V40.0.pdf.

Werk zowel dist/ChatGPT.g3a als je wifi-module bij. Bewaar CGPT3 op de Casio en wis niet de hele flash.

- ATOM Lite-testbench: py tools/flash_windows.py COM5.
- Rev E ESP32-C6-MINI-1-H4: py tools/flash_c6_windows.py COM5.

Vervang COM5 door je eigen COM-poort. Gebruik de flasher voor jouw apparaat; C6 en ATOM zijn niet uitwisselbaar. Installeer zo nodig eerst esptool: py -m pip install esptool==5.0.0.

Modelkeuze en Verbinding controleren starten elk een kleine betaalde API-test. Actief model wordt alleen bevestigd op basis van API-modelgegevens. Het openen van modelinformatie of het actieve-modelscherm kost geen API-aanvraag.

Klantpakket: dist/ChatGPT-V40.0-installatie.zip. De hardwareproductiebestanden staan apart op GitHub en veranderen niet bij deze software-update.

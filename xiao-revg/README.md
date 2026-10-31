# XIAO-firmware voor Rev G — drie Casio-draden

Alleen voor de **Rev G XIAO-carrier met U6 SN74LVC1G126**. Niet gebruiken op Rev F, Rev E of Atom: die hebben andere pinnen en/of uitgangspolariteit. De Casio V40.1-app is compatibel en zit in de download.

Arduino IDE: ESP32 by Espressif **3.3.0**, ArduinoJson **7.4.2**, bord **XIAO_ESP32C6**, partitie **Huge APP**, USB CDC On Boot **Enabled**. Open de complete map `ChatGPTXIAORevG`, inclusief alle headers. De exacte bordkeuze is vereist voor de Seeed-antenne-initialisatie.

Programmeer de losse XIAO vóór montage. Eerst USB los, daarna solderen. Voor eventueel later USB-onderhoud: batterijstekker J3 los vóór USB aansluiten; USB verwijderen vóór J3 terugplaatsen. Verbind de onderste XIAO-BAT-pads nergens mee.

Rev G: UART1 RX GPIO17, normale TX GPIO16, heartbeat GPIO1, arm GPIO2. Een hardware-buffer schakelt de uitgang naar Casio RX naar hoge impedantie buiten geldig protocolcontact. Opnieuw openen geeft 200us idle-HIGH vóór de eerste startbit. Een lang LOW blijvende RX-lijn zonder data/voortgang kan vanaf 100ms de uitgang sluiten; protocollease 30s, radio/CPU-park na 35s contactverlies. Dit is geen directe fysieke aan/uitmeting.

J4 heeft alleen Casio TX, Casio RX en GND. De batterijvoeding loopt afzonderlijk via J3. Bijbehorende productie- en ontwerpbestanden staan onder `hardware/RevG-XIAO-3wire-06mm-OSP` en `hardware/RevG-XIAO-ontwerp`.

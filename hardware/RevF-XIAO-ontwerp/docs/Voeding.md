# Voeding Rev F

4×AAA -> Q1 ompoolbeveiliging -> TPS63070RNMR -> nominale 4,978 V -> XIAO 5V-pin. De 3V3-uitgang van de XIAO voedt U3/U4. J4-VIO voedt uitsluitend de Casio-RX-pull-up; Casio-VIO is geen extra 5V-ingang. Geen accu of laadcircuit op de carrier.

U1 is de instelbare TPS63070, niet de vaste TPS630701 uit Rev D. R15=200k en R16=38,3k: 0,8 × (1 + 200/38,3) = 4,978 V nominaal. C1/C4 zijn lokale 10uF; de uitgang heeft daarnaast zeven 22uF-condensatoren (164uF nominaal zonder C9). Werkelijke capaciteit bij gelijkspanning en temperatuur moet worden meegenomen bij metingen. L1 is Sunlord SWPA4020S1R5NT, 1,5uH ±30%; het is geen Coilcraft-substitutie met identieke parameters.

R1/R2=422k/100k bewaart de oude pack-UVLO: ongeveer 4,18 V inschakelen / 3,65 V uitschakelen, met component- en IC-spreiding. Vier deels lege AAA-cellen hebben extra bronweerstand; meet opstarten en wifi-pieken samen met de werkende Casio. De drempel beschermt niet iedere afzonderlijke zwakke cel.

Ontwerpbelasting ongeveer 5V/0,5A moet met belastingsstappen en thermisch worden getest. Bij 3,65V, 2,5W en aangenomen 85% rendement is de ingangsstroom ongeveer 0,81A. Dit is geen gemeten efficiëntie of gegarandeerde continue capaciteit. De smalle lokale converteraansluitingen moeten bij de eerste PCBA worden beoordeeld op spanningsval en warmte.

De LM66100 uit Rev D is verwijderd. Er is GEEN USB-batterij-isolatie. De XIAO-5V-pin deelt USB-VBUS. USB en J3 mogen nooit tegelijk aangesloten zijn. Programmeer vooraf los. Voor later onderhoud J3 losnemen, USB aansluiten, USB verwijderen, dan J3 herstellen. De oorspronkelijke Casio-USB-poort is een andere aansluiting.

Firmware behoudt V40.1: wifi uit en CPU op 80MHz na 35 seconden zonder geldig Casio-contact; UART blijft actief voor automatisch herstellen. Geen volledige stroomonderbreking of diepe slaap. Ruststroom moet op de volledige print worden gemeten.

TX: GPIO16 geïnverteerd -> R7 -> Q2/Q3 open drain -> R6 -> Casio RX. GPIO1 hartslag en GPIO2 arm houden de hardware-zendsper alleen tijdens geldige protocolcontacten open. RX: Casio TX -> R4 -> 74LVC1G17 -> GPIO17 en GPIO0. Hiermee wordt de 5V-tolerante ingang gebruikt; geen rechtstreekse hoge Casio-spanning op ESP32-GPIO's.

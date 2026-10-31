# Onderdeelcontrole vóór een bestelling van vijf PCBA's

## Oordeel over de vorige bestanden

**Rev F niet bestellen voor een Casio-aansluiting met alleen RX/TX/GND.** Die versie had een open-drain retouruitgang met een pull-up naar een externe Casio-VIO-aansluiting. Dat paste niet bij de drie draden die je wilt gebruiken. Daarnaast gebruikte het voedings-IC een vereenvoudigd soldeerpatroon. De eerdere schone DRC/softwaretests bewezen die twee ontwerpkeuzes niet.

Rev G corrigeert beide punten. De uitgang is nu een hardware-geschakelde 3V3 push-pull-buffer. Voor voeding en signaaltype sluit dat aan op de werkende ATOM-testopstelling, met RX-beveiliging en de zendsper behouden. Gerber/BOM/CPL en XIAO-firmware zijn vernieuwd als bij elkaar horende Rev G-set.

## Wat zit al op de XIAO, en waarom staat er nog elektronica op de carrier?

| Functie | Beoordeling |
|---|---|
| ESP32-C6, wifi, flash | Alleen op de eigen XIAO; niet nogmaals op de carrier. |
| USB-C en programmeren | Alleen de XIAO-USB. Geen CH340 of tweede programmeeraansluiting. |
| XIAO-3V3-regelaar | Wordt gebruikt. De carrier heeft geen tweede parallelle 3V3-regelaar. |
| U1 + L1: 4×AAA naar ongeveer 5V | Geen doublure: dit maakt een geschikte VBUS-voeding uit de wisselende AAA-spanning. De XIAO maakt daarna zelf 3V3. Vier AAA's horen niet rechtstreeks op de 3,7V-LiPo-pads. |
| XIAO-laadfunctie | Ongebruikt; de onderste BAT-pads blijven los. De AAA's worden niet via de XIAO geladen. Geen aparte carrier-lader. |
| U3: Casio TX naar XIAO RX | Beschermde Schmitt-ingang; dit is geen tweede UART-controller. Ontworpen voor een Casio-TX-signaal binnen 0–5,5V. |
| U6: XIAO TX naar Casio RX | Normale 3V3-uitgang met hoge impedantie wanneer uitgeschakeld. Hiermee vervallen de vierde VIO-draad en Q2/Q3 uit Rev F. |
| U4: hardware-zendsper | Bewuste extra beveiliging bovenop de firmware, geen tweede processor. Blokkeert opstartuitvoer en kan de lijn vrijgeven als de heartbeat wegvalt. Niet nodig om alleen wifi te laten werken, wel behouden voor deze interface. |
| Ontkoppel- en buffercondensatoren | Meerdere condensatoren hebben verschillende plaatsen/functies. De voedingsconverter heeft effectieve capaciteit nodig na gelijkspanningsverlies; het zijn geen willekeurige duplicaten. C16 is verwijderd; U6 krijgt een eigen lokale 100nF C17. |
| Q1 | Ompoolbeveiliging, geen extra voedingsregelaar. |

## Voeding: controle op concrete onderdelen

Het TI-datasheet SLVSC58B van de **TPS63070** is geraadpleegd, niet alleen dat van de vaste TPS630701.

- VIN-bereik 2–16V past bij vier AAA's. De eerder gekozen EN-deler behoudt een nominale startdrempel rond 4,18V en stopdrempel rond 3,65V; werkelijke drempels hebben toleranties.
- PS/SYNC mag aan VIN voor automatische power-save. VSEL aan GND en ongebruikte FB2 open zijn toegestaan. VAUX wordt alleen ontkoppeld, niet als voeding voor andere IC's gebruikt.
- R15=200k en R16=38,3k geven met 0,8V-feedback **4,978V nominaal**. De XIAO wordt op VBUS/5V gevoed; 3V3 wordt alleen uit de XIAO gehaald.
- De lokale 10uF-condensatoren TMK107BBJ106MA-T en uitgangscondensatoren GRM21BC81C226ME44L staan als geschikte typen in TI's voorbeeldselectie.
- De instelbare TPS63070 vraagt effectief minimaal 10uF per uH, niet de strengere factor 15 die eerder voor de vaste variant werd gebruikt. Met 142uF nominaal en aangenomen totaal 80% capaciteitsverlies resteert 28,4uF. Bij 1,95uH (+30%) is dat 14,56uF/uH: boven 10. Dat is een conservatieve berekening, geen gemeten biascurve.
- Sunlord SWPA4020S1R5NT: 1,5uH, opgegeven continue stroom 1,98A en verzadigingsstroom 4,45A. Bij het aangenomen 5V/0,5A-ontwerpdoel, 3,65V ingang, 85% rendement en -30% inductantietolerantie is de berekende spoelpiek ongeveer 1,00A; met 20% marge nog beneden de opgegeven verzadigingsstroom.
- Voedingssporen BAT/VIN/REG5V/3V3 zijn minimaal 0,4mm breed. De berekende somweerstand van alle REG5V-sporen achter elkaar is circa 0,099ohm bij 35um koper en 20°C, dus circa 50mV bij 0,5A. Dat telt zelfs alle aftakkingen mee; via's, retourvlak, temperatuur en echte belasting moeten bij meting worden meegenomen.

## Soldeerpatroon U1

TI-tekening **RNM0015A 4222000/B, maart 2020** beschrijft naast koper ook afzonderlijke soldeermasker- en pasta-openingen. Rev G verwerkt de gedeelde vermogensvlakken voor 7/8 en 12/13, de juiste koperbreedtes en posities voor 9–11, de U-vormige maskeropeningen en de verdeelde pasta-openingen. Binnen de gedeelde koperoppervlakken bewaren kleine interne CAD-pads de aparte logische pinnummers 8 en 13; dit verandert de fysieke koperunie niet. De masker-U-contour is in CAD als polygonale opening vastgelegd.

Deze geometrie is gecontroleerd in `source/component_review.py`. Een onafhankelijk openbaar referentieontwerp is tevens gebruikt voor vergelijking; het fabrikantdatasheet is leidend. Geen ongecontroleerde standaard-QFN-footprint of via-in-pad-proces.

## Drie draden en firmware

J4 heeft daadwerkelijk drie signaalcontacten, geen ongebruikte vierde VIO-positie. Pin 1 gaat naar Casio TX, pin 2 naar Casio RX en pin 3 naar GND. J4 is JST GH 1,25mm, met GHR-03V-S-kabelbehuizing. De horizontale SH-3-code C160403 had in de catalogus slechts twee stuks; daarom is de beschikbare GH-3 C514175 gekozen. De fabrikant is JST, gecontroleerd met de juiste bijbehorende fabrikantentabel uit dezelfde cataloguscommit. Een tabel van een andere catalogusversie kan foutieve fabrikantnamen opleveren.

De firmware gebruikt normale UART-polariteit; Rev F-inversie is uitgeschakeld. De hardware-gate staat bij opstarten uit, gaat pas na geldig HELLO open en krijgt 200us hoogtijd vóór de eerste startbit. Een laag blijvende retourlijn zonder data/protocolvoortgang kan de uitgang vanaf 100ms sluiten; geldige frames, pending UART-data en de 30s-lease voorkomen verkeerde aannamen bij gewone communicatie. De drie-draads aansluiting meet niet rechtstreeks of de Casio fysiek aan of uit is.

## Besteladvies

De nu gevonden incompatibiliteit met jouw drie-draads wens is opgelost in Rev G. De onderdelen hebben een concrete functie, de voeding is getoetst aan het TI-datasheet en de XIAO wordt niet dubbel gevoed vanuit twee 3V3-regelaars. Er is in deze controle geen resterend aangetoond onderdeelconflict gevonden. De resterende onbekenden zijn de werkelijke Casio-ingangsbelasting/signaalmarges, voedingspieken met jouw batterijen en de fysieke assemblage/RF-werking.

Daarmee is Rev G een gecontroleerd **proefontwerp**, geen bewezen verkoopserie. De keuze om €150 aan vijf ongeprobeerde PCBA's uit te geven blijft een prototype-investering. Softwaretests en berekeningen zijn geen meetrapport van die vijf printen. Controleer ook de uiteindelijke JLC-componentmatches en plaatsingspreview vóór betaling; een voorraadmomentopname reserveert geen onderdelen en is geen prijsopgave.

## Bronnen

- [Seeed: XIAO ESP32-C6 officiële handleiding](https://github.com/Seeed-Studio/wiki-documents/blob/98874b6/sites/en/docs/Sensor/SeeedStudio_XIAO/SeeedStudio_XIAO_ESP32C6/XIAO_ESP32C6_Getting_Started.md): VBUS is 5V input/output, 3V3 is output, BAT voor 3,7V LiPo.
- [TI TPS63070 datasheet SLVSC58B, publieke kopie](https://github.com/C7H10N2/TPS63070-Converter/blob/739374ab235b3f08ab6fa69a90cf4c2b3f508a58/Doc/Datasheet/TPS63070RNMR_en.pdf): pinfuncties p3, componentkeuze p16–19, land/mask/stencil p40–41.
- [Onafhankelijke TPS63070-referentie](https://github.com/TobleMiner/tps63070-module/tree/0a591d39f6ed781e6de7a60029fdc62f35148709).
- Openbare JLCparts-catalogus; de nieuwe J4-code en attributentabel zijn samen vastgelegd op commit `be1a7596ec087c35a57ca87087ecd3e7058b5785`. Bronbestanden en hashes staan in `reference/catalog-selected.json`.

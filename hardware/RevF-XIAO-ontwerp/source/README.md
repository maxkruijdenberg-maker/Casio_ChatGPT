# Ontwerpbron Rev F

Het gerouteerde KiCad 9-project in `hardware/` is de beoordeelde bron. `parts.json` bevat de elektrische aansluitingen en plaatsing. De officiële Seeed-DIP-footprint staat bij `reference/` als maatbron; de carrier gebruikt een eigen 14-PTH-patroon zonder SMT-castellations of bottom-BAT-pads.

`build.py` overschrijft de PCB met een ONGEROUTEERDE versie. `schematic.py` maakt schema/symbolen. `route.py` routeert op twee lagen; `finalize_board.py` koppelt de XML-netlist en vult zones. `check_and_clean.py` verwijdert alleen bewezen losse koperstompjes/vias en eist daarna 0 volledige DRC/parity-meldingen. `final_metadata.py` schrijft revisie/printgegevens. `manufacturing.py` audit en exporteert de BOM/CPL. `check_exports.py` controleert daarna de Gerber-centra, boorgaten, contour en productie-aantallen. `previews.py` maakt de schaaltekening en plaatsingspreview.

De catalogusmomentopname in `reference/catalog-selected.json` is de onderbouwing van de BOM. `catalog.py` is alleen de historische extractiestap uit de lokaal gedownloade openbare cataloguscache; die cache en het vorige Rev E-project zijn niet meegeleverd. Dit script is niet nodig voor KiCad of de overige audits.

Benodigd: KiCad 9.0.2 met Python pcbnew, numpy/scipy voor de router, ReportLab voor PDF en Inkscape voor SVG->PNG. De shellscript-cachepaden mogen aan de eigen machine worden aangepast. Voer export.sh pas uit nadat DRC/ERC en BOM/CPL opnieuw gecontroleerd zijn. Gerber-job 'Finish' moet OSP zijn, niet de KiCad-standaard None.

Geen ongecontroleerde generatoruitvoer naar JLC sturen: na een wijziging opnieuw schema, routing, zones, netlistparity, DRC/ERC, audits, previews en exports verwerken. Het afgeronde resultaat van deze controle staat in `reports/`.

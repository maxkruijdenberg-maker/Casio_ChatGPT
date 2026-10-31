# Rev G ontwerpbron

De gerouteerde KiCad 9.0.2 PCB in `hardware/` is de beoordeelde bron. `parts.json` legt netten, plaatsing, MPN en JLC-code vast. Het bevat 39 componenten, waarvan U5 door de gebruiker wordt gemonteerd. BOM/CPL bevatten 38 posities en 20 codes.

`build.py` maakt een ONGEROUTEERDE PCB en bundelt footprints. Het RNM0015A-patroon bevat TI-kopergeometrie, afzonderlijke maskers en verdeelde pasta. Pads 8/13 zijn kleine interne CAD-markers in de gedeelde vlakken 7/8 en 12/13. `schematic.py` maakt het schema. `route.py` routeert twee lagen. `finalize_board.py` verwerkt de kicadxml-netlist en vult zones; `check_and_clean.py` verwijdert uitsluitend bewezen losse stompjes/vias en eist daarna schone DRC/parity. `final_metadata.py` zet de revisie.

Na ontwerpwijzigingen: schema en PCB verwerken, XML-netlist exporteren, finale netkoppeling, DRC/parity en ERC uitvoeren. Daarna `manufacturing.py`, `component_review.py`, `export.sh`, `check_exports.py` en `previews.py` uitvoeren. `component_review.py` accepteert optioneel het repositorypad als argument als het project buiten de repository ligt. Controleer vóór verpakken dat gbrjob Finish=OSP is; standaard KiCad-export zet dit op None.

Benodigd: KiCad 9.0.2 met Python pcbnew, numpy/scipy voor de router, ReportLab voor PDF, Inkscape voor SVG->PNG. Cachepaden in export.sh zijn voor de cloudomgeving en kunnen lokaal worden aangepast. De catalogusreferentie is bevroren; geen automatisch opgeroepen SKU-substitutie. Voor C514175 horen bronbestand en attributentabel bij dezelfde vastgelegde cataloguscommit.

De oude Rev F-genereer- of firmwarebestanden mogen niet over Rev G worden gesynchroniseerd: Rev G heeft een andere uitgangsschakeling, normale TX-polariteit en een RX-lijnbewaking. De Casio-app en gedeelde applicatielogica blijven V40.1.

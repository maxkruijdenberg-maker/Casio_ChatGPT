# V40.1 — rustigere interface en echte controles

1. Pak ChatGPT-V40.1-installatie.zip volledig uit en open START-HIER.html.
2. Werk de firmware bij die bij jouw apparaat hoort: ATOM-testbench of Rev E ESP32-C6. Gebruik geen ATOM-image op de C6-print.
3. Vervang dist/ChatGPT.g3a op de Casio, behoud CGPT3 en herstart de Casio eenmaal.
4. Bij Model staat uitleg op F2. Iedere bevestigde keuze start automatisch een kleine betaalde API-test, zoals door de gebruiker gevraagd. Ook Verbinding controleren start zo’n test; er is geen automatische retry.
5. Actief model toont alleen de modelnaam en API-bevestiging. Zonder passend API-modelveld blijft de keuze onbevestigd.

Volledig wissen is voor deze update niet nodig. De opslagindeling voor instellingen en chats is behouden. Een oude wifi-module biedt de nieuwe volledige controle niet; update beide kanten naar V40. De software is gebouwd en met hosttests gecontroleerd. De nieuwe functies zijn nog niet op echte hardware met een eigen API-account beproefd.

V40.1 corrigeert Info naar F2 en toont bij Model de API-bevestigde modelnaam. De statusbadge staat alleen op het chatbeginscherm. Tekenweergave is uitgebreid voor eindtekens en onzichtbare Unicode. Na circa 35 seconden zonder contact gaat de wifi-radio uit; opnieuw openen hervat automatisch. Werk voor deze energiebesparing ook de wifi-module bij.

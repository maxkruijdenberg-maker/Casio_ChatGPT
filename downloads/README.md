# Publicatie en controle

`manifest.json` beschrijft de klantpakketten en hun vaste downloadtags. Elke tag bevat alleen het uitgepakte pakket, zodat GitHubs normale ZIP-download geen hele ontwikkelrepository met oude versies bevat.

1. Werk de bedoelde bron/binaire bestanden en checksums bij. Voor het standaard V40.1-pakket: `python3 tools/package_customer.py`.
2. Pas bij gewijzigde inhoud de downloadtag in manifest.json aan naar een nieuwe revisie. Bestaande tags worden nooit overschreven.
3. Test: `python3 tests/test_download_publication.py -v`.
4. Publiceer binnen de door de gebruiker gegeven toestemming: `python3 tools/publish_downloads.py publish`.
5. Deel uitsluitend de links die na succesvolle verificatie in DOWNLOADS.md staan. Commit en push de bijgewerkte manifest-, rapport- en overzichtsbestanden.

De controle gebruikt geen GitHub API: gewone Git-uploads en publieke HTTPS-downloads. Er wordt geen Authorization-header voor de download gebruikt. De repository moet aantoonbaar openbaar zijn. Een fout stopt het maken van het downloadverslag en de bevestiging. De GitHub Actions-workflow herhaalt de controle bij wijzigingen aan pakketten of publicatiecode.

De controle bewijst dat de concrete links na publicatie werken en de juiste bestanden leveren. Ze kan latere GitHub-storingen, verwijderde tags of een wijziging van repositoryrechten niet voorkomen. Publiceer daarom nieuwe revisies en behoud de oude downloadtags.

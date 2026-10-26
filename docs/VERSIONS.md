# Gebruikte versies en bronnen

- Arduino CLI 1.3.1 Linux x86_64. Officiële release plus SHA-256-controle:
  https://github.com/arduino/arduino-cli/releases/tag/v1.3.1
- ESP32 Arduino core 3.3.0, board-ID `esp32:esp32:m5stack_atom`, boardnaam
  `M5Atom`, originele ESP32. Officiële core en tool-assets gedownload via de
  Espressif-index, alle SHA-256-waarden gecontroleerd.
  https://github.com/espressif/arduino-esp32/releases/tag/3.3.0
- esp-x32 2411, xtensa-esp-elf GCC 14.2.0_20241119.
- esp32-arduino-libs idf-release_v5.5-b66b5448-v1.
- esptool 5.0.0. Arduino-ctags 5.8-arduino11, SHA-512 gecontroleerd.
- ArduinoJson 7.4.2, commit 733bc4ee82630c88c0a619a883cd3a206efae977:
  https://github.com/bblanchon/ArduinoJson/tree/v7.4.2
- Casio: officiële PrizmSDK 0.6-headers, libfxcg/libc en big-endian libgcc uit
  PrizmSDK-win-0.6.zip; SDK-compiler voor Windows is GCC 10.1.0.
  https://github.com/Jonimoose/libfxcg/releases/tag/v0.6
- Linux-compilatie van Casio-main.c: Debian gcc-14-sh4-linux-gnu
  14.2.0-19cross1 en binutils-sh4-linux-gnu 2.44-3. Gedownload via door APT
  geverifieerde Debian Trixie-indexen. Freestanding, SH4A no-FPU, big-endian,
  Hitachi-conventie, leading underscores; daarna gelinkt met SDK 0.6-libraries.
  De binary heeft de originele SDK-startup op adres 0x00300000; linkerkaart
  inbegrepen. Voor de definitieve test is echt Casio-hardware nodig.
- mkg3a git 502381ee402d8435126be3ac651ad9b42db26d33 (GitLab taricorp/mkg3a).
  Gebouwd met BMP-ondersteuning; eigen iconen 92x64 pixels.
  https://gitlab.com/taricorp/mkg3a
- DejaVu Sans Mono 13px-raster, 177 expliciete glyphs (V2); generator inbegrepen.
  Zie FONT-LICENSE.txt. Geen automatische aanname over Casio-codepagina's.
- OpenAI API-specificatie opgehaald van de officiële repository tijdens bouw:
  https://github.com/openai/openai-openapi/blob/master/openapi.yaml
  API-referentie: https://platform.openai.com/docs/api-reference/chat/create
  Modelnaam standaard `gpt-4.1-mini`, wijzigbaar. Beschikbaarheid voor een
  concreet account is niet getest: de Atom doet zelf een geauthenticeerde
  modelcontrole voordat een betaalde completion start.
- TLS-ankers: originele Mozilla-certificaten uit Debian ca-certificates,
  zonder uitschakeling van verificatie of vervanging van checksums.

Bron- en licentievermeldingen van libfxcg, mkg3a, ArduinoJson en het lettertype
staan in deze map. De ESP32-core en vooraf gebouwde Espressif-bibliotheken
behouden hun upstreamlicenties: https://github.com/espressif/arduino-esp32 en
https://github.com/espressif/esp-idf . De volledige toepassing en de benodigde
Arduino-bronbestanden zijn meegeleverd zodat je de firmware opnieuw kunt bouwen.

`dist/sha256.json` bevat hashes van de geleverde binaire bestanden. Een hash
bevestigt bestandsgelijkheid; hij bewijst geen fysieke werking.

## V2.0 changes

Runtime toolchain versions are unchanged. Firmware and add-in report V2.0;
wire framing remains protocol v1 with additive W_GET_CONFIG/W_DEVICE commands.
The shared Unicode helper is in atom/ChatGPTAtom/utf8.h. The font now contains
177 glyphs (middle dot added); tools/make_font.py no longer overwrites the
menu icons. tools/make_assets.py builds those from assets/openai.svg using
CairoSVG and Pillow on the build machine. Their versions are recorded in
`docs/logs/v2-assets.txt`; neither is needed to flash prebuilt files.

The C++ host API tests use the same ArduinoJson 7.4.2 headers as the firmware.
GCC emits a may-be-uninitialized warning for an upstream ArduinoJson iterator
in the sanitized host build; tests finish successfully without sanitizer
errors. The ESP32 target build also completes successfully.

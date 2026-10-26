#pragma once
/* Original Atom Lite only. A future C6 board requires a separate hardware port. */
#if !defined(CONFIG_IDF_TARGET_ESP32) || !CONFIG_IDF_TARGET_ESP32
#error "Select M5Atom with original ESP32-PICO-D4, not AtomS3 or ESP32-C6."
#endif
static const int RX_PIN=32;
static const int TX_PIN=26;

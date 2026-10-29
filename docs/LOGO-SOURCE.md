# Logo asset

`assets/openai.svg` is the OpenAI knot from Simple Icons version 13.0.0:
https://github.com/simple-icons/simple-icons/blob/13.0.0/icons/openai.svg

The icon artwork is distributed by Simple Icons under CC0-1.0. This does not
transfer trademark rights. This is an independent Casio add-in using the OpenAI
API, not an official OpenAI or Casio product. Do not suggest endorsement.

`tools/make_assets.py` produces the monochrome in-app masks and the 92x64 BMP
menu icons. The bitmap font and logo are rasterized assets; there is no SVG
renderer on the calculator. These assets do not change the Casio system menu's
layout or the icons of any other application.

V3 removes all bitmap text and selection decorations. Both menu bitmaps are
white with a black logo, leaving the native Casio gray/black label to the OS.
This fixes the duplicate text; it does not recolor the rest of the system menu.

## V21 in-app refinement

`tools/make_logo_aa.py` generates `casio/logo_aa.h` from the same pinned SVG.
Native 16/24/40/48/56/64 pixel masks use 4-bit alpha coverage; the Casio renderer
blends edges over the current RGB565 framebuffer. The large mark receives a
mint-to-lilac tint and a decorative rim. The underlying knot source and trademark
attribution remain unchanged. The native-menu BMP icons are not regenerated.

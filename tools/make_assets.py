"""Rebuild in-app logo masks and blank 92x64 menu icons from the pinned SVG source."""
from pathlib import Path
from io import BytesIO
import sys
from PIL import Image,ImageDraw,ImageFont
try: import cairosvg
except ImportError:
 sys.path.insert(0,'/workspace/casio-tools/python');import cairosvg
root=Path(__file__).resolve().parents[1]
svg=(root/'assets/openai.svg').read_bytes().replace(b'<path ',b'<path fill="white" ')
def logo(size):
 return Image.open(BytesIO(cairosvg.svg2png(bytestring=svg,output_width=size*4,output_height=size*4))).convert('RGBA').resize((size,size),Image.Resampling.LANCZOS)
rows=['/* Generated from assets/openai.svg; see docs/LOGO-SOURCE.md. */','#ifndef CHATGPT_LOGO_H','#define CHATGPT_LOGO_H','#include <stdint.h>']
for size in (16,24,40):
 im=logo(size);data=[int(im.getpixel((x,y))[3]>=128) for y in range(size) for x in range(size)]
 packed=[sum((data[i+j] if i+j<len(data) else 0)<<(7-j) for j in range(8)) for i in range(0,len(data),8)]
 rows += [f'static const uint8_t logo{size}[]={{',','.join('0x%02x'%v for v in packed),'};']
rows+=['#endif'];(root/'casio/logo.h').write_text('\n'.join(rows)+'\n')
# Blank menu tiles only; the in-app logo masks above remain unchanged.
# The native OS may still draw focus or shortcut overlays outside these pixels.
for name in ('unselected','selected'):
 Image.new('RGB',(92,64),'white').save(root/f'casio/{name}.bmp')
print('V11 blank menu tiles: both states white, no logo, badge or painted focus.')

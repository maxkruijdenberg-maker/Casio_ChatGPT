"""Rebuild V6 logo masks and 92x64 menu icons from the pinned SVG source."""
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
# No rasterized label: the OS draws its own single gray/black app name.
black_svg=(root/'assets/openai.svg').read_bytes()
for name in ('unselected','selected'):
 im=Image.new('RGB',(92,64),'white')
 if name=='selected':
  # Match the requested CG50 style: rectangular blue outline, white interior.
  # Same 92x64 bitmap and logo placement as the unselected tile; no shadow,
  # rounded corners, gradient or baked-in app name.
  ImageDraw.Draw(im).rectangle((0,0,91,63),outline=(82,198,247),width=3)
 mark=Image.open(BytesIO(cairosvg.svg2png(bytestring=black_svg,output_width=176,output_height=176))).convert('RGBA').resize((44,44),Image.Resampling.LANCZOS)
 im.paste(mark,(24,4),mark)
 im.save(root/f'casio/{name}.bmp')
print('V6 logo masks and label-free normal/selected menu tiles generated.')

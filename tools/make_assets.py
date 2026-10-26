"""Rebuild V8 logo masks and 92x64 menu icons from the pinned SVG source."""
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
# A native menu tile includes the app-name area at its bottom. Keep all logo
# pixels above y=37, leaving y=40..63 for the OS label and its focus drawing.
# Never rasterize a second name or try to choose its own text size.
for name in ('unselected','selected'):
 im=Image.new('RGB',(92,64),(82,198,247) if name=='selected' else 'white')
 # Standard tile composition: white rectangular logo panel, blue surrounding
 # selection including the native label area. No blue behind the black mark.
 ImageDraw.Draw(im).rectangle((4,2,87,39),fill='white')
 mark=Image.open(BytesIO(cairosvg.svg2png(bytestring=black_svg,output_width=128,output_height=128))).convert('RGBA').resize((32,32),Image.Resampling.LANCZOS)
 im.paste(mark,(30,4),mark)
 im.save(root/f'casio/{name}.bmp')
print('V8 menu tiles: white logo panel, blue selection surround, 32px logo above native label area.')

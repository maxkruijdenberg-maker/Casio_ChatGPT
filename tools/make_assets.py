"""Rebuild V2 logo masks and 92x64 menu icons from the pinned SVG source."""
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
font=ImageFont.truetype('/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf',11)
for selected,name in [(False,'unselected'),(True,'selected')]:
 im=Image.new('RGB',(92,64),'#171717');d=ImageDraw.Draw(im)
 if selected:
  d.rounded_rectangle((0,0,91,63),radius=6,outline='#74e7bf',width=2)
  d.rectangle((2,48,89,61),fill='#74e7bf')
 im.paste(logo(40),(26,4),logo(40))
 d.text((46,53),'ChatGPT',font=font,anchor='mm',fill='#171717' if selected else '#ececec')
 im.save(root/f'casio/{name}.bmp')
print('V2 logo masks and menu icons generated.')

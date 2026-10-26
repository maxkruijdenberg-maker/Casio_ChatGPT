from pathlib import Path
from PIL import Image,ImageDraw,ImageFont
root=Path(__file__).resolve().parents[1]
font=ImageFont.truetype('/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf',13)
chars=''.join(chr(i) for i in range(32,127))+'éëèêáàâäíïìîóöòôúüùûçñÉËÈÊÁÀÂÄÍÏÌÎÓÖÒÔÚÜÙÛÇÑπθαβγδμωφ∞√≤≥≠×÷±∑∫∆→←↑↓²³°•…‘’“”–—€�'
chars=''.join(dict.fromkeys(chars))
rows=['/* Generated DejaVu Sans Mono 13px. See docs/FONT-LICENSE.txt. */','#ifndef CHATGPT_FONT_H','#define CHATGPT_FONT_H','#include <stdint.h>','typedef struct {uint32_t code;uint8_t rows[14];} Glyph;','static const Glyph font_glyphs[]={']
for ch in chars:
 im=Image.new('1',(8,14));ImageDraw.Draw(im).text((0,-1),ch,font=font,fill=1)
 data=[sum((1<<(7-x)) if im.getpixel((x,y)) else 0 for x in range(8)) for y in range(14)]
 rows.append('{0x%x,{%s}},'%(ord(ch),','.join('0x%02x'%x for x in data)))
rows.extend(['};','#define FONT_COUNT (sizeof(font_glyphs)/sizeof(font_glyphs[0]))','#endif'])
(root/'casio/font.h').write_text('\n'.join(rows)+'\n')
(root/'docs/FONT-LICENSE.txt').write_text(Path('/usr/share/doc/fonts-dejavu-core/copyright').read_text())
# Original chat-bubble icons; BMP is supported by mkg3a without libpng.
for selected in [False,True]:
 im=Image.new('RGB',(92,64),'#15253b' if selected else 'white');d=ImageDraw.Draw(im)
 d.rounded_rectangle((8,4,84,48),radius=10,fill='#10a37f');d.polygon([(20,46),(20,58),(38,46)],fill='#10a37f')
 d.text((21,16),'GPT',font=ImageFont.truetype('/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf',23),fill='white')
 im.save(root/'casio'/('selected.bmp' if selected else 'unselected.bmp'))
print(len(chars),'glyphs generated')

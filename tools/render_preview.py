"""Render actual V8 framebuffer fixtures (host tests, not physical devices)."""
from pathlib import Path
import subprocess,tempfile
from PIL import Image,ImageDraw,ImageFont
root=Path(__file__).resolve().parents[1]
runner=root/'build/tests/test_ui'
if not runner.exists():raise SystemExit('Run sh tools/test.sh first.')
out=root/'docs/design';out.mkdir(exist_ok=True)
font=ImageFont.truetype('/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf',19)
board=Image.new('RGB',(1224,1484),'#f2f2f2');d=ImageDraw.Draw(board)
d.text((24,16),'ChatGPT V8 · Werkelijke UI-renderer (hosttest, geen apparaatfoto)',font=font,fill='#212121')
items=[('input','Vraag typen',24,62),('answer','Antwoord lezen',636,62),('sidebar','Zijbalk met lokale chats',24,436),('settings','Instellingen',636,436),('calibration','Zelf schaal en positie instellen',24,810),('screen','Volledig scherm controleren',636,810)]
with tempfile.TemporaryDirectory() as temp:
 subprocess.run([str(runner),'--screens',temp],check=True)
 for name,label,x,y in items:
  im=Image.open(Path(temp)/(name+'.ppm'));im.save(out/('V8-'+name+'.png'))
  d.text((x,y),label,font=font,fill='#212121');board.paste(im.resize((576,324),Image.Resampling.NEAREST),(x,y+28))
d.text((24,1198),'Menu: witte basis · wit logo-paneel; blauwe selectie eromheen · naamtekst door Casio',font=font,fill='#212121')
for name,label,x in [('unselected','Normaal',24),('selected','Geselecteerd',360)]:
 im=Image.open(root/f'casio/{name}.bmp');board.paste(im.resize((276,192),Image.Resampling.NEAREST),(x,1230));d.text((x,1430),label,font=font,fill='#212121')
board.save(out/'V8-actual-screens.png');print(out/'V8-actual-screens.png')

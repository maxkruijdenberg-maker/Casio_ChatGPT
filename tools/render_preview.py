"""Render actual V15 framebuffer fixtures (host tests, not physical devices)."""
from pathlib import Path
import subprocess,tempfile
from PIL import Image,ImageDraw,ImageFont
root=Path(__file__).resolve().parents[1]
runner=root/'build/tests/test_ui'
if not runner.exists():raise SystemExit('Run sh tools/test.sh first.')
out=root/'docs/design';out.mkdir(exist_ok=True)
font=ImageFont.truetype('/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf',19)
board=Image.new('RGB',(1224,1484),'#f2f2f2');d=ImageDraw.Draw(board)
d.text((24,16),'ChatGPT V15 · Werkelijke UI-renderer (hosttest, geen apparaatfoto)',font=font,fill='#212121')
items=[('input','Vraag typen',24,62),('answer','Antwoord lezen',636,62),('settings','Instellingen voor kopers',24,436),('models','Model kiezen',636,436),('loading','Laden zonder technische status',24,810),('diagnosis','Verbindingscontrole in gewone taal',636,810)]
with tempfile.TemporaryDirectory() as temp:
 subprocess.run([str(runner),'--screens',temp],check=True)
 for name,label,x,y in items:
  im=Image.open(Path(temp)/(name+'.ppm'));im.save(out/('V15-'+name+'.png'))
  d.text((x,y),label,font=font,fill='#212121');board.paste(im.resize((576,324),Image.Resampling.NEAREST),(x,y+28))
 for extra in ['calibration','screen','sidebar','phone']+[f'loading-{i}' for i in range(8)]:
  Image.open(Path(temp)/(extra+'.ppm')).save(out/('V15-'+extra+'.png'))
d.text((24,1198),'Menu: witte basis · lager afgerond wit vlak; J-badge; blauwe selectie · naamtekst door Casio',font=font,fill='#212121')
for name,label,x in [('unselected','Normaal',24),('selected','Geselecteerd',360)]:
 im=Image.open(root/f'casio/{name}.bmp');board.paste(im.resize((276,192),Image.Resampling.NEAREST),(x,1230));d.text((x,1430),label,font=font,fill='#212121')
board.save(out/'V15-actual-screens.png');print(out/'V15-actual-screens.png')

"""Render actual V2 framebuffer fixtures (not photographs and not live API)."""
from pathlib import Path
import subprocess,tempfile
from PIL import Image,ImageDraw,ImageFont
root=Path(__file__).resolve().parents[1]
runner=root/'build/tests/test_ui'
if not runner.exists():raise SystemExit('Run sh tools/test.sh first to build the actual UI host test.')
out=root/'docs/design';out.mkdir(exist_ok=True)
font=ImageFont.truetype('/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf',19)
board=Image.new('RGB',(1224,1090),'#f2f2f2');d=ImageDraw.Draw(board)
d.text((24,16),'ChatGPT V2 · Werkelijke UI-renderer (hosttest, geen foto van hardware)',font=font,fill='#212121')
with tempfile.TemporaryDirectory() as temp:
 subprocess.run([str(runner),'--screens',temp],check=True)
 for name,label,x,y in [('input','Vraag typen',24,62),('answer','Antwoord lezen',636,62),('settings','Instellingen',24,436),('offline','Instellingen zonder Atom',636,436)]:
  im=Image.open(Path(temp)/(name+'.ppm'));im.save(out/('V2-'+name+'.png'))
  d.text((x,y),label,font=font,fill='#212121');board.paste(im.resize((576,324),Image.Resampling.NEAREST),(x,y+28))
d.text((24,824),'Eigen ChatGPT-menu-iconen (92 × 64 pixels)',font=font,fill='#212121')
for name,label,x in [('unselected','Normaal',24),('selected','Geselecteerd',360)]:
 im=Image.open(root/f'casio/{name}.bmp');board.paste(im.resize((276,192),Image.Resampling.NEAREST),(x,854))
 d.text((x,1052),label,font=font,fill='#212121')
board.save(out/'V2-actual-screens.png')
print(out/'V2-actual-screens.png')

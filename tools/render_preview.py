"""Render actual V32.3 framebuffer fixtures (host tests, not physical devices)."""
from pathlib import Path
import subprocess,tempfile
from PIL import Image,ImageDraw,ImageFont
root=Path(__file__).resolve().parents[1]
runner=root/'build/tests/test_ui'
if not runner.exists():raise SystemExit('Run sh tools/test.sh first.')
out=root/'docs/design';out.mkdir(exist_ok=True)
font=ImageFont.truetype('/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf',19)
board=Image.new('RGB',(1224,1484),'#f2f2f2');d=ImageDraw.Draw(board)
d.text((24,16),'ChatGPT V32.3 · Werkelijke UI-renderer (hosttest, geen apparaatfoto)',font=font,fill='#212121')
items=[('home','Chatbalk en groen statusbolletje',24,62),('answer','Doorlopende chat met invoerbalk',636,62),('settings','Instellingen voor kopers',24,436),('models','Model kiezen',636,436),('loading','Symmetrische planeetring',24,810),('help','Hulp zonder technische instellingen',636,810)]
with tempfile.TemporaryDirectory() as temp:
 subprocess.run([str(runner),'--screens',temp],check=True)
 for name,label,x,y in items:
  im=Image.open(Path(temp)/(name+'.ppm'));im.save(out/('V32.3-'+name+'.png'))
  d.text((x,y),label,font=font,fill='#212121');board.paste(im.resize((576,324),Image.Resampling.NEAREST),(x,y+28))
 for extra in ['input','calibration','screen','sidebar','phone','diagnosis','more','all-models','models-recommended','answer-start','shift-math','account-success','account-failure','models-stopping','model-info','characters','characters-space','conversation-up','conversation-draft','math-answer','models-unchecked','input-long','online','offline']+[f'loading-{i}' for i in range(8)]:
  Image.open(Path(temp)/(extra+'.ppm')).save(out/('V32.3-'+extra+'.png'))
d.text((24,1198),'Menu: witte basis · lager afgerond wit vlak; J-badge; blauwe selectie · naamtekst door Casio',font=font,fill='#212121')
for name,label,x in [('unselected','Normaal',24),('selected','Geselecteerd',360)]:
 im=Image.open(root/f'casio/{name}.bmp');board.paste(im.resize((276,192),Image.Resampling.NEAREST),(x,1230));d.text((x,1430),label,font=font,fill='#212121')
board.save(out/'V32.3-actual-screens.png');print(out/'V32.3-actual-screens.png')

frames=[Image.open(out/f'V32.3-loading-{i}.png').convert('RGB').resize((768,432),Image.Resampling.NEAREST) for i in range(8)]
frames[0].save(out/'V32.3-loading.gif',save_all=True,append_images=frames[1:],duration=120,loop=0)

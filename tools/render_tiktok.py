"""Create a 20s vertical storyboard from actual host UI fixtures, not a hardware demo."""
from pathlib import Path
import subprocess
from PIL import Image, ImageDraw, ImageFont
root=Path(__file__).resolve().parents[1]
out=root/'docs/design';work=root/'build/tiktok';work.mkdir(parents=True,exist_ok=True)
font_dir=Path('/usr/share/fonts/truetype/dejavu')
def font(size,bold=False):return ImageFont.truetype(str(font_dir/('DejaVuSans-Bold.ttf' if bold else 'DejaVuSans.ttf')),size)
slides=[
 ('input','Je vraag.\nOp je Casio.','Uitleg voor je volgende oefensessie.'),
 ('answer','Stap voor stap.\nVraag gerust door.','Van formule naar uitleg, met je eigen vraag.'),
 ('models','Kies je model.\nHelder geregeld.','Drie keuzes, met naam en kostenindicatie.'),
 ('sidebar','Je chats.\nLees rustig terug.','Je recente gesprekken lokaal bewaard.'),
 ('phone','Instellen?\nPak je telefoon.','Wifi invullen. Je eigen API-sleutel plakken.')]
paths=[]
for i,(screen,title,sub) in enumerate(slides):
 im=Image.new('RGB',(1080,1920),'#0e1512');d=ImageDraw.Draw(im)
 d.rounded_rectangle((60,65,575,124),radius=25,fill='#203b30')
 d.text((82,77),'INTERFACEPREVIEW / V14',font=font(29,True),fill='#b1f2d7')
 d.multiline_text((60,180),title,font=font(84,True),fill='#f1f6f3',spacing=20)
 d.text((64,420),sub,font=font(35),fill='#b1c6ba')
 d.rounded_rectangle((44,600,1036,1212),radius=30,fill='#2b3b32')
 frame=Image.open(out/f'V14-{screen}.png').convert('RGB').resize((960,540),Image.Resampling.NEAREST)
 im.paste(frame,(60,636))
 d.text((64,1275),'Vraag. Begrijp. Oefen.',font=font(53,True),fill='#b1f2d7')
 d.text((64,1390),'Voor de Casio fx-CG50',font=font(38),fill='#edf4ef')
 d.text((64,1450),'met een aangesloten Atom Lite.',font=font(36),fill='#b1c6ba')
 d.line((64,1620,1016,1620),fill='#30443a',width=2)
 for y,text in enumerate(['Eigen API-account, betaald tegoed en wifi nodig.',
                          'AI kan fouten maken. Controleer de uitleg.',
                          'Interfacepreview: geen fysieke apparaatopname.']):
  d.text((64,1660+50*y),text,font=font(27),fill='#9db3a6')
 d.text((64,1850),'Onafhankelijk project',font=font(24),fill='#7f9a8b')
 path=work/f'frame-{i}.png';im.save(path);paths.append(path)
 if i==0:im.save(out/'V14-tiktok-cover.png')
concat=work/'slides.txt'
concat.write_text(''.join("file '"+str(p)+"'\nduration 4\n" for p in paths)+"file '"+str(paths[-1])+"'\n")
subprocess.run(['ffmpeg','-hide_banner','-loglevel','error','-y','-f','concat','-safe','0','-i',str(concat),
 '-t','20','-r','30','-c:v','libx264','-preset','fast','-crf','25','-pix_fmt','yuv420p','-threads','2',
 '-movflags','+faststart',str(out/'V14-tiktok-preview.mp4')],check=True)
print(out/'V14-tiktok-preview.mp4')

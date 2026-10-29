"""Build offline customer HTML/PDF from the current Dutch manual and real UI fixture."""
from pathlib import Path
import re,html,base64
from reportlab.platypus import SimpleDocTemplate,Paragraph,Spacer,Image,KeepTogether
from reportlab.lib.styles import ParagraphStyle
from reportlab.lib.colors import HexColor
from reportlab.pdfbase import pdfmetrics
from reportlab.pdfbase.ttfonts import TTFont
root=Path(__file__).resolve().parents[1]
version=re.search(r'Versie ([0-9.]+)',(root/'casio/main.c').read_text()).group(1)
source=root/f'docs/V{version}-HANDLEIDING.md'
blocks=[];pending=[];kind='p'
def flush():
 global pending
 if pending:blocks.append((kind,' '.join(pending)));pending=[]
for line in source.read_text().splitlines():
 if not line.strip():flush();kind='p';continue
 if line.startswith('#'):
  flush();n=len(line)-len(line.lstrip('#'));blocks.append((f'h{n}',line[n:].strip()));kind='p'
 elif line.startswith('- ') or re.match(r'^\d+\. ',line):
  flush();kind='bullet' if line.startswith('- ') else 'step';pending=[line[2:] if kind=='bullet' else line]
 else:pending.append(line.strip())
flush()
image_path=root/f'docs/design/V{version}-home.png'
image_data=base64.b64encode(image_path.read_bytes()).decode()
def linked(text,pdf=False):
 safe=html.escape(text)
 def replace(match):
  raw=match.group(0);url=raw.rstrip('.,;');tail=raw[len(url):]
  return ('<link href="'+url+'">'+url+'</link>' if pdf else '<a href="'+url+'">'+url+'</a>')+tail
 return re.sub(r'https?://[^\s<]+',replace,safe)
parts=[]
for i,(k,text) in enumerate(blocks):
 safe=linked(text)
 if k in ('h1','h2','h3'):parts.append(f'<{k}>{safe}</{k}>')
 else:parts.append(f'<p class="{k}">{"• " if k=="bullet" else ""}{safe}</p>')
 if i==1:parts.append(f'<img alt="Voorbeeld van het beginscherm" src="data:image/png;base64,{image_data}"><p class="caption">Softwareweergave met voorbeeldgegevens.</p>')
css='''body{margin:0;background:#0d1422;color:#eef4fc;font:17px/1.65 system-ui,Arial,sans-serif}main{max-width:850px;margin:40px auto;padding:28px;background:#172235;border-radius:20px}h1{color:#6be3c1;font-size:34px;line-height:1.2}h2{color:#c0a7ff;margin-top:2em;font-size:24px}p{margin:12px 0}.step,.bullet{padding-left:16px}.caption{font-size:13px;color:#a7b4c9}img{display:block;max-width:100%;width:576px;margin:25px auto;border-radius:12px}a{color:#6be3c1}@media print{body,main{background:white;color:#172235}main{margin:0;padding:0;max-width:none}h1,h2{color:#173f55}.caption{color:#4d5868}h2{break-after:avoid}img{width:384px}}'''
(root/f'docs/Handleiding-V{version}.html').write_text('<!doctype html><html lang="nl"><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>Handleiding V'+version+'</title><style>'+css+'</style><main>'+''.join(parts)+'</main></html>')
fonts=Path('/usr/share/fonts/truetype/dejavu')
pdfmetrics.registerFont(TTFont('Guide',str(fonts/'DejaVuSans.ttf')))
pdfmetrics.registerFont(TTFont('GuideBold',str(fonts/'DejaVuSans-Bold.ttf')))
styles={
 'p':ParagraphStyle('body',fontName='Guide',fontSize=10,leading=15,spaceAfter=7,textColor=HexColor('#172235')),
 'h1':ParagraphStyle('title',fontName='GuideBold',fontSize=24,leading=30,spaceAfter=13,textColor=HexColor('#164c4b')),
 'h2':ParagraphStyle('section',fontName='GuideBold',fontSize=13,leading=18,spaceBefore=13,spaceAfter=7,keepWithNext=True,textColor=HexColor('#3b3868')),
 'h3':ParagraphStyle('subsection',fontName='GuideBold',fontSize=11,leading=16,spaceBefore=10,spaceAfter=6,keepWithNext=True),
}
styles['bullet']=ParagraphStyle('bullet',parent=styles['p'],leftIndent=10)
styles['step']=ParagraphStyle('step',parent=styles['p'],leftIndent=10)
story=[]
for i,(k,text) in enumerate(blocks):
 story.append(Paragraph(('• ' if k=='bullet' else '')+linked(text,True),styles[k]))
 if i==1:
  story.append(KeepTogether([Spacer(1,8),Image(str(image_path),width=288,height=162),Paragraph('Softwareweergave met voorbeeldgegevens.',styles['p']),Spacer(1,7)]))
def furniture(canvas,doc):
 canvas.saveState();canvas.setStrokeColor(HexColor('#dce4ec'));canvas.line(42,36,553,36)
 canvas.setFillColor(HexColor('#536475'));canvas.setFont('Guide',8)
 canvas.drawString(42,24,'ChatGPT voor Casio · V'+version+' · Eigen API-account vereist')
 canvas.drawRightString(553,24,str(doc.page));canvas.restoreState()
pdf_path=root/f'docs/Handleiding-V{version}.pdf'
doc=SimpleDocTemplate(str(pdf_path),pagesize=(595.28,841.89),rightMargin=42,leftMargin=42,topMargin=36,bottomMargin=48,title='ChatGPT voor Casio — Handleiding V'+version,author='Casio ChatGPT project')
doc.build(story,onFirstPage=furniture,onLaterPages=furniture)
print(f'PDF and offline HTML: V{version}, {doc.page} pages')

start_html='<!doctype html><html lang="nl"><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>Start — ChatGPT V'+version+'</title><style>'+css+'a.button{display:block;padding:15px 20px;margin:15px 0;border:1px solid #6be3c1;border-radius:12px;text-decoration:none}a.button:first-of-type{background:#6be3c1;color:#0d1422}</style><main><h1>ChatGPT voor Casio</h1><p>V'+version+' · Installeren en gebruiken</p><p>Pak de ZIP eerst volledig uit. Deze handleidingen werken daarna offline.</p><a class="button" href="docs/Handleiding-V'+version+'.html">Open de handleiding</a><a class="button" href="docs/Handleiding-V'+version+'.pdf">Open de PDF-handleiding</a><h2>Al eerder geïnstalleerd?</h2><p><strong>Werk bij deze versie ook de Atom-firmware bij.</strong> Alleen ChatGPT.g3a vervangen verhelpt de accountlijstfout niet. Volg eerst de Atom-installatie en daarna de Casio-installatie in de handleiding.</p><p>Laat volledig wissen uitgeschakeld en CGPT3 staan. Zo blijven normaal wifi, sleutel, chats en scherminstellingen bewaard. Je hebt eigen OpenAI-API-toegang en tegoed nodig.</p></main></html>'
(root/'START-HIER.html').write_text(start_html)

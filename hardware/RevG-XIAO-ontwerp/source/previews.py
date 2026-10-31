from pathlib import Path
import pcbnew as p,html
from reportlab.pdfgen import canvas
from reportlab.lib.pagesizes import landscape,A4
from reportlab.lib.units import mm
r=Path(__file__).resolve().parents[1];b=p.LoadBoard(str(r/'hardware/CG50-WiFi.kicad_pcb'));xy=lambda a:(p.ToMM(a.x),p.ToMM(a.y));outline=[(0,0),(89,0),(89,19),(34,19),(34,34),(0,34)]
s=['<svg xmlns="http://www.w3.org/2000/svg" viewBox="-5 -10 99 55" width="1584" height="880">','<rect x="-5" y="-10" width="99" height="55" fill="#f0f4fa"/>','<text x="0" y="-5" font-family="sans-serif" font-size="2.5" fill="#17324a">REV G · XIAO ESP32-C6 zelf solderen</text>','<polygon points="'+' '.join(f'{x},{y}' for x,y in outline)+'" fill="#14544d" stroke="#12342e" stroke-width=".2"/>','<rect x="10.6" y="0" width="4.9" height="14.5" fill="#c9a84c" opacity=".45"/>']
for f in b.GetFootprints():
 for g in f.GraphicalItems():
  if isinstance(g,p.PCB_SHAPE) and g.GetLayer()==p.F_Fab and g.GetShape()==p.SHAPE_T_SEGMENT:
   x,y=xy(g.GetStart());xx,yy=xy(g.GetEnd());s.append(f'<path d="M{x},{y} L{xx},{yy}" stroke="#dceae8" stroke-width=".12" fill="none"/>')
 for a in f.Pads():
  x,y=xy(a.GetPosition());sx,sy=xy(a.GetSize());rot=-a.GetOrientationDegrees();color='#e7bd6e' if a.GetNumber()=='1' else '#bdc8c7'
  s.append(f'<rect x="{x-sx/2}" y="{y-sy/2}" width="{sx}" height="{sy}" fill="{color}" transform="rotate({rot} {x} {y})"/>')
  if a.GetAttribute()==p.PAD_ATTRIB_PTH:s.append(f'<circle cx="{x}" cy="{y}" r=".525" fill="#f0f4fa"/>')
 if f.GetReference()!='U5':
  x,y=xy(f.GetPosition());s.append(f'<text x="{x}" y="{y+.2}" text-anchor="middle" font-family="sans-serif" font-size=".7" fill="#a3fff0">{f.GetReference()}</text>')
s+=['<text x="17" y="18" text-anchor="middle" font-family="sans-serif" font-size="1.25" fill="#ffffff">XIAO C6</text>','<text x="17" y="20.5" text-anchor="middle" font-family="sans-serif" font-size=".8" fill="#c2e5dd">zelf plaatsen</text>','<text x="17" y="32" text-anchor="middle" font-family="sans-serif" font-size="1.1" fill="#ffffff">USB-C</text>','<text x="40" y="27" font-family="sans-serif" font-size="2" fill="#17324a">89 × 34 mm · 0,6 mm · OSP</text>','<text x="40" y="31" font-family="sans-serif" font-size="1.5" fill="#325570">38 SMT-onderdelen · 2 koperlagen</text>','<text x="40" y="35" font-family="sans-serif" font-size="1.25" fill="#325570">J4: alleen Casio TX / RX / GND</text>','<text x="0" y="40" font-family="sans-serif" font-size="1.2" fill="#325570">Montagecontour, geen foto. Werkelijke passing en hoogte eerst testen.</text>','</svg>'];(r/'previews/Assembly-top.svg').write_text('\n'.join(s))
c=canvas.Canvas(str(r/'previews/Maatvoering.pdf'),pagesize=landscape(A4))
def txt(t,x,y,n=11):c.setFont('Helvetica',n);c.drawString(x*mm,y*mm,t)
def board(ox,oy,scale,details=False):
 def coord(a):return((ox+a[0]*scale)*mm,(oy+(34-a[1])*scale)*mm)
 c.setLineWidth(.15*mm);path=c.beginPath();path.moveTo(*coord(outline[0]))
 for a in outline[1:]:path.lineTo(*coord(a))
 path.close();c.drawPath(path)
 if details:
  for f in b.GetFootprints():
   for g in f.GraphicalItems():
    if isinstance(g,p.PCB_SHAPE) and g.GetLayer()==p.F_Fab and g.GetShape()==p.SHAPE_T_SEGMENT:c.line(*coord(xy(g.GetStart())),*coord(xy(g.GetEnd())))
   for a in f.Pads():
    if a.GetAttribute()==p.PAD_ATTRIB_PTH:
     x,y=coord(xy(a.GetPosition()));c.circle(x,y,.525*scale*mm)
   x,y=xy(f.GetPosition());c.setFont('Helvetica',5);c.drawCentredString(*coord((x,y)),f.GetReference())
txt('CG50 WiFi - Rev G | 1:1 maatvoering',20,190,18);txt('Print op 100% / werkelijke grootte. Controleer de 10mm-maat.',20,178)
board(30,110,1,True);txt('89 mm totaal',65,150);txt('34 mm',10,126);txt('34 x 34 mm kop',30,103);txt('55 x 19 mm arm',79,128)
c.line(30*mm,88*mm,40*mm,88*mm);txt('10 mm controlemaat',45,86)
txt('0,6mm PCB, 2 lagen, OSP. 14 doorgemetalliseerde XIAO-gaten: 1,05mm boring.',20,70)
txt('Steek 2,54mm; rijafstand 15,24mm. Losse soldeerpennen, geen headerrails.',20,62)
txt('USB-C blijft binnen de PCB-contour. Maximum inbouwhoogte 7mm is nog te meten.',20,54)
txt('Programmeer XIAO eerst los. Batterij ALTIJD los voordat USB wordt aangesloten.',20,42)
c.showPage();txt('Rev G - plaatsing bovenkant',20,190,18);board(22,70,2.8,True)
txt('USB-C op de XIAO wijst naar de onderrand van het brede deel.',20,50)
txt('J3: 1 BAT+, 2 GND. Kabel JST SHR-02V-S (SH, 1mm).',20,40)
txt('J4: 1 Casio TX, 2 Casio RX, 3 GND. Geen VIO-draad.',20,30)
txt('J4-kabel: JST GHR-03V-S (GH, 1,25mm). Controleer de draadvolgorde.',20,20);c.save()

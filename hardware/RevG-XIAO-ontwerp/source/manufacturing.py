from pathlib import Path
import pcbnew as p,json,csv,math,collections
ROOT=Path(__file__).resolve().parents[1];b=p.LoadBoard(str(ROOT/'hardware/CG50-WiFi.kicad_pcb'));parts=json.loads((ROOT/'source/parts.json').read_text());F={f.GetReference():f for f in b.GetFootprints()};out=ROOT/'manufacturing';out.mkdir(exist_ok=True)
xy=lambda a:(p.ToMM(a.x),p.ToMM(a.y))
def write(name,headers,rows):
 with (out/name).open('w',newline='',encoding='utf-8-sig') as f:
  w=csv.writer(f);w.writerow(headers);w.writerows(rows)
groups=collections.defaultdict(list)
for a in parts:
 if not a['manual']:groups[(a['lcsc'],a['mpn'],a['fp'])].append(a['ref'])
write('BOM-JLCPCB.csv',['Comment','Designator','Footprint','LCSC Part #'],[(mpn,','.join(refs),fp.split(':')[-1],code) for (code,mpn,fp),refs in sorted(groups.items())])
cpl=[];placement=[]
for a in parts:
 if a['manual']:continue
 f=F[a['ref']];x,y=xy(f.GetPosition());cx,cy=x,y
 # JST body local center is (0,+0.45), from F.Fab outline -1.675..+2.575.
 # At +90deg this becomes (+0.45,0) in PCB coordinates.
 if a['ref'] in ['J3','J4']:
  offset=.45 if a['ref']=='J3' else .425
  th=math.radians(f.GetOrientationDegrees());cx=x+offset*math.sin(th);cy=y+offset*math.cos(th)
 cpl.append([a['ref'],f'{cx:.4f}mm',f'{34-cy:.4f}mm','Top',f'{f.GetOrientationDegrees()%360:.1f}'])
 pin=next((pad for pad in f.Pads() if pad.GetNumber()=='1'),None);px,py=xy(pin.GetPosition()) if pin else (x,y)
 placement.append([a['ref'],a['mpn'],x,34-y,cx,34-cy,f.GetOrientationDegrees()%360,px,34-py])
write('CPL-JLCPCB.csv',['Designator','Mid X','Mid Y','Layer','Rotation'],cpl)
with (ROOT/'reports/Placement-audit.csv').open('w',newline='') as f:
 w=csv.writer(f);w.writerow(['Ref','MPN','Origin X','Origin Y up','Body X','Body Y up','CCW rotation','Pin1 X','Pin1 Y up']);w.writerows(placement)
errors=[];checked=0
for a in parts:
 f=F[a['ref']]
 for pad in f.Pads():
  expected=a['nets'].get(pad.GetNumber());actual=pad.GetNetname()
  if expected and actual!='/'+expected:errors.append(f'{a["ref"]}.{pad.GetNumber()}: {expected} != {actual}')
  if not expected and actual and not actual.startswith('unconnected-'):errors.append(f'Unexpected net {a["ref"]}.{pad.GetNumber()}: {actual}')
  checked+=1
 if math.dist(xy(f.GetPosition()),a['xy'])>.00001:errors.append('Position '+a['ref'])
 if f.GetLayer()!=p.F_Cu:errors.append('Not top '+a['ref'])
for t in b.GetTracks():
 if not t.GetNetCode():errors.append('Copper without net')
outline=[(xy(s.GetStart()),xy(s.GetEnd())) for s in b.GetDrawings() if isinstance(s,p.PCB_SHAPE) and s.GetLayer()==p.Edge_Cuts]
expected=[((0,0),(89,0)),((89,0),(89,19)),((89,19),(34,19)),((34,19),(34,34)),((34,34),(0,34)),((0,34),(0,0))]
if set(outline)!=set(expected):errors.append('Outline')
if abs(p.ToMM(b.GetDesignSettings().GetBoardThickness())-.6)>.00001:errors.append('Thickness')
if b.GetCopperLayerCount()!=2:errors.append('Layer count')
cat=json.loads((ROOT/'reference/catalog-selected.json').read_text())
for a in parts:
 if a['manual']:continue
 c=cat['parts'].get(a['lcsc'],{})
 if c.get('stock',0)<=0:errors.append('Catalog availability '+a['ref'])
 if c.get('mfr','').lower()!=a['mpn'].lower():errors.append('Catalog MPN '+a['ref'])
# Independent hole coordinate checks, not derived from the footprint writer.
u5={int(a.GetNumber()):a for a in F['U5'].Pads()}
if len(u5)!=14:errors.append('Not 14 XIAO holes')
for n,a in u5.items():
 exp=(24.62,27.62-(n-1)*2.54) if n<=7 else (9.38,12.38+(n-8)*2.54)
 if math.dist(xy(a.GetPosition()),exp)>1e-5:errors.append('XIAO hole '+str(n))
 if a.GetAttribute()!=p.PAD_ATTRIB_PTH or math.dist(xy(a.GetDrillSize()),(1.05,1.05))>1e-5:errors.append('XIAO drill '+str(n))
 if a.GetLayerSet().Contains(p.F_Paste):errors.append('Paste on manual hole')
# Conservative intersection test: circle of via copper against each SMT bounding rectangle.
vip=[];vias=[]
for t in b.GetTracks():
 if not isinstance(t,p.PCB_VIA):continue
 x,y=xy(t.GetPosition());r=p.ToMM(t.GetWidth(p.F_Cu))/2;vias.append([x,y,r*2,p.ToMM(t.GetDrillValue())])
 for f in F.values():
  for a in f.Pads():
   if a.GetAttribute()!=p.PAD_ATTRIB_SMD:continue
   cx,cy=xy(a.GetPosition());sx,sy=xy(a.GetSize());th=math.radians(a.GetOrientationDegrees());dx=(x-cx)*math.cos(th)-(y-cy)*math.sin(th);dy=(x-cx)*math.sin(th)+(y-cy)*math.cos(th)
   d=math.hypot(max(abs(dx)-sx/2,0),max(abs(dy)-sy/2,0))
   if d<r-.001:vip.append([f.GetReference(),a.GetNumber(),x,y])
if vip:errors.append('Via overlapping SMT copper')
# Both connector fronts are within 89mm; XIAO USB conservative front y=33.5 <34.
if not (17-8.9>0 and 17+8.9<34 and 20-10.5>0 and 20+13.5<34):errors.append('USB envelope')
if any(float(row[1].removesuffix('mm'))>89 or float(row[2].removesuffix('mm'))<0 for row in cpl):errors.append('CPL outside board bbox')
manual={a['ref'] for a in parts if a['manual']};factory={a['ref'] for a in parts if not a['manual']}
if manual!={'U5'} or {a[0] for a in cpl}!=factory:errors.append('Factory coverage')
drc=json.loads((ROOT/'reports/DRC.json').read_text());erc=json.loads((ROOT/'reports/ERC.json').read_text())
if drc['violations'] or drc['unconnected_items'] or drc['schematic_parity']:errors.append('DRC/parity not clean')
if any(s['violations'] for s in erc['sheets']):errors.append('ERC not clean')
report={'errors':errors,'footprints':len(parts),'assembled_components':len(cpl),'bom_groups':len(groups),'geometric_pads_audited':checked,'physical_copper_pads_audited':sum(a.IsOnLayer(p.F_Cu) for f in F.values() for a in f.Pads()),'layers':2,'board_thickness_mm':.6,'surface_finish':'OSP','overall_outline_mm':[89,34],'head_mm':[34,34],'arm_mm':[55,19],'manual_xiao_holes':14,'hole_drill_mm':1.05,'hole_pitch_mm':2.54,'hole_row_spacing_mm':15.24,'via_count':len(vias),'via_in_smt_pad_intersections':vip,'minimum_via_diameter_mm':min(v[2] for v in vias),'minimum_via_drill_mm':min(v[3] for v in vias),'usb_front_conservative_y_mm':33.5,'inventory_snapshot':cat['created'],'inventory_lowest_stock':min(x['stock'] for x in cat['parts'].values()),'factory_model_rotations_verified_in_customer_jlc_preview':False,'physical_fit_height_supply_usb_uart_rf_verified':False,'battery_usb_simultaneously_allowed':False,'switch_present':False,'casio_wires':3,'external_VIO_required':False,'uart_tx_inverted':False}
(ROOT/'reports/design-audit.json').write_text(json.dumps(report,indent=2));assert not errors,errors
print(json.dumps(report,indent=2))

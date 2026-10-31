from pathlib import Path
import json,re,csv,collections,hashlib
import pcbnew as p
r=Path(__file__).resolve().parents[1];g=r/'gerbers';b=p.LoadBoard(str(r/'hardware/CG50-WiFi.kicad_pcb'));xy=lambda a:(round(p.ToMM(a.x),6),round(34-p.ToMM(a.y),6));expected=collections.Counter()
for f in b.GetFootprints():
 for a in f.Pads():
  if a.IsOnLayer(p.F_Cu):expected[xy(a.GetPosition())]+=1
for t in b.GetTracks():
 if isinstance(t,p.PCB_VIA):expected[xy(t.GetPosition())]+=1
flash=collections.Counter((int(a)/1e6,int(bb)/1e6) for a,bb in re.findall(r'^X(-?\d+)Y(-?\d+)D03\*$',(g/'CG50-WiFi-F_Cu.gtl').read_text(),re.M))
assert flash==expected,('Copper flash centers',expected-flash,flash-expected)
# Independent Excellon tool and drill coordinates versus actual board holes.
drills={};tool=None
for line in (g/'CG50-WiFi-PTH.drl').read_text().splitlines():
 if re.fullmatch(r'T\d+',line):tool=line
 m=re.fullmatch(r'X(-?[\d.]+)Y(-?[\d.]+)',line)
 if m:drills.setdefault(tool,[]).append((float(m[1]),float(m[2])))
via_count=sum(isinstance(t,p.PCB_VIA) for t in b.GetTracks());assert len(drills['T1'])==via_count and len(drills['T2'])==14
assert set(drills['T2'])=={xy(a.GetPosition()) for f in b.GetFootprints() if f.GetReference()=='U5' for a in f.Pads()}
assert set(drills['T1'])=={xy(t.GetPosition()) for t in b.GetTracks() if isinstance(t,p.PCB_VIA)}
assert 'T1C0.300' in (g/'CG50-WiFi-PTH.drl').read_text() and 'T2C1.050' in (g/'CG50-WiFi-PTH.drl').read_text()
# Profile is six straight edges in a common lower-left datum.
profile=(g/'CG50-WiFi-Edge_Cuts.gm1').read_text();segments=[];last=None
for xx,yy,dd in re.findall(r'^X(-?\d+)Y(-?\d+)D0([12])\*$',profile,re.M):
 a=(int(xx)/1e6,int(yy)/1e6)
 if dd=='1':segments.append(frozenset((last,a)))
 last=a
out=[(0,34),(89,34),(89,15),(34,15),(34,0),(0,0)];assert set(segments)=={frozenset((a,bb)) for a,bb in zip(out,out[1:]+out[:1])}
# No XIAO paste apertures; only the fourteen manual holes on the bottom mask.
assert '%TO.C,U5*%' not in (g/'CG50-WiFi-F_Paste.gtp').read_text()
bmask=(g/'CG50-WiFi-B_Mask.gbs').read_text();assert len(re.findall(r'D03\*',bmask))==14
parts=json.loads((r/'source/parts.json').read_text());factory={a['ref'] for a in parts if not a['manual']}
with (r/'manufacturing/BOM-JLCPCB.csv').open(encoding='utf-8-sig') as f:bom=list(csv.DictReader(f))
with (r/'manufacturing/CPL-JLCPCB.csv').open(encoding='utf-8-sig') as f:cpl=list(csv.DictReader(f))
assert {ref for a in bom for ref in a['Designator'].split(',')}==factory=={a['Designator'] for a in cpl}
assert len(cpl)==len(factory)==38
report={'errors':[],'gerber_top_copper_pad_and_via_centers_matched':sum(expected.values()),'plated_via_drills':via_count,'plated_xiao_drills':14,'profile_edges_matched':6,'bottom_solder_mask_hole_openings':14,'manual_xiao_in_bom_or_cpl':False,'manual_xiao_paste':False,'cpl_bom_factory_refs_match':True,'common_origin':'X=0,Y=0 at board lower-left; X right, Y up','not_a_full_gerber_polygon_or_manufacturing_simulation':True}
(r/'reports/export-audit.json').write_text(json.dumps(report,indent=2));print(json.dumps(report,indent=2))

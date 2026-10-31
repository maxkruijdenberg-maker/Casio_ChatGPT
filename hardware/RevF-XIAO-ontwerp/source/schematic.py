import json,uuid,pathlib,math
ROOT=pathlib.Path(__file__).resolve().parents[1];HW=ROOT/'hardware';parts=json.loads((ROOT/'source/parts.json').read_text())
def uid(s):return str(uuid.uuid5(uuid.NAMESPACE_DNS,'cg50-revf-'+s))
def q(s):return json.dumps(str(s),ensure_ascii=False)
def eff(size=1):return f'(effects (font (size {size} {size})))'
lib=[];inst=[];connections=[]
# Functional blocks are ordered along rows: power, sockets, receive, transmit.
order=[a['ref'] for a in parts if a['ref']!='U5']+['U5']
by={a['ref']:a for a in parts}
for idx,ref in enumerate(order):
 a=by[ref];bom='no' if a.get('manual') else 'yes';name='Part_'+ref;keys=list(a['nets']);N=len(keys);split=math.ceil(N/2);h=max(3.81,(split-1)*1.27+2.54);w=8.89
 pos={}
 for j,k in enumerate(keys):
  left=j<split;row=j if left else j-split;pos[k]=(-w-2.54 if left else w+2.54, ((split-1)/2-row)*2.54,0 if left else 180)
 pins=[]
 for k,(x,y,ang) in pos.items():
  n,t=a['pins'].get(k,[k,'passive']);
  if ref=='U1' and k=='8':t='passive'
  pins.append(f'(pin {t} line (at {x} {y} {ang})(length 2.54)(name {q(n)} {eff(.9)})(number {q(k)} {eff(.9)}))')
 lib.append(f'''(symbol "CG50:{name}" (pin_names (offset .5))(in_bom {bom})(on_board yes)
 (property "Reference" "{ref.rstrip('0123456789')}" (at 0 {h+2.54} 0) {eff()})
 (property "Value" {q(a['value'])} (at 0 {-h-2.54} 0) {eff()})
 (symbol "{name}_0_1" (rectangle (start {-w} {h})(end {w} {-h})(stroke (width .254)(type default))(fill (type background))))
 (symbol "{name}_1_1" {''.join(pins)}))''')
 col=idx%6;row=idx//6;X=35.56+col*88.9;Y=30.48+row*33.02
 if ref=='U5':X=650.24;Y=86.36
 props=f'(property "Reference" {q(ref)} (at {X} {Y-h-2} 0) {eff(1.1)})(property "Value" {q(a["value"])} (at {X} {Y+h+2} 0) {eff(.9)})'
 for pname,val in [('Footprint','CG50:'+a['fp'].split(':')[1]),('MPN',a['mpn']),('LCSC',a['lcsc'])]:props+=f'(property {q(pname)} {q(val)} (at {X} {Y} 0)(effects (font (size 1 1))(hide yes)))'
 inst.append(f'(symbol (lib_id "CG50:{name}")(at {X} {Y} 0)(unit 1)(in_bom {bom})(on_board yes)(dnp no)(uuid {a["uuid"]}){props}'+''.join(f'(pin {q(k)} (uuid {uid(ref+"-pin"+k)}))' for k in keys)+f'(instances (project "CG50-WiFi" (path "/{uid("sheet")}" (reference {q(ref)})(unit 1)))))')
 for k,(x,y,ang) in pos.items():
  ax=round(X+x,4);ay=round(Y-y,4);net=a['nets'][k]
  if net is None:connections.append(f'(no_connect (at {ax} {ay})(uuid {uid(ref+"nc"+k)}))');continue
  ex=round(ax+(-3.81 if x<0 else 3.81),4)
  connections.append(f'(wire (pts (xy {ax} {ay})(xy {ex} {ay}))(stroke (width 0)(type default))(uuid {uid(ref+"w"+k)}))')
  connections.append(f'(label {q(net)} (at {ex} {ay} {0 if x<0 else 180}) {eff(.85)} (uuid {uid(ref+"lbl"+k)}))')
# flags are separate visibly documented sources, not ERC exclusions
lib.append('(symbol "CG50:PWR_FLAG" (power)(pin_numbers hide)(pin_names (offset 0) hide)(in_bom no)(on_board no)(property "Reference" "#FLG" (at 0 0 0)(effects (font (size 1 1)) hide))(property "Value" "PWR_FLAG" (at 0 2.54 0) '+eff()+')(symbol "PWR_FLAG_0_1" (polyline (pts (xy 0 0)(xy -1.27 1.27)(xy 0 2.54)(xy 1.27 1.27)(xy 0 0))(stroke (width .254)(type default))(fill (type none))))(symbol "PWR_FLAG_1_1" (pin power_out line (at 0 0 90)(length 0)(name "pwr" '+eff()+')(number "1" '+eff()+'))))')
for j,n in enumerate(['VIN','GND']):
 X=599.44+j*50.8;Y=190.5;ref='#FLG0'+str(j+1)
 inst.append(f'(symbol (lib_id "CG50:PWR_FLAG")(at {X} {Y} 0)(unit 1)(in_bom no)(on_board no)(dnp no)(uuid {uid(ref)})(property "Reference" "{ref}" (at {X} {Y} 0)(effects (font (size 1 1)) hide))(property "Value" "PWR_FLAG" (at {X} {Y-3} 0){eff(.85)})(pin "1" (uuid {uid(ref+"pin")}))(instances (project "CG50-WiFi" (path "/{uid("sheet")}" (reference "{ref}")(unit 1)))))')
 connections.append(f'(label "{n}" (at {X} {Y} 0){eff(.85)}(uuid {uid(ref+"label")}))')
notes=['REV F | 89x34 L outline | 0.6mm | OSP | 2 copper layers',
'CUSTOMER installs XIAO ESP32-C6 without headers; 14 PTH holes only. Exclude U5 from PCBA.',
'USB-C belongs to XIAO; no factory-mounted bare C6, CH340C, USB connector or programmer.',
'TPS63070 adjustable: 200k / 38.3k -> 4.978V nominal; U5 provides 3V3 for the logic.',
'Disconnect J3 battery BEFORE connecting USB. Program XIAO before installation.',
'XIAO D7 GPIO17 RX, D6 GPIO16 inverted TX; D1 GPIO1 heartbeat; D2 GPIO2 arm.',
'J3: 1 BAT+, 2 GND. J4: 1 Casio TX, 2 Casio RX, 3 GND, 4 switched Casio VIO.',
'No vias in SMT lands; normal tented through vias. No VIPPO process requested.',
'Physical installed height <=7mm, USB envelope and power/RF behavior need first-article tests.']
for j,t in enumerate(notes):connections.append(f'(text {q(t)} (at 16 {480+j*3.5} 0)(effects (font (size .9 .9))(justify left))(uuid {uid("note"+str(j))}))')
s='(kicad_sch (version 20250114)(generator "eeschema")(uuid '+uid('sheet')+')(paper "A1")(title_block (title "Casio fx-CG50 - hand-soldered XIAO C6")(rev "F - XIAO PTH no headers")(date "2026-10-31"))(lib_symbols '+''.join(lib)+')'+''.join(connections)+''.join(inst)+')'
(HW/'CG50-WiFi.kicad_sch').write_text(s)
print('schematic',len(parts))

(HW/"CG50.kicad_sym").write_text('(kicad_symbol_lib (version 20241209)(generator "kicad_symbol_editor")'+"".join(lib).replace('(symbol "CG50:','(symbol "')+')')
(HW/"sym-lib-table").write_text('(sym_lib_table (lib (name "CG50")(type "KiCad")(uri "${KIPRJMOD}/CG50.kicad_sym")(options "")(descr "Project symbols")))')

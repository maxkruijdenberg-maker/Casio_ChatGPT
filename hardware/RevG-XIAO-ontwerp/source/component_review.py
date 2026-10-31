"""Independent component/net/land-pattern checks against cited manufacturer facts."""
from pathlib import Path
import json,math,hashlib,sys,xml.etree.ElementTree as E,collections
import pcbnew as p
r=Path(__file__).resolve().parents[1];repo=Path(sys.argv[1]) if len(sys.argv)>1 else r.parents[1]
parts={a['ref']:a for a in json.loads((r/'source/parts.json').read_text())};cat=json.loads((r/'reference/catalog-selected.json').read_text())
b=p.LoadBoard(str(r/'hardware/CG50-WiFi.kicad_pcb'));f={a.GetReference():a for a in b.GetFootprints()};errors=[]
net={}
for n in E.parse(r/'reports/netlist.xml').findall('.//nets/net'):
 for a in n.findall('node'):net[(a.get('ref'),a.get('pin'))]=n.get('name').removeprefix('/')
def expect(ref,m):
 for pin,want in m.items():assert net[(ref,str(pin))]==want,(ref,pin,want)
expect('U1',{1:'PS',3:'VAUX',4:'GND',5:'FB',7:'REG5V',8:'REG5V',9:'SW2',10:'GND',11:'SW1',12:'VIN',13:'VIN',14:'EN',15:'GND'})
expect('R3',{1:'VIN',2:'PS'});expect('C8',{1:'VAUX',2:'GND'})
expect('R15',{1:'REG5V',2:'FB'});expect('R16',{1:'FB',2:'GND'})
expect('U5',{1:'RX_WAKE',2:'HEARTBEAT',3:'TX_ARM',7:'TX_GPIO',8:'RX_WAKE',12:'3V3',13:'GND',14:'REG5V'})
expect('U3',{2:'RX_IN',3:'GND',4:'RX_WAKE',5:'3V3'})
expect('U4',{1:'GND',2:'HEARTBEAT',3:'TX_ARM',4:'GND',5:'TX_ENABLE',6:'MONO_C',7:'MONO_RC',8:'3V3'})
expect('U6',{1:'TX_ENABLE',2:'TX_DATA',3:'GND',4:'TX_OUT',5:'3V3'})
expect('R7',{1:'TX_GPIO',2:'TX_DATA'});expect('R8',{1:'TX_DATA',2:'3V3'})
expect('R10',{1:'TX_ENABLE',2:'GND'});expect('R6',{1:'TX_OUT',2:'CASIO_RX'})
expect('J4',{1:'CASIO_TX',2:'CASIO_RX',3:'GND'})
assert not any('VIO' in n.GetNetname() for n in b.GetNetsByNetcode().values())
assert parts['U1']['nets']['2'] is None and parts['U1']['nets']['6'] is None
assert not {'Q2','Q3','R9','R11','C16'}&parts.keys()
assert parts['R5']['value']=='100k'
assert cat['parts']['C514175']['decoded_attributes']['Manufacturer']=='JST'
assert cat['parts']['C514175']['stock']>=5
assert parts['J4']['lcsc']=='C514175' and parts['U6']['lcsc']=='C7834'
# TI 4222000/B physical copper lands. 8/13 are interior net-number markers.
x0,y0=p.ToMM(f['U1'].GetPosition().x),p.ToMM(f['U1'].GetPosition().y)
u={a.GetNumber():a for a in f['U1'].Pads() if a.GetNumber()}
for n,cx,cy,w,h in [(7,.525,1.4,.85,.7),(12,.525,-1.4,.85,.7),(9,.775,.5,1.45,.35),(11,.775,-.5,1.45,.35),(10,.6,0,1.8,.35)]:
 a=u[str(n)];actual=(p.ToMM(a.GetPosition().x)-x0,p.ToMM(a.GetPosition().y)-y0,p.ToMM(a.GetSize().x),p.ToMM(a.GetSize().y))
 assert all(abs(x-y)<1e-6 for x,y in zip(actual,(cx,cy,w,h))),actual
 assert a.IsOnLayer(p.F_Cu) and not a.IsOnLayer(p.F_Mask) and not a.IsOnLayer(p.F_Paste)
masks=[sh for sh in f['U1'].GraphicalItems() if isinstance(sh,p.PCB_SHAPE) and sh.GetLayer()==p.F_Mask]
assert len(masks)==2 and all(abs(sh.GetPolyShape().Area()/1e12-.3625)<1e-8 for sh in masks)
paste=[a for a in f['U1'].Pads() if not a.IsOnLayer(p.F_Cu) and a.IsOnLayer(p.F_Paste)]
assert len(paste)==10
assert collections.Counter((round(p.ToMM(a.GetSize().x),3),round(p.ToMM(a.GetSize().y),3)) for a in paste)=={(.575,.25):4,(.72,.25):2,(.25,.6):4}
# Manufacturer selection checks and conservative DC-bias arithmetic (not SPICE).
vo=.8*(1+200/38.3);imin=3.65;eff=.85;load=.5;f_hz=2.4e6;lmin=1.5e-6*.7;lmax=1.5e-6*1.3
peak=vo*load/(imin*eff)+imin*(1-imin/vo)/(2*f_hz*lmin)
cout_nom=6*22+10;cout_eff=cout_nom*.2;cin_eff=3*10*.2
assert cout_eff>=10*(lmax*1e6) and cin_eff>=4.7 and cout_nom*1.2<=470
assert peak*1.2<cat['parts']['C42099']['decoded_attributes']['Saturation Current (Isat)']
fw=(repo/'xiao-revg/ChatGPTXIAORevG/ChatGPTXIAORevG.ino').read_text();hh=(repo/'xiao-revg/ChatGPTXIAORevG/hardware.h').read_text()
assert 'UART_SIGNAL_INV_DISABLE' in fw and 'UART_SIGNAL_TXD_INV' not in fw
assert 'RX_PIN=17,TX_PIN=16,TX_ARM_PIN=2,HEARTBEAT_PIN=1' in hh
assert 'delayMicroseconds(200)' in hh
power_tracks={}
for name in ['/BAT+','/VIN','/REG5V','/3V3']:
 ts=[t for t in b.GetTracks() if t.GetNetname()==name and not isinstance(t,p.PCB_VIA)]
 assert min(p.ToMM(t.GetWidth()) for t in ts)>=.4
 power_tracks[name]={'min_width_mm':min(p.ToMM(t.GetWidth()) for t in ts),'total_branch_length_mm':sum(p.ToMM(t.GetLength()) for t in ts),'upper_bound_all_tracks_series_ohms_at_35um_20C':sum(.01724*(p.ToMM(t.GetLength())/1000)/(p.ToMM(t.GetWidth())*.035) for t in ts)}
report={'errors':[],'casio_wires':3,'external_VIO_required':False,'factory_components':38,'bom_codes':20,'catalogue_MPN_and_manufacturer_J4':'JST SM03B-GHS-TB(LF)(SN) C514175','output_nominal_V':vo,'input_effective_capacitance_uF_assuming_80_percent_total_loss':cin_eff,'output_effective_capacitance_uF_assuming_80_percent_total_loss':cout_eff,'output_C_over_L_uF_per_uH':cout_eff/(lmax*1e6),'TI_adjustable_minimum_ratio':10,'inductor_calculated_peak_A_at_0_5A_output':peak,'inductor_catalogue_Isat_A':4.45,'inductor_catalogue_rated_A':1.98,'TI_RNM0015A_copper_mask_stencil_checks':'pass; paired lands preserve all 15 logical pin numbers','uart_normal_push_pull_matches_working_Atom_signal_type':True,'power_tracks':power_tracks,'sources':['TI SLVSC58B rev B, pin table p3; selection p16-19; RNM0015A 4222000/B p40-41','https://raw.githubusercontent.com/C7H10N2/TPS63070-Converter/739374ab235b3f08ab6fa69a90cf4c2b3f508a58/Doc/Datasheet/TPS63070RNMR_en.pdf','https://raw.githubusercontent.com/Seeed-Studio/wiki-documents/98874b6/sites/en/docs/Sensor/SeeedStudio_XIAO/SeeedStudio_XIAO_ESP32C6/XIAO_ESP32C6_Getting_Started.md'],'physical_measurements_performed':False,'manufacturer_SPICE_model_used':False,'unmeasured_Casio_input_load_still_requires_hardware_confirmation':True}
(r/'reports/component-compatibility.json').write_text(json.dumps(report,indent=2));print(json.dumps(report,indent=2))

import pcbnew as p, pathlib, json, uuid, shutil, math, os, re
ROOT=pathlib.Path(__file__).resolve().parents[1]; HW=ROOT/'hardware';LIB=pathlib.Path(os.environ.get('KICAD9_FOOTPRINT_DIR','/usr/share/kicad/footprints')); FP=HW/'CG50.pretty';FP.mkdir(exist_ok=True)
def uid(s):return str(uuid.uuid5(uuid.NAMESPACE_DNS,'cg50-revf-'+s))
def v(x,y):return p.VECTOR2I(round(x*1e6),round(y*1e6))
def ls(*layers):
 s=p.LSET()
 for l in layers:s.AddLayer(l)
 return s
plugin=p.PCB_IO_MGR.PluginFind(p.PCB_IO_MGR.KICAD_SEXP)
parts=json.loads((ROOT/'source/parts.json').read_text())
# Create custom land patterns, with top-view numbering from TI RNM0015A 4222000/B.
def custom(name,pads,body):
 f=p.FOOTPRINT(None);f.SetReference('REF**');f.SetValue(name);f.SetAttributes(p.FP_SMD)
 for num,x,y,w,h,tht in pads:
  a=p.PAD(f);a.SetNumber(str(num));a.SetPosition(v(x,y));a.SetSize(v(w,h));a.SetShape(p.PAD_SHAPE_ROUNDRECT if not tht else p.PAD_SHAPE_CIRCLE)
  if tht:a.SetAttribute(p.PAD_ATTRIB_PTH);a.SetDrillSize(v(.8,.8));a.SetLayerSet(ls(p.F_Cu,p.B_Cu,p.F_Mask,p.B_Mask));f.SetAttributes(p.FP_THROUGH_HOLE)
  else:a.SetAttribute(p.PAD_ATTRIB_SMD);a.SetLayerSet(ls(p.F_Cu,p.F_Paste,p.F_Mask));a.SetRoundRectRadiusRatio(.2)
  f.Add(a)
 def rect(layer,ex):
  x,y=body[0]/2+ex,body[1]/2+ex
  for A,B in [((-x,-y),(x,-y)),((x,-y),(x,y)),((x,y),(-x,y)),((-x,y),(-x,-y))]:
   s=p.PCB_SHAPE(f);s.SetShape(p.SHAPE_T_SEGMENT);cy=(int(name[-1])-1)*1.25 if name.startswith('Wire_') else 0;s.SetStart(v(A[0],A[1]+cy));s.SetEnd(v(B[0],B[1]+cy));s.SetLayer(layer);s.SetWidth(int(.05*1e6));f.Add(s)
 rect(p.F_Fab,0);rect(p.F_CrtYd,.35);plugin.FootprintSave(str(FP),f) if False else None
 # library writer needs footprint identifier name
 f.SetFPID(p.LIB_ID('CG50',name));plugin.FootprintSave(str(FP),f)
custom('RNM0015A',[(i,-1.15,-.75+(i-1)*.5,.6,.25,False) for i in range(1,5)]+[(5,-.725,1.4,.25,.6,False),(6,-.225,1.4,.25,.6,False),(7,.3,1.4,.25,.6,False),(8,.825,1.4,.25,.6,False),(9,.675,.5,1.35,.25,False),(10,.525,0,1.7,.25,False),(11,.675,-.5,1.35,.25,False),(12,.825,-1.4,.25,.6,False),(13,.3,-1.4,.25,.6,False),(14,-.225,-1.4,.25,.6,False),(15,-.725,-1.4,.25,.6,False)],(2.5,3.4))
custom('XFL4020',[(1,-1.675,0,.98,3.4,False),(2,1.675,0,.98,3.4,False)],(4.35,4.35))
for n in [2,4]:custom('Wire_'+str(n),[(i,0,(i-1)*2.5,2,2,True) for i in range(1,n+1)],(2.2,n*2.5))
# Two true through-hole rows for individual wire pins, no plastic headers.
# Official Seeed DIP hole centers: 2.54mm pitch and 15.24mm row spacing.
# No extra castellated SMT pads or bottom BAT/debug pads.
f=p.FOOTPRINT(None);f.SetReference('REF**');f.SetValue('XIAO customer solder');f.SetAttributes(p.FP_THROUGH_HOLE|p.FP_EXCLUDE_FROM_BOM|p.FP_EXCLUDE_FROM_POS_FILES)
for n in range(1,15):
 x=7.62-(n-1)*2.54 if n<=7 else -7.62+(n-8)*2.54;y=-7.62 if n<=7 else 7.62
 a=p.PAD(f);a.SetNumber(str(n));a.SetPosition(v(x,y));a.SetSize(v(2,2));a.SetDrillSize(v(1.05,1.05));a.SetAttribute(p.PAD_ATTRIB_PTH);a.SetShape(p.PAD_SHAPE_RECT if n==1 else p.PAD_SHAPE_CIRCLE);a.SetLayerSet(ls(p.F_Cu,p.B_Cu,p.F_Mask,p.B_Mask));f.Add(a)
# Conservative module plus USB envelope: after 270deg rotation, USB front
# remains at y=33.5, inside the 34mm carrier. This is not a measured 3D fit.
for layer in [p.F_Fab,p.F_CrtYd]:
 points=[(-10.5,-8.9),(10.5,-8.9),(10.5,-5.5),(13.5,-5.5),(13.5,5.5),(10.5,5.5),(10.5,8.9),(-10.5,8.9)]
 if layer==p.F_CrtYd:points=[(-11,-9.4),(11,-9.4),(11,-5.8),(13.8,-5.8),(13.8,5.8),(11,5.8),(11,9.4),(-11,9.4)]
 for A,B in zip(points,points[1:]+points[:1]):
  z=p.PCB_SHAPE(f);z.SetShape(p.SHAPE_T_SEGMENT);z.SetStart(v(*A));z.SetEnd(v(*B));z.SetLayer(layer);z.SetWidth(50000);f.Add(z)
f.SetFPID(p.LIB_ID('CG50','XIAO-C6-THT-14-Customer'));plugin.FootprintSave(str(FP),f)
# PCB
b=p.BOARD();b.GetDesignSettings().SetCopperLayerCount(2);b.GetDesignSettings().SetBoardThickness(int(.6e6));b.GetDesignSettings().SetAuxOrigin(v(0,34))
netnames=sorted({n for a in parts for n in a['nets'].values() if n});nets={}
for n in netnames:a=p.NETINFO_ITEM(b,n);b.Add(a);nets[n]=a
fps={}
for a in parts:
 lib,name=a['fp'].split(':');src=FP if lib=='CG50' else LIB/(lib+'.pretty');f=plugin.FootprintLoad(str(src),name)
 if not f:raise Exception(a['fp'])
 # bundle standard footprint locally
 if lib!='CG50':shutil.copy2(src/(name+'.kicad_mod'),FP/(name+'.kicad_mod'))

 if a['ref']=='U5':
  f.SetAttributes(p.FP_THROUGH_HOLE|p.FP_EXCLUDE_FROM_BOM|p.FP_EXCLUDE_FROM_POS_FILES)
 f.SetFPID(p.LIB_ID('CG50',name));f.SetReference(a['ref']);f.SetValue(a['value']);f.SetUuid(p.KIID(a['uuid'])) if hasattr(f,'SetUuid') else None
 f.SetPath(p.KIID_PATH('/'+uid('sheet')+'/'+a['uuid']));b.Add(f);f.SetPosition(v(*a['xy']));f.SetOrientationDegrees(a['rot'])
 for pad in f.Pads():
  n=a['nets'].get(pad.GetNumber());
  if n:pad.SetNet(nets[n])
 f.Reference().SetTextSize(v(.7,.7));f.Reference().SetTextThickness(int(.12e6));f.Reference().SetPosition(v(a['xy'][0],a['xy'][1]-2.7));f.Reference().SetLayer(p.F_Fab);f.Value().SetVisible(False)
 fps[a['ref']]=f
# outline and descriptive silkscreen
for A,B in [((0,0),(89,0)),((89,0),(89,19)),((89,19),(34,19)),((34,19),(34,34)),((34,34),(0,34)),((0,34),(0,0))]:
 s=p.PCB_SHAPE();s.SetShape(p.SHAPE_T_SEGMENT);s.SetStart(v(*A));s.SetEnd(v(*B));s.SetWidth(int(.05e6));s.SetLayer(p.Edge_Cuts);b.Add(s)
def text(t,x,y,size=.8,layer=p.F_SilkS):
 s=p.PCB_TEXT(b);s.SetText(t);s.SetPosition(v(x,y));s.SetTextSize(v(size,size));s.SetTextThickness(int(.13e6));s.SetLayer(layer);s.SetMirrored(layer==p.B_SilkS);b.Add(s)
text('CG50 WiFi | REV F',48,1.6,.65,p.B_SilkS);text('USB',29,30,.65)
text('J3 BAT 1+ 2-',84,1,.6,p.B_SilkS);text('J4 TX RX G VIO',76,18,.6,p.B_SilkS)
text('ANT',13,5,.6);text('HAND SOLDER XIAO',17,8,.6);text('DISCONNECT BAT BEFORE USB',48,18,.6,p.B_SilkS)
# Antenna keepout both copper layers below sockets, deliberately generous board-edge opening.
z=p.ZONE(b);z.SetIsRuleArea(True);z.SetDoNotAllowTracks(True);z.SetDoNotAllowVias(True);z.SetDoNotAllowCopperPour(True);z.SetDoNotAllowPads(True);z.SetLayerSet(ls(p.F_Cu,p.B_Cu));poly=z.Outline();poly.NewOutline()
for x,y in [(10.6,0),(15.5,0),(15.5,14.5),(10.6,14.5)]:poly.Append(int(x*1e6),int(y*1e6))
b.Add(z)
p.SaveBoard(str(HW/'CG50-WiFi.kicad_pcb'),b)
# JSON is authoritative design data for independent parity check.
(ROOT/'source/parts.json').write_text(json.dumps(parts,indent=2))
(HW/'fp-lib-table').write_text('(fp_lib_table (lib (name "CG50")(type "KiCad")(uri "${KIPRJMOD}/CG50.pretty")(options "")(descr "Bundled verified land patterns")))\n')
# Explicit minimum fabrication constraints. No waived electrical DRC errors.
pro={'meta':{'filename':'CG50-WiFi.kicad_pro','version':1},'board':{'design_settings':{'rules':{'min_clearance':.15,'min_track_width':.15,'min_via_diameter':.6,'min_via_drill':.3,'min_hole_clearance':.25,'min_copper_edge_clearance':.3,'min_silk_clearance':.15,'min_text_height':.6,'min_text_thickness':.1},'defaults':{'zones':{'min_clearance':.2}}}},'net_settings':{'classes':[{'name':'Default','clearance':.15,'track_width':.2,'via_diameter':.6,'via_drill':.3,'microvia_diameter':.3,'microvia_drill':.1,'diff_pair_width':.2,'diff_pair_gap':.25,'diff_pair_via_gap':.25}],'meta':{'version':3}},'erc':{'erc_exclusions':[]},'schematic':{'legacy_lib_dir':'','legacy_lib_list':[]}}
(HW/'CG50-WiFi.kicad_pro').write_text(json.dumps(pro,indent=2))
print('PCB CREATED',len(parts),'components',len(netnames),'nets')

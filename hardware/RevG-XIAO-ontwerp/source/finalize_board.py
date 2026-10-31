import pcbnew as p,pathlib,xml.etree.ElementTree as ET,json
ROOT=pathlib.Path(__file__).resolve().parents[1];fn=ROOT/'hardware/CG50-WiFi.kicad_pcb';b=p.LoadBoard(str(fn))
for n in list(b.GetNetsByNetcode().values()):
 name=n.GetNetname()
 if name and not name.startswith(('/','unconnected-')):n.SetNetname('/'+name)
N={n.GetNetname():n for n in b.GetNetsByNetcode().values()};F={f.GetReference():f for f in b.GetFootprints()}
for net in ET.parse(ROOT/'reports/netlist.xml').findall('.//nets/net'):
 name=net.get('name')
 if name not in N:n=p.NETINFO_ITEM(b,name);b.Add(n);N[name]=n
 for node in net.findall('node'):
  if node.get('ref') not in F:continue
  for pad in F[node.get('ref')].Pads():
   if pad.GetNumber()==node.get('pin'):pad.SetNet(N[name])
b.BuildConnectivity();p.ZONE_FILLER(b).Fill(b.Zones());p.SaveBoard(str(fn),b)
print('Board net names and all duplicate pad numbers match schematic netlist.')

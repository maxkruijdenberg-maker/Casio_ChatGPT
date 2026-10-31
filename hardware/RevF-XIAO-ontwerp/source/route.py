import pcbnew as p,numpy as np,math,heapq,json
from pathlib import Path
from scipy.ndimage import distance_transform_edt
ROOT=Path(__file__).resolve().parents[1];FN=ROOT/'hardware/CG50-WiFi.kicad_pcb';b=p.LoadBoard(str(FN));S=.1;NX=891;NY=341;LAYERS=[p.F_Cu,p.B_Cu]
F={f.GetReference():f for f in b.GetFootprints()};N={n.GetNetname():n for n in b.GetNetsByNetcode().values()}
v=lambda x,y:p.VECTOR2I(round(x*1e6),round(y*1e6));xy=lambda a:(p.ToMM(a.x),p.ToMM(a.y));idx=lambda a:(round(a[0]/S),round(a[1]/S))
def pt(ref,pin):return xy(next(a for a in F[ref].Pads() if a.GetNumber()==str(pin)).GetPosition())
def tr(net,pts,w=.2,l=0):
 for A,B in zip(pts,pts[1:]):
  if math.dist(A,B)<.000001:continue
  t=p.PCB_TRACK(b);t.SetStart(v(*A));t.SetEnd(v(*B));t.SetWidth(round(w*1e6));t.SetLayer(LAYERS[l]);t.SetNet(N[net]);b.Add(t)
def via(net,pos):
 for f in F.values():
  for pad in f.Pads():
   if pad.GetAttribute()==p.PAD_ATTRIB_PTH and pad.GetNetname()==net and math.dist(xy(pad.GetPosition()),pos)<min(xy(pad.GetSize()))/2-.1:return
 for t in b.GetTracks():
  if isinstance(t,p.PCB_VIA) and math.dist(xy(t.GetPosition()),pos)<.04 and t.GetNetname()==net:return
 a=p.PCB_VIA(b);a.SetPosition(v(*pos))
 for layer in [p.F_Cu,p.B_Cu]:a.SetWidth(layer,600000)
 a.SetDrill(300000);a.SetViaType(p.VIATYPE_THROUGH);a.SetLayerPair(p.F_Cu,p.B_Cu);a.SetNet(N[net]);b.Add(a)
PADS=[]
for f in F.values():
 for a in f.Pads():
  sx,sy=xy(a.GetSize());ang=math.radians(a.GetOrientationDegrees());PADS.append((a.GetNetname(),xy(a.GetPosition()),abs(sx*math.cos(ang))+abs(sy*math.sin(ang)),abs(sx*math.sin(ang))+abs(sy*math.cos(ang)),a.GetAttribute()!=p.PAD_ATTRIB_SMD))
def rect(arr,pos,sx,sy,margin):
 x,y=pos;x0=max(0,math.ceil((x-sx/2-margin)/S));x1=min(NX,math.floor((x+sx/2+margin)/S)+1);y0=max(0,math.ceil((y-sy/2-margin)/S));y1=min(NY,math.floor((y+sy/2+margin)/S)+1);arr[x0:x1,y0:y1]=True
def segment(arr,A,B,r):
 x0=max(0,math.floor((min(A[0],B[0])-r)/S));x1=min(NX,math.ceil((max(A[0],B[0])+r)/S)+1);y0=max(0,math.floor((min(A[1],B[1])-r)/S));y1=min(NY,math.ceil((max(A[1],B[1])+r)/S)+1)
 if x1<=x0 or y1<=y0:return
 xx,yy=np.ogrid[x0:x1,y0:y1];xx=xx*S;yy=yy*S;dx=B[0]-A[0];dy=B[1]-A[1];den=dx*dx+dy*dy
 t=np.clip(((xx-A[0])*dx+(yy-A[1])*dy)/den,0,1) if den else 0
 arr[x0:x1,y0:y1]|=(xx-A[0]-t*dx)**2+(yy-A[1]-t*dy)**2<=(r+.015)**2

def obstacles(net,w):
 ar=np.zeros((2,NX,NY),dtype=bool);m=.15+w/2+.02
 for n,pos,sx,sy,tht in PADS:
  if n==net:continue
  rect(ar[0],pos,sx,sy,m)
  if tht:
   rect(ar[1],pos,sx,sy,m)
 for t in b.GetTracks():
  if t.GetNetname()==net:continue
  if isinstance(t,p.PCB_VIA):
   for arr in ar:segment(arr,xy(t.GetPosition()),xy(t.GetPosition()),p.ToMM(t.GetWidth(p.F_Cu))/2+m)
  else:segment(ar[LAYERS.index(t.GetLayer())],xy(t.GetStart()),xy(t.GetEnd()),p.ToMM(t.GetWidth())/2+m)
 ar[:,:5,:]=True;ar[:,104:158,:147]=True;ar[:,-5:,:]=True;ar[:,:,:5]=True;ar[:,:,-5:]=True;ar[:,335:,185:]=True
 return ar

def route(net,A,B,la=0,lb=0,w=.2):
 ar=obstacles(net,w);sx,sy=idx(A);gx,gy=idx(B)
 def terminal(pos,layer,x,y):
  if not ar[layer,x,y]:return x,y
  for n,center,px,py,tht in PADS:
   if n!=net or math.dist(center,pos)>.02:continue
   candidates=[(math.dist((xx*S,yy*S),pos),xx,yy) for xx in range(x-2,x+3) for yy in range(y-2,y+3) if 0<=xx<NX and 0<=yy<NY and not ar[layer,xx,yy] and abs(xx*S-center[0])+w/2<px/2 and abs(yy*S-center[1])+w/2<py/2]
   if candidates:_,xx,yy=min(candidates);return xx,yy
  return x,y
 sx,sy=terminal(A,la,sx,sy);gx,gy=terminal(B,lb,gx,gy);start=(la,sx,sy);goal=(lb,gx,gy)
 if ar[start] or ar[goal]:return False
 viaok=np.stack([distance_transform_edt(~a) for a in ar]).min(axis=0)>(.3-w/2+.04)/S
 # No via copper may overlap any SMT land: standard tented vias only.
 for n,pos,px,py,tht in PADS:
  temp=np.zeros((NX,NY),bool)
  if tht:
   segment(temp,pos,pos,.8);viaok[temp]=False
   if n==net:
    inside=np.zeros((NX,NY),bool);segment(inside,pos,pos,min(px,py)/2-.2);viaok[inside&(~ar).all(axis=0)]=True
  else:
   rect(temp,pos,px,py,.45);viaok[temp]=False
 for t in b.GetTracks():
  if isinstance(t,p.PCB_VIA) and t.GetNetname()==net:
   x,y=idx(xy(t.GetPosition()));tmp=np.zeros((NX,NY),bool);segment(tmp,xy(t.GetPosition()),xy(t.GetPosition()),.65);viaok[tmp]=False;viaok[x,y]=True
 def h(z):return max(abs(z[1]-gx),abs(z[2]-gy))+.4142*min(abs(z[1]-gx),abs(z[2]-gy))+(0 if z[0]==lb else 8)
 heap=[(h(start),0,start)];cost={start:0};prev={};closed=set()
 while heap:
  _,g,z=heapq.heappop(heap)
  if z in closed:continue
  if z==goal:
   path=[z]
   while path[-1]!=start:path.append(prev[path[-1]])
   path.reverse();last=path[0];anchor=last;direction=None
   for cur in path[1:]:
    if cur[0]!=last[0]:
     tr(net,[(anchor[1]*S,anchor[2]*S),(last[1]*S,last[2]*S)],w,last[0]);via(net,(cur[1]*S,cur[2]*S));anchor=cur;direction=None
    else:
     d=(cur[1]-last[1],cur[2]-last[2])
     if direction is not None and d!=direction:tr(net,[(anchor[1]*S,anchor[2]*S),(last[1]*S,last[2]*S)],w,last[0]);anchor=last
     direction=d
    last=cur
   tr(net,[(anchor[1]*S,anchor[2]*S),(last[1]*S,last[2]*S)],w,last[0]);tr(net,[A,(sx*S,sy*S)],w,la);tr(net,[(gx*S,gy*S),B],w,lb);return True
  closed.add(z)
  if len(closed)>600000:break
  l,x,y=z;moves=[(l,x+dx,y+dy,c) for dx,dy,c in [(1,0,1),(-1,0,1),(0,1,1),(0,-1,1),(1,1,1.4142),(1,-1,1.4142),(-1,1,1.4142),(-1,-1,1.4142)]]
  if viaok[x,y]:moves +=[(nl,x,y,18) for nl in range(2) if nl!=l]
  for nl,xx,yy,c in moves:
   if not(0<=xx<NX and 0<=yy<NY) or ar[nl,xx,yy]:continue
   if xx!=x and yy!=y and (ar[l,xx,y] or ar[l,x,yy]):continue
   zz=(nl,xx,yy);ng=g+c+(0.05 if nl else 0)
   if ng<cost.get(zz,1e30):cost[zz]=ng;prev[zz]=z;heapq.heappush(heap,(ng+h(zz),ng,zz))
 return False

def treepoints(net):
 out=[]
 for t in b.GetTracks():
  if t.GetNetname()!=net:continue
  if isinstance(t,p.PCB_VIA):out.extend([(xy(t.GetPosition()),l) for l in range(2)])
  else:
   A=xy(t.GetStart());B=xy(t.GetEnd());l=LAYERS.index(t.GetLayer())
   out.extend([((A[0]+f*(B[0]-A[0]),A[1]+f*(B[1]-A[1])),l) for f in np.linspace(0,1,max(2,math.ceil(math.dist(A,B)/.5)))])
 return out

def run():
 # High-di/dt switch loops and local bypasses remain short and hand routed.
 tr('SW1',[pt('U1',11),(49.8,9.5),(50.7,8.325),pt('L1',1)],.3)
 tr('SW2',[pt('U1',9),(49.8,10.5),(50.7,11.675),pt('L1',2)],.3)
 tr('GND',[pt('U1',10),(50.3,10)],.25);via('GND',(50.3,10))
 tr('VIN',[pt('U1',12),(48.825,7.8),(48.3,7.8),pt('U1',13)],.25)
 tr('VIN',[(48.3,7.8),(49.075,7.025),pt('C1',1)],.5)
 tr('REG5V',[pt('U1',8),(48.825,12.2),(48.3,12.2),pt('U1',7)],.25)
 tr('REG5V',[(48.3,12.2),(47.525,12.975),pt('C4',1)],.5)
 tr('TX_ARM',[pt('U4',3),(26.3,5.325),(25.4,6.225)],.18);via('TX_ARM',(25.4,6.225))
 tr('HEARTBEAT',[pt('U4',2),(26.3,4.675),(25.4,3.775)],.18);via('HEARTBEAT',(25.4,3.775))
 tr('EN',[pt('U1',14),(47.775,7.9),(47.6,7.6)],.18);via('EN',(47.6,7.6))
 tr('PS',[pt('U1',1),(45.7,9.25),(45.7,7),pt('R3',2)],.18)
 tr('VAUX',[pt('U1',3),(45.7,10.25),pt('C8',1)],.18)
 tr('MONO_RC',[pt('U4',7),(31.8,4.675),(31.8,2.775),pt('C14',1)],.18)
 tr('MONO_RC',[pt('R12',2),(30.825,2.2),(31.8,2.775)],.18)
 tr('MONO_C',[pt('U4',6),(32,5.325)],.18);via('MONO_C',(32,5.325))
 tr('MONO_C',[pt('C14',2),(33.5,1.225)],.18);via('MONO_C',(33.5,1.225))
 tr('MONO_C',[(32,5.325),(33.5,3.825),(33.5,1.225)],.2,1)
 # Dedicated local returns outside the capacitor lands, then a connected
 # bottom-layer ground tree. No via is hidden in an SMT solder joint.
 tr('GND',[pt('C1',2),(47.525,5.3)],.22);via('GND',(47.525,5.3))
 tr('GND',[pt('C4',2),(50.275,13.5)],.22);via('GND',(50.275,13.5))
 assert route('GND',(47.525,5.3),(50.3,10),1,1,.25)
 assert route('GND',(50.275,13.5),(50.3,10),1,1,.25)
 results=[]
 order=['TX_ARM','HEARTBEAT','FB','GND','SW1','SW2','VIN','BAT+','REG5V','3V3','VAUX','PS','EN','MONO_C','MONO_RC']
 order+=sorted(set(N)-set(order)-{'','GND'})
 for net in order:
  pads=[a for f in F.values() for a in f.Pads() if a.GetNetname()==net]
  if len(pads)<2:continue
  tree=treepoints(net)
  if not tree:a=pads.pop(0);tree=[(xy(a.GetPosition()),0)]
  while pads:
   _,a,B,l=min(((math.dist(xy(a.GetPosition()),pos),a,pos,l) for a in pads for pos,l in tree),key=lambda x:x[0]);A=xy(a.GetPosition());pads.remove(a)
   if math.dist(A,B)<.005:continue
   ok=False
   for w in ([.4,.3,.2,.15] if net in ['VIN','BAT+','REG5V','3V3'] else [.18,.15]):
    if route(net,A,B,0,l,w):ok=True;break
   if not ok:
    for pos,layer in sorted(tree,key=lambda x:math.dist(x[0],A))[::max(1,len(tree)//8)]:
     if route(net,A,pos,0,layer,.15):ok=True;break
   results.append((net,A,ok))
   if ok:tree=treepoints(net)
   else:print('FAILED',net,A,flush=True)
  print('net',net,'done',flush=True);p.SaveBoard(str(FN),b)
 z=p.ZONE(b);z.SetLayer(p.B_Cu);z.SetNet(N['GND']);z.SetLocalClearance(150000);z.SetMinThickness(150000);z.SetPadConnection(p.ZONE_CONNECTION_FULL);z.SetIslandRemovalMode(p.ISLAND_REMOVAL_MODE_ALWAYS);o=z.Outline();o.NewOutline()
 for x,y in [(.3,.3),(88.7,.3),(88.7,18.7),(33.7,18.7),(33.7,33.7),(.3,33.7)]:o.Append(round(x*1e6),round(y*1e6))
 b.Add(z);b.BuildConnectivity();p.ZONE_FILLER(b).Fill(b.Zones());p.SaveBoard(str(FN),b)
 (ROOT/'reports/routing.json').write_text(json.dumps(results,indent=2));print('Remaining',sum(not a[-1] for a in results),flush=True)
if __name__=='__main__':run()

from pathlib import Path
import subprocess,json,re,os
r=Path(__file__).resolve().parents[1];pcb=r/'hardware/CG50-WiFi.kicad_pcb'
env=dict(os.environ,XDG_CACHE_HOME=str(r.parent/'.cache'),XDG_CONFIG_HOME=str(r.parent/'.config'),XDG_DATA_HOME=str(r.parent/'.local'))
for attempt in range(8):
 subprocess.run(['kicad-cli','pcb','drc','--schematic-parity','--format','json','-o',str(r/'reports/DRC.json'),str(pcb)],env=env,check=True)
 d=json.loads((r/'reports/DRC.json').read_text());ids={i['uuid'] for v in d['violations'] if v['type'] in ['track_dangling','via_dangling'] for i in v['items']}
 if not ids:break
 s=pcb.read_text();spans=[]
 for m in re.finditer(r'\((?:segment|via)\b',s):
  depth=0
  for i in range(m.start(),len(s)):
   if s[i]=='(':depth+=1
   elif s[i]==')':
    depth-=1
    if depth==0:
     block=s[m.start():i+1]
     if any(u in block for u in ids):spans.append((m.start(),i+1))
     break
 for a,b in reversed(spans):s=s[:a]+s[b:]
 assert spans,'Could not find reported stub'
 pcb.write_text(s);print('Trim dangling copper:',len(spans))
assert not d['violations'] and not d['unconnected_items'] and not d['schematic_parity'],d
print('FULL DRC AND PARITY CLEAN')

from pathlib import Path
import re
ROOT=Path(__file__).resolve().parents[1];p=ROOT/'hardware/CG50-WiFi.kicad_pcb';s=p.read_text();spans=[]
for m in re.finditer(r'\(via\b',s):
 depth=0
 for i in range(m.start(),len(s)):
  if s[i]=='(':depth+=1
  elif s[i]==')':
   depth-=1
   if not depth:
    block=s[m.start():i+1];a=re.search(r'\(at\s+([-\d.]+)\s+([-\d.]+)\)',block)
    if a and (float(a[1]),float(a[2])) in [(47.6,7.6),(25.4,3.775),(25.4,6.225)]:spans.append((m.start(),i+1))
    break
for a,b in reversed(spans):s=s[:a]+s[b:]
p.write_text(s);print('Removed unnecessary single-layer vias:',len(spans))

from pathlib import Path
import json,gzip,hashlib
r=Path(__file__).resolve().parents[1];original=json.loads((r.parent/'CG50-WiFi-RevE/reference/catalog-selected.json').read_text());parts=json.loads((r/'source/parts.json').read_text());codes={a['lcsc'] for a in parts if not a['manual']};result={c:original['parts'][c] for c in codes if c in original['parts']};lut=json.load(gzip.open(r.parent/'catalog-raw/attributes-lut.json.gz','rt'))
for f in sorted((r.parent/'catalog-raw').glob('components-*.jsonl.gz')):
 if len(result)==len(codes):break
 with gzip.open(f,'rt') as g:
  fields=json.loads(next(g))
  for line in g:
   a=json.loads(line);code=a[fields['lcsc']]
   if code not in codes or code in result:continue
   item={k:a[v] for k,v in fields.items()};item['source_file']=f.name;item['source_sha256']=hashlib.sha256(f.read_bytes()).hexdigest();item['decoded_attributes']={}
   for idx in item.get('attributes',[]):
    key,val=lut[idx];v=val.get('values',{});vv=v.get(val.get('primary')) or next(iter(v.values()),None)
    if vv is not None:item['decoded_attributes'][key]=vv[0]
   result[code]=item
assert set(result)==codes,codes-set(result)
out={k:v for k,v in original.items() if k!='parts'};out['parts']=result;(r/'reference/catalog-selected.json').write_text(json.dumps(out,indent=2,ensure_ascii=False))
for a in parts:
 if a['manual']:continue
 c=result[a['lcsc']];assert c['stock']>0
 if c['mfr'].lower()!=a['mpn'].lower():print('MPN difference',a['ref'],a['mpn'],c['mfr'])
print('Catalog codes',len(result),'lowest stock',min(c['stock'] for c in result.values()))

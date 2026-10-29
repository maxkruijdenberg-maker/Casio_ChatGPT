"""Create a focused customer ZIP without historical releases or developer logs."""
from pathlib import Path
from zipfile import ZipFile,ZIP_DEFLATED
import re,hashlib,json
root=Path(__file__).resolve().parents[1]
version=re.search(r'Versie ([0-9.]+)',(root/'casio/main.c').read_text()).group(1)
manifest=json.loads((root/'dist/sha256.json').read_text())
for name,sha in manifest.items():assert hashlib.sha256((root/'dist'/name).read_bytes()).hexdigest()==sha,name
files={}
def add(path):
 p=root/path;assert p.is_file(),p;files[str(path)]=p
for name in list(manifest)+['sha256.json']:add(Path('dist')/name)
for folder in ['atom','casio']:
 for p in (root/folder).rglob('*'):
  if p.is_file() and not any(part in {'build','__pycache__'} for part in p.relative_to(root).parts):add(p.relative_to(root))
for name in [f'docs/Handleiding-V{version}.pdf',f'docs/Handleiding-V{version}.html',f'docs/V{version}-HANDLEIDING.md',f'docs/V{version}-CONTROLE-VOOR-VERKOOP.md',f'UPGRADE-V{version}.md','docs/LOGO-SOURCE.md','START-HIER.html','tools/flash_windows.py','tools/build-casio.ps1']:
 add(Path(name))
for p in (root/'docs').glob('*LICENSE*'):add(p.relative_to(root))
start=(root/'START-HIER.md').read_text()
start=re.sub(r'Het overzichtelijke klantpakket staat.*?oude versiegidsen\.\n\n','',start,flags=re.S)
archive=root/f'dist/ChatGPT-V{version}-installatie.zip';prefix=f'ChatGPT-V{version}/'
with ZipFile(archive,'w',ZIP_DEFLATED) as z:
 z.writestr(prefix+'START-HIER.md',start)
 z.writestr(prefix+'README.md',start)
 for name,p in sorted(files.items()):z.write(p,prefix+name)
assert ZipFile(archive).testzip() is None
archive.with_suffix('.sha256').write_text(hashlib.sha256(archive.read_bytes()).hexdigest()+'  '+archive.name+'\n')
print(archive)

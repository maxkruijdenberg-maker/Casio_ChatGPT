"""Package sources and existing build artifacts; this script does not compile."""
import hashlib,json,pathlib,zipfile
root=pathlib.Path(__file__).resolve().parents[1]
dist=root/'dist'
expected=['ChatGPT.g3a','ChatGPTAtom.ino.bin','ChatGPTAtom.ino.bootloader.bin','ChatGPTAtom.ino.partitions.bin','boot_app0.bin','ChatGPTC6.ino.bin','ChatGPTC6.ino.bootloader.bin','ChatGPTC6.ino.partitions.bin']
sha={name:hashlib.sha256((dist/name).read_bytes()).hexdigest() for name in expected}
(dist/'sha256.json').write_text(json.dumps(sha,indent=2)+'\n')
archive=root.parent/'ChatGPT-Casio-Atom.zip'
with zipfile.ZipFile(archive,'w',zipfile.ZIP_DEFLATED) as z:
 for p in sorted(root.rglob('*')):
  rel=p.relative_to(root)
  if p.is_file() and not any(x in {'.git','build','__pycache__'} for x in rel.parts):
   z.write(p,pathlib.Path(root.name)/rel)
print(archive)

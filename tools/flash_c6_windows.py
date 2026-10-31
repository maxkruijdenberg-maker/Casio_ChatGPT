"""Windows Rev E only: py tools/flash_c6_windows.py COM5. Never for Atom Lite."""
import hashlib,json,pathlib,subprocess,sys
ROOT=pathlib.Path(__file__).resolve().parents[1]
IMAGES=[('0x0','ChatGPTC6.ino.bootloader.bin'),('0x8000','ChatGPTC6.ino.partitions.bin'),('0xe000','boot_app0.bin'),('0x10000','ChatGPTC6.ino.bin')]
def main():
 if len(sys.argv)!=2 or not sys.argv[1].upper().startswith('COM') or not sys.argv[1][3:].isdigit():
  raise SystemExit('Gebruik: py tools\\flash_c6_windows.py COM5\nAlleen voor Rev E met ESP32-C6-MINI-1-H4 en CH340C.')
 manifest=json.loads((ROOT/'dist/sha256.json').read_text())
 for _,name in IMAGES:
  p=ROOT/'dist'/name
  if hashlib.sha256(p.read_bytes()).hexdigest()!=manifest[name]:raise SystemExit(f'Checksum klopt niet: {name}')
 cmd=[sys.executable,'-m','esptool','--chip','esp32c6','--port',sys.argv[1],'--baud','115200','write-flash','--flash-mode','dio','--flash-freq','80m','--flash-size','4MB']
 for offset,name in IMAGES:cmd.extend([offset,str(ROOT/'dist'/name)])
 subprocess.run(cmd,check=True)
 print('Rev E geflasht. Open ChatGPT op de Casio.')
if __name__=='__main__':main()

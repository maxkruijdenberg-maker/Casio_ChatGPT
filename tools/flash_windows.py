"""Run on Windows: py tools\flash_windows.py COM5. No secret entry on PC."""
import hashlib,json,pathlib,subprocess,sys
ROOT=pathlib.Path(__file__).resolve().parents[1]
IMAGES=[('0x1000','ChatGPTAtom.ino.bootloader.bin'),('0x8000','ChatGPTAtom.ino.partitions.bin'),('0xe000','boot_app0.bin'),('0x10000','ChatGPTAtom.ino.bin')]
def main():
 if len(sys.argv)!=2 or not sys.argv[1].upper().startswith('COM') or not sys.argv[1][3:].isdigit():
  raise SystemExit('Gebruik: py tools\\flash_windows.py COM5\nZoek de juiste COM-poort in Apparaatbeheer. Koppel TX en RX los voor het flashen.')
 manifest=json.loads((ROOT/'dist/sha256.json').read_text())
 for _,name in IMAGES:
  p=ROOT/'dist'/name
  if hashlib.sha256(p.read_bytes()).hexdigest()!=manifest[name]:raise SystemExit(f'Checksum klopt niet: {name}')
 cmd=[sys.executable,'-m','esptool','--chip','esp32','--port',sys.argv[1],'--baud','115200','write-flash']
 for offset,name in IMAGES:cmd.extend([offset,str(ROOT/'dist'/name)])
 subprocess.run(cmd,check=True)
 print('Flashen voltooid. Sluit Arduino Serial Monitor. Verbind TX/RX opnieuw en open ChatGPT op de Casio. F6 opent Instellingen.')
if __name__=='__main__':main()

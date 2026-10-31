from pathlib import Path
import pcbnew as p
r=Path(__file__).resolve().parents[1];fn=r/'hardware/CG50-WiFi.kicad_pcb';b=p.LoadBoard(str(fn));t=b.GetTitleBlock();t.SetTitle('CG50 WiFi - XIAO ESP32-C6 hand solder');t.SetRevision('F');t.SetDate('2026-10-31');t.SetComment(0,'0.6mm / 2 layers / OSP');t.SetComment(1,'Program loose XIAO first. Disconnect J3 before USB.');p.SaveBoard(str(fn),b)

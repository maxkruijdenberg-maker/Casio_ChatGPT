"""Keep the Rev E C6 build aligned with the Atom testbench backend."""
from pathlib import Path
import shutil
root=Path(__file__).resolve().parents[1];src=root/'atom/ChatGPTAtom';dst=root/'esp32c6/ChatGPTC6';dst.mkdir(parents=True,exist_ok=True)
for p in src.glob('*.h'):
 if p.name!='hardware.h':shutil.copy2(p,dst/p.name)
s=(src/'ChatGPTAtom.ino').read_text().replace('V40 Atom Lite, ORIGINAL ESP32-PICO-D4. No sleep, no TX inversion.','V40 Rev E ESP32-C6-MINI-1-H4. No sleep. TX inversion for Q2/Q3.').replace('Select M5Atom (ESP32).','Select ESP32C6 Dev Module, 4MB, Huge APP, USB CDC off.')
s=s.replace('bootId=esp_random();pinMode(TX_PIN,INPUT);','boardInitPins();bootId=esp_random();pinMode(TX_PIN,INPUT);')
s=s.replace('Calc.begin(9600,SERIAL_8N1,RX_PIN,TX_PIN,false);','Calc.begin(9600,SERIAL_8N1,RX_PIN,TX_PIN,false);\n boardUartReady=uart_set_line_inverse(UART_NUM_1,UART_SIGNAL_TXD_INV)==ESP_OK;')
s=s.replace('servicePortal();','boardServiceLease();servicePortal();').replace('if(w_feed(&parser,c,&f)==1)handle(f);','if(w_feed(&parser,c,&f)==1&&boardAcceptFrame(f))handle(f);').replace(' delay(1);\n}', ' boardServiceLease();delay(1);\n}')
s=s.replace('GPIO26 stays UART idle-high while active.','GPIO21 uses TX-only inversion and the gated open-drain output.')
(dst/'ChatGPTC6.ino').write_text(s)
print('Synced C6 backend; GPIO20/21, heartbeat1/arm2 and TX-only inversion retained.')

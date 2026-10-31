"""Netlist and simplified UART RC analysis; NOT a physical/IC SPICE validation."""
from pathlib import Path
import xml.etree.ElementTree as E
import json,re,math
r=Path(__file__).resolve().parents[1];repo=r.parents[1]
root=E.parse(r/'reports/netlist.xml').getroot();nets={}
for n in root.findall('.//nets/net'):
 for node in n.findall('node'):nets[(node.get('ref'),node.get('pin'))]=n.get('name').removeprefix('/')
def expect(ref,pins):
 for pin,net in pins.items():assert nets[(ref,str(pin))]==net,(ref,pin,nets.get((ref,str(pin))),net)
# Independent electrical expectations, rather than copying source/parts.json.
expect('Q1',{1:'GND',2:'VIN',3:'BAT+'})
expect('Q2',{1:'TX_GATE',2:'SINK_LINK',3:'TX_DRAIN'})
expect('Q3',{1:'EN_GATE',2:'GND',3:'SINK_LINK'})
expect('U3',{2:'RX_IN',3:'GND',4:'RX_WAKE',5:'3V3'})
expect('U4',{1:'GND',2:'HEARTBEAT',3:'TX_ARM',4:'GND',5:'TX_ENABLE',6:'MONO_C',7:'MONO_RC',8:'3V3'})
expect('U5',{1:'RX_WAKE',2:'HEARTBEAT',3:'TX_ARM',7:'TX_INVERT',8:'RX_WAKE',12:'3V3',13:'GND',14:'REG5V'})
expect('J3',{1:'BAT+',2:'GND'});expect('J4',{1:'CASIO_TX',2:'CASIO_RX',3:'GND',4:'CASIO_VIO'})
expect('R6',{1:'TX_DRAIN',2:'CASIO_RX'});expect('R11',{1:'CASIO_VIO',2:'CASIO_RX'})
expect('R7',{1:'TX_INVERT',2:'TX_GATE'});expect('R9',{1:'TX_ENABLE',2:'EN_GATE'})
expect('R8',{1:'TX_GATE',2:'GND'});expect('R10',{1:'EN_GATE',2:'GND'})
expect('R13',{1:'TX_ARM',2:'GND'});expect('R14',{1:'HEARTBEAT',2:'GND'})
expect('R12',{1:'3V3',2:'MONO_RC'});expect('C14',{1:'MONO_RC',2:'MONO_C'})
expect('R15',{1:'REG5V',2:'FB'});expect('R16',{1:'FB',2:'GND'})
parts={a['ref']:a for a in json.loads((r/'source/parts.json').read_text())}
def ohms(ref):
 v=parts[ref]['value'];m=re.fullmatch(r'([\d.]+)([RkM])',v);assert m,v
 return float(m[1])*{'R':1,'k':1000,'M':1e6}[m[2]]
rpull=ohms('R11');rseries=ohms('R6')+.2
bit_s=1/9600 # 9600 baud, one start + eight data + one stop bit.
cases=0;minimum_high=1.;maximum_low=0.
# Assumptions: ideal active monostable, summed MOSFET Rds=.2ohm,
# 30%/70% receiver thresholds, no unknown internal Casio resistor/diode.
for vio in [3.,3.3,5.]:
 for capacitance in [1e-12,100e-12,1e-9,5e-9]:
  for byte in range(256):
   v=vio
   for bit in [0]+[(byte>>n)&1 for n in range(8)]+[1]:
    gpio=1-bit # UART TX-only inversion, followed by the open-drain inversion.
    target=vio*rseries/(rpull+rseries) if gpio else vio
    resistance=rpull*rseries/(rpull+rseries) if gpio else rpull
    decay=math.exp(-bit_s/2/(resistance*capacitance))
    sample=target+(v-target)*decay;fraction=sample/vio
    if bit:assert fraction>.7;minimum_high=min(minimum_high,fraction)
    else:assert fraction<.3;maximum_low=max(maximum_low,fraction)
    v=target+(sample-target)*decay
   cases+=1
    # Each byte independently starts from an idle-high line.
# If an internal pull-up exists, the unknown resistance must be measured.
min_total_pullup=rseries*(1/.3-1)
min_internal_pullup=1/(1/min_total_pullup-1/rpull)
ino=(repo/'xiao/ChatGPTXIAO/ChatGPTXIAO.ino').read_text()
assert 'Calc.begin(9600,SERIAL_8N1,RX_PIN,TX_PIN,false)' in ino
assert 'uart_set_line_inverse(UART_NUM_1,UART_SIGNAL_TXD_INV)' in ino
variant=Path('/workspace/casio-tools/esp32-src/variants/XIAO_ESP32C6/variant.cpp').read_text()
assert 'digitalWrite(WIFI_ENABLE, LOW)' in variant and 'digitalWrite(WIFI_ANT_CONFIG, LOW)' in variant
report={'source_hardware_commit':'0a74894','netlist_path_expectations':'pass','uart_ideal_RC_model_byte_cases':cases,'baud':9600,'assumed_capacitance_pf_range':[1,5000],'assumed_VIO_volts':[3,3.3,5],'minimum_sample_high_ratio':minimum_high,'maximum_sample_low_ratio':maximum_low,'assumed_receiver_threshold_ratios':[.3,.7],'if_internal_pullup_present_minimum_ohms_for_this_model':min_internal_pullup,'nominal_output_V_assuming_0_8V_feedback_reference':.8*(1+ohms('R15')/ohms('R16')),'xiao_variant_selects_builtin_antenna':True,'real_Casio_input_load_and_thresholds_measured':False,'manufacturer_converter_SPICE_simulation_performed':False,'converter_datasheet_retrieval':'blocked by HTTP 403; no new primary-datasheet signoff','physical_power_RF_UART_USB_measurement':False,'production_qualified':False}
(r/'reports/functional-electrical-analysis.json').write_text(json.dumps(report,indent=2));print(json.dumps(report,indent=2))

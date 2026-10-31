#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
export XDG_CACHE_HOME=/workspace/pcb-rev-e/.cache XDG_CONFIG_HOME=/workspace/pcb-rev-e/.config XDG_DATA_HOME=/workspace/pcb-rev-e/.local
kicad-cli pcb export gerbers --layers F.Cu,B.Cu,F.Mask,B.Mask,F.Paste,F.SilkS,B.SilkS,Edge.Cuts --subtract-soldermask --use-drill-file-origin -o gerbers/ hardware/CG50-WiFi.kicad_pcb
kicad-cli pcb export drill --drill-origin plot --excellon-separate-th --excellon-units mm -o gerbers/ hardware/CG50-WiFi.kicad_pcb

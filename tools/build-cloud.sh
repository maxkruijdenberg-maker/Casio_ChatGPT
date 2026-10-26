#!/bin/bash
# Uses the verified toolchains retained in the prepared cloud snapshot.
set -euo pipefail
root=$(cd "$(dirname "$0")/.." && pwd)
tools=/workspace/casio-tools
output=/workspace/casio-build
sdk="$tools/prizmsdk/PrizmSDK-win-0.6"
export LD_LIBRARY_PATH="$tools/sh/usr/lib/x86_64-linux-gnu${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
mkdir -p "$output" "$root/dist" "$root/docs/logs"
bash "$root/tools/build-casio.sh"
"$tools/bin/arduino-cli" --config-file "$tools/arduino-cli.yaml" compile --fqbn esp32:esp32:m5stack_atom --jobs 4 --build-property "runtime.tools.esp-x32.path=$tools/asset-esp-x32/xtensa-esp-elf" --build-property "runtime.tools.esp32-arduino-libs.path=$tools/asset-esp32-arduino-libs/esp32-arduino-libs" --build-property "runtime.tools.esptool_py.path=$tools/asset-esptool_py/esptool-linux-amd64" --build-property "runtime.tools.ctags.path=$tools/ctags" --build-path "$output/atom" "$root/atom/ChatGPTAtom"
cp "$output/atom/ChatGPTAtom.ino.bin" "$output/atom/ChatGPTAtom.ino.bootloader.bin" "$output/atom/ChatGPTAtom.ino.partitions.bin" "$root/dist/"
cp "$tools/asset-esp32/esp32-3.3.0/tools/partitions/boot_app0.bin" "$root/dist/"
python3 "$root/tools/package.py"

#!/bin/bash
# Uses the verified toolchains retained in the prepared cloud snapshot.
set -euo pipefail
root=$(cd "$(dirname "$0")/.." && pwd)
tools=/workspace/casio-tools
output=/workspace/casio-build
sdk="$tools/prizmsdk/PrizmSDK-win-0.6"
export LD_LIBRARY_PATH="$tools/sh/usr/lib/x86_64-linux-gnu${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
mkdir -p "$output" "$root/dist" "$root/docs/logs"
flags=(-mb -m4a-nofpu -mhitachi -fleading-underscore -fno-pic -fno-pie -ffreestanding -Os -Wall -Wno-misleading-indentation -Wno-unused-function)
"$tools/sh/usr/bin/sh4-linux-gnu-gcc-14" "${flags[@]}" -I"$sdk/include" -c "$root/casio/main.c" -o "$output/casio-main.o"
"$tools/sh/usr/bin/sh4-linux-gnu-ld" -T "$sdk/toolchain/prizm.x" -L"$sdk/lib" "$output/casio-main.o" -lc -lfxcg "$sdk/lib/gcc/sh3eb-elf/10.1.0/libgcc.a" -Map="$root/docs/logs/ChatGPT.map" -o "$output/ChatGPT.bin"
"$tools/bin/mkg3a" -n basic:ChatGPT -i "uns:$root/casio/unselected.bmp" -i "sel:$root/casio/selected.bmp" "$output/ChatGPT.bin" "$root/dist/ChatGPT.g3a"
"$tools/bin/arduino-cli" --config-file "$tools/arduino-cli.yaml" compile --fqbn esp32:esp32:m5stack_atom --jobs 4 --build-property "runtime.tools.esp-x32.path=$tools/asset-esp-x32/xtensa-esp-elf" --build-property "runtime.tools.esp32-arduino-libs.path=$tools/asset-esp32-arduino-libs/esp32-arduino-libs" --build-property "runtime.tools.esptool_py.path=$tools/asset-esptool_py/esptool-linux-amd64" --build-property "runtime.tools.ctags.path=$tools/ctags" --build-path "$output/atom" "$root/atom/ChatGPTAtom"
cp "$output/atom/ChatGPTAtom.ino.bin" "$output/atom/ChatGPTAtom.ino.bootloader.bin" "$output/atom/ChatGPTAtom.ino.partitions.bin" "$root/dist/"
cp "$tools/asset-esp32/esp32-3.3.0/tools/partitions/boot_app0.bin" "$root/dist/"
python3 "$root/tools/package.py"

#!/bin/bash
# Casio-only build in the prepared cloud toolchain; leaves Atom binaries alone.
set -euo pipefail
root=$(cd "$(dirname "$0")/.." && pwd)
tools=/workspace/casio-tools
output=/workspace/casio-build
sdk="$tools/prizmsdk/PrizmSDK-win-0.6"
export LD_LIBRARY_PATH="$tools/sh/usr/lib/x86_64-linux-gnu${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
mkdir -p "$output" "$root/dist" "$root/docs/logs"
flags=(-mb -m4a-nofpu -mhitachi -fleading-underscore -fno-pic -fno-pie -ffreestanding -Os -Wall -Wno-misleading-indentation -Wno-unused-function)
cc="$tools/sh/usr/bin/sh4-linux-gnu-gcc-14"
"$cc" "${flags[@]}" -I"$sdk/include" -c "$root/casio/main.c" -o "$output/casio-main.o"
"$cc" "${flags[@]}" -I"$sdk/include" -c "$root/casio/native_syscalls.S" -o "$output/native-syscalls.o"
"$tools/sh/usr/bin/sh4-linux-gnu-ld" -T "$sdk/toolchain/prizm.x" -L"$sdk/lib" "$output/casio-main.o" "$output/native-syscalls.o" -lc -lfxcg "$sdk/lib/gcc/sh3eb-elf/10.1.0/libgcc.a" -Map="$root/docs/logs/ChatGPT.map" -o "$output/ChatGPT.bin"
"$tools/bin/mkg3a" -n "basic: " -n internal:ChatGPT  -i "uns:$root/casio/unselected.bmp" -i "sel:$root/casio/selected.bmp" "$output/ChatGPT.bin" "$root/dist/ChatGPT.g3a"

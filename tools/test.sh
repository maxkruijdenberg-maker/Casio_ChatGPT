#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
mkdir -p build/tests
for name in core ledger ui; do
  cc -std=c99 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -Wall -Wno-misleading-indentation -Wno-unused-function -Itests/stubs "tests/test_$name.c" -o "build/tests/test_$name"
  "build/tests/test_$name"
done

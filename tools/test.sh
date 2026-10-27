#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
mkdir -p build/tests
for name in core ledger upload layout store viewport native_menu keyboard ui; do
  cc -std=c99 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -Wall -Wno-misleading-indentation -Wno-unused-function -Itests/stubs "tests/test_$name.c" -o "build/tests/test_$name"
  "build/tests/test_$name"
done

ARDUINOJSON_HOME=${ARDUINOJSON_HOME:-/workspace/casio-tools/ArduinoJson}
c++ -std=c++17 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -Wall -Wno-misleading-indentation -Wno-unused-function -I"$ARDUINOJSON_HOME/src" tests/test_api.cpp -o build/tests/test_api
build/tests/test_api

c++ -std=c++17 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -Wall -Wno-misleading-indentation -Wno-unused-function tests/test_context.cpp -o build/tests/test_context
build/tests/test_context

c++ -std=c++17 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -Wall -Wno-misleading-indentation -Wno-unused-function -I"$ARDUINOJSON_HOME/src" tests/test_phone.cpp -o build/tests/test_phone
build/tests/test_phone

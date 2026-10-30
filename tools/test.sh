#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
mkdir -p build/tests
for name in core ledger upload layout store viewport native_menu keyboard answer ui; do
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

c++ -std=c++17 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -Wall -Wno-misleading-indentation -I"$ARDUINOJSON_HOME/src" tests/test_catalog.cpp -o build/tests/test_catalog
build/tests/test_catalog

c++ -std=c++17 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -Wall -Wno-misleading-indentation -Wno-unused-function tests/test_probe.cpp -o build/tests/test_probe
build/tests/test_probe

cc -std=c99 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -Wall -Wno-misleading-indentation -Wno-unused-function tests/test_conversation.c -o build/tests/test_conversation
build/tests/test_conversation

# Single-GET catalog parser: fixed memory regardless of full account-list size.
c++ -std=c++17 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -Wall -Wno-misleading-indentation -Wno-unused-function tests/test_catalog_stream.cpp -o build/tests/test_catalog_stream
build/tests/test_catalog_stream

# Shared V31 API/account cancellation and total-deadline transport guards.
c++ -std=c++17 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -Wall -Wno-misleading-indentation -Wno-unused-function tests/test_network_operation.cpp -o build/tests/test_network_operation
build/tests/test_network_operation

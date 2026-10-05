#!/bin/bash
# Cross-compiles dist/trc-wap-relay.exe from Linux.
# Needs the mingw-w64 cross compiler (Debian/Ubuntu: apt install gcc-mingw-w64-i686 make).
set -e
cd "$(dirname "$0")"
make "$@"
echo "Built $(pwd)/dist/trc-wap-relay.exe"

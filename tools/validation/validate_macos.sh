#!/usr/bin/env bash
set -euo pipefail
root=$(cd "$(dirname "$0")/../.." && pwd)
"$root/tools/bootstrap/build_macos.sh"
ctest --preset macos-arm64-debug --output-on-failure
"$root/build/macos-arm64-debug/uavsim_info"

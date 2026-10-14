#!/usr/bin/env bash
set -euo pipefail
root=$(cd "$(dirname "$0")/../.." && pwd)
cd "$root"
"$root/tools/bootstrap/build_macos.sh"
ctest --preset macos-arm64-debug --output-on-failure
"$root/build/macos-arm64-debug/uavsim_info"
if [[ ! -x "$root/.venv/bin/python" ]]; then python3.12 -m venv "$root/.venv"; fi
"$root/.venv/bin/python" "$root/tools/validation/python_smoke.py"
"$root/.venv/bin/python" -m pip --version

#!/usr/bin/env bash
set -euo pipefail
root=$(cd "$(dirname "$0")/../.." && pwd)
command -v cmake >/dev/null || { echo 'BLOCKED: cmake is not installed' >&2; exit 127; }
command -v ninja >/dev/null || { echo 'BLOCKED: ninja is not installed' >&2; exit 127; }
cmake --preset macos-arm64-debug
cmake --build --preset macos-arm64-debug

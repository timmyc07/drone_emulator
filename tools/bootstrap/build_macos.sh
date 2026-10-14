#!/usr/bin/env bash
set -euo pipefail
[[ "$(uname -s)" == Darwin && "$(uname -m)" == arm64 ]] || { echo 'BLOCKED: native macOS ARM64 required' >&2; exit 1; }
root=$(cd "$(dirname "$0")/../.." && pwd)
cd "$root"
command -v cmake >/dev/null || { echo 'BLOCKED: cmake is not installed' >&2; exit 127; }
command -v ninja >/dev/null || { echo 'BLOCKED: ninja is not installed' >&2; exit 127; }
cmake --preset macos-arm64-debug
cmake --build --preset macos-arm64-debug

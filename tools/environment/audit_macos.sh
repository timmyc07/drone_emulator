#!/usr/bin/env bash
set +e
set -u
status=PASS
command -v python3 >/dev/null 2>&1 || status=WARNING
json_escape() { python3 -c 'import json,sys; print(json.dumps(sys.stdin.read().strip()))'; }
val() { "$@" 2>/dev/null | head -1; }
mem=$(val sysctl -n hw.memsize)
logical=$(val sysctl -n hw.logicalcpu)
macos=$(val sw_vers -productVersion)
arch=$(val uname -m)
clang=$(val clang++ --version)
git=$(val git --version)
python=$(val python3 --version)
cmake=$(val cmake --version)
ninja=$(val ninja --version)
brew=$(val brew --version)
code=$(val code --version)
xcode_select=$(val xcode-select -p)
avail=$(df -Pk . 2>/dev/null | awk 'NR==2 {print $4}')
cat <<JSON
{
  "status": "$status",
  "host": "mac-audit",
  "system": {"os": "macOS", "version": $(printf '%s' "$macos" | json_escape), "architecture": $(printf '%s' "$arch" | json_escape), "cpu": $(val sysctl -n machdep.cpu.brand_string | json_escape), "logical_processors": ${logical:-null}, "memory_bytes": ${mem:-null}, "available_disk_kb": ${avail:-null}},
  "development": {"git": $(printf '%s' "$git" | json_escape), "cmake": $(printf '%s' "$cmake" | json_escape), "ninja": $(printf '%s' "$ninja" | json_escape), "clang": $(printf '%s' "$clang" | json_escape), "python": $(printf '%s' "$python" | json_escape), "homebrew": $(printf '%s' "$brew" | json_escape), "vs_code": $(printf '%s' "$code" | json_escape), "xcode_select": $(printf '%s' "$xcode_select" | json_escape)},
  "classification": {"cmake": "$(command -v cmake >/dev/null 2>&1 && echo PASS || echo NOT_INSTALLED)", "ninja": "$(command -v ninja >/dev/null 2>&1 && echo PASS || echo NOT_INSTALLED)", "homebrew": "$(command -v brew >/dev/null 2>&1 && echo PASS || echo NOT_INSTALLED)", "full_xcode": "NOT_INSTALLED"}
}
JSON

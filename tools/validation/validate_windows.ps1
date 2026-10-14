$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
Set-Location $root
& "$root/tools/bootstrap/build_windows.ps1"
ctest --preset windows-x64-debug --output-on-failure
& "$root/build/windows-x64-debug/Debug/uavsim_info.exe"

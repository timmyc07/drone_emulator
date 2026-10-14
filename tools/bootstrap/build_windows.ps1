$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
Set-Location $root
cmake --preset windows-x64-debug
cmake --build --preset windows-x64-debug

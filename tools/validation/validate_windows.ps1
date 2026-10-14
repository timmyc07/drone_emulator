[CmdletBinding()]
param()
$ErrorActionPreference = 'Stop'
if (-not $IsWindows) { throw 'Windows is required.' }
$root = Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
& "$root/tools/bootstrap/build_windows.ps1"
& "$root/build/windows-x64-debug/uavsim_info.exe"
if ($LASTEXITCODE -ne 0) { throw 'UAVSim information probe failed.' }
$python = Join-Path $root '.venv/Scripts/python.exe'
if (-not (Test-Path $python)) {
    python -c "import sys; assert sys.version_info[:2] == (3,12), 'Python 3.12 required'"
    if ($LASTEXITCODE -ne 0) { throw 'Python 3.12 check failed.' }
    python -m venv "$root/.venv"
    if ($LASTEXITCODE -ne 0) { throw 'venv creation failed.' }
}
& $python "$root/tools/validation/python_smoke.py"
if ($LASTEXITCODE -ne 0) { throw 'Python smoke test failed.' }
& $python -m pip --version
if ($LASTEXITCODE -ne 0) { throw 'venv pip failed.' }

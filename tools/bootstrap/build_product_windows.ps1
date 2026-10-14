[CmdletBinding()]
param([string]$EngineRoot = 'C:/Program Files/Epic Games/UE_5.8')
$ErrorActionPreference = 'Stop'
if (-not $IsWindows) { throw 'Windows is required.' }
$root = Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
$project = Join-Path $root 'unreal/UAVSim/UAVSim.uproject'
$reportDir = Join-Path $root 'reports/product'
New-Item -ItemType Directory -Force $reportDir | Out-Null
& "$root/tools/bootstrap/build_windows.ps1"
& (Join-Path $EngineRoot 'Engine/Build/BatchFiles/Build.bat') UAVSimEditor Win64 Development "-Project=$project" -WaitMutex -NoHotReloadFromIDE -MaxParallelActions=1 -NoUBA *> (Join-Path $reportDir 'unreal-build.log')
if ($LASTEXITCODE -ne 0) { throw 'Product Editor build failed; inspect reports/product/unreal-build.log.' }
Write-Output 'Portable core and Unreal product Editor build PASS.'

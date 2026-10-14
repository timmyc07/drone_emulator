[CmdletBinding()]
param([string]$EngineRoot = 'C:/Program Files/Epic Games/UE_5.8')
$ErrorActionPreference = 'Stop'
if (-not $IsWindows) { throw 'Windows is required.' }
$root = Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
$project = Join-Path $root 'unreal/UAVSim/UAVSim.uproject'
$reportDir = Join-Path $root 'reports/product'
New-Item -ItemType Directory -Force $reportDir | Out-Null
if (-not (Test-Path (Join-Path $root 'unreal/UAVSim/Content/Maps/UAVSim_Test.umap'))) { throw 'Generate the product test map with tools/unreal/create_product_level.py before packaging.' }
if (-not (Test-Path (Join-Path $root 'unreal/UAVSim/Content/Drone/SM_DefaultDrone.uasset'))) { throw 'Import the user-provided default drone before packaging.' }
# Explicit manifest prevents an incomplete third-party notice bundle from shipping.
$noticeFiles = [ordered]@{
    'third_party/NOTICES.md' = 'README.md'
    'third_party/README.md' = 'VERSIONS.md'
    'third_party/eigen/COPYING.README' = 'eigen/COPYING.README'
    'third_party/eigen/COPYING.MPL2' = 'eigen/COPYING.MPL2'
    'third_party/eigen/COPYING.BSD' = 'eigen/COPYING.BSD'
    'third_party/eigen/COPYING.MINPACK' = 'eigen/COPYING.MINPACK'
    'third_party/nlohmann/LICENSE.MIT' = 'nlohmann-json/LICENSE.MIT'
    'third_party/picosha2/LICENSE' = 'picosha2/LICENSE'
}
foreach ($relative in $noticeFiles.Keys) {
    if (-not (Test-Path -LiteralPath (Join-Path $root $relative) -PathType Leaf)) { throw "Required third-party notice is missing: $relative" }
}
$output = Join-Path $root ('dist/UAVSim-Windows-' + (Get-Date -Format 'yyyyMMdd-HHmmss'))
& (Join-Path $EngineRoot 'Engine/Build/BatchFiles/RunUAT.bat') BuildCookRun "-project=$project" -noP4 -platform=Win64 -clientconfig=Development -build -cook -stage -pak -iostore -archive "-archivedirectory=$output" -prereqs -unattended -utf8output -nocompileeditor -skipbuildeditor '-ubtargs=-MaxParallelActions=1 -NoUBA' *> (Join-Path $reportDir 'package.log')
if ($LASTEXITCODE -ne 0) { throw 'Windows packaging failed; inspect reports/product/package.log.' }
# Copy notices beside the executable, inside the distributable Windows folder.
$package = Join-Path $output 'Windows'
if (-not (Test-Path -LiteralPath (Join-Path $package 'UAVSim.exe') -PathType Leaf)) { throw 'UAT did not produce the expected Windows/UAVSim.exe archive.' }
$notices = Join-Path $package 'ThirdPartyNotices'
foreach ($entry in $noticeFiles.GetEnumerator()) {
    $source = Join-Path $root $entry.Key
    $destination = Join-Path $notices $entry.Value
    New-Item -ItemType Directory -Path (Split-Path $destination -Parent) -Force | Out-Null
    Copy-Item -LiteralPath $source -Destination $destination
    if ((Get-FileHash -LiteralPath $source -Algorithm SHA256).Hash -ne (Get-FileHash -LiteralPath $destination -Algorithm SHA256).Hash) {
        throw "Packaged third-party notice verification failed: $($entry.Value)"
    }
}
Write-Output "Third-party notices verified: $($noticeFiles.Count) files."
Write-Output "Packaged Windows output: $output"
$output | Set-Content (Join-Path $reportDir 'latest-package.txt') -Encoding utf8

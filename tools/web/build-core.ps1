[CmdletBinding()]
param([string]$SdkPath, [string]$CMakePath, [string]$NinjaPath)
$ErrorActionPreference = 'Stop'
if (-not $IsWindows) { throw 'This helper requires Windows; use emcmake/CMake on other hosts.' }
$root = Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
if (-not $SdkPath) { $SdkPath = Join-Path $root 'build/tools/emsdk' }
$sdk = (Resolve-Path $SdkPath).Path
$emcmake = Join-Path $sdk 'upstream/emscripten/emcmake.exe'
$emcc = Join-Path $sdk 'upstream/emscripten/emcc.exe'
if (-not (Test-Path $emcmake)) { throw 'Install and locally activate official Emscripten 6.0.12 in build/tools/emsdk first.' }
if (-not $CMakePath -or -not $NinjaPath) {
    $vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio/Installer/vswhere.exe'
    if (-not (Test-Path $vswhere)) { throw 'Provide -CMakePath and -NinjaPath or install the approved build tools.' }
    $vs = & $vswhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.CMake.Project -property installationPath
    if (-not $vs) { throw 'Visual Studio CMake tools not found; provide explicit paths.' }
    if (-not $CMakePath) { $CMakePath = Join-Path $vs 'Common7/IDE/CommonExtensions/Microsoft/CMake/CMake/bin/cmake.exe' }
    if (-not $NinjaPath) { $NinjaPath = Join-Path $vs 'Common7/IDE/CommonExtensions/Microsoft/CMake/Ninja/ninja.exe' }
}
$previousTemp = $env:EMCC_TEMP_DIR
try {
    # Keep compiler scratch files inside the project, including sandboxed builds.
    $env:EMCC_TEMP_DIR = Join-Path $root 'build/web-temp'
    New-Item -ItemType Directory -Force $env:EMCC_TEMP_DIR | Out-Null
    $version = & $emcc --version
    if ($LASTEXITCODE -ne 0 -or ($version -join "`n") -notmatch '6\.0\.12') { throw 'The web build requires Emscripten 6.0.12.' }
    Write-Host ($version | Select-Object -First 1)
    & $emcmake $CMakePath -S $root -B (Join-Path $root 'build/web') -G Ninja "-DCMAKE_MAKE_PROGRAM=$NinjaPath" -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF -DUAVSIM_BUILD_CPP20_PROBE=OFF
    if ($LASTEXITCODE -ne 0) { throw 'Web core configure failed.' }
    & $CMakePath --build (Join-Path $root 'build/web') --target uavsim_web
    if ($LASTEXITCODE -ne 0) { throw 'Web core build failed.' }
    Write-Host 'PASS: web/generated/uavsim.mjs and uavsim.wasm built from the C++ core.'
} finally { $env:EMCC_TEMP_DIR = $previousTemp }

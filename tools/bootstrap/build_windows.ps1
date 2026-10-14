[CmdletBinding()]
param()
$ErrorActionPreference = 'Stop'
if (-not $IsWindows) { throw 'Windows is required.' }
$root = Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio/Installer/vswhere.exe'
if (-not (Test-Path $vswhere)) { throw 'Visual Studio Installer/vswhere is missing.' }
$vs = & $vswhere -latest -products '*' -version '[17.0,18.0)' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if (-not $vs) { throw 'VS2022 x64 C++ tools are missing; installer changes require approval.' }
$developer = Join-Path $vs 'Common7/Tools/VsDevCmd.bat'
# Import environment into this process only; preserve all machine/user settings.
$lines = & $env:ComSpec /d /s /c "`"`"$developer`" -arch=x64 -host_arch=x64 >nul && set`""
if ($LASTEXITCODE -ne 0) { throw 'Developer environment setup failed.' }
foreach ($line in $lines) {
    if ($line -match '^([^=]+)=(.*)$') { [Environment]::SetEnvironmentVariable($matches[1], $matches[2], 'Process') }
}
$cmake = Join-Path $vs 'Common7/IDE/CommonExtensions/Microsoft/CMake/CMake/bin/cmake.exe'
$ctest = Join-Path (Split-Path $cmake) 'ctest.exe'
Push-Location $root
try {
    & $cmake --preset windows-x64-debug
    if ($LASTEXITCODE -ne 0) { throw 'CMake configure failed.' }
    & $cmake --build --preset windows-x64-debug
    if ($LASTEXITCODE -ne 0) { throw 'C++ build failed.' }
    & $ctest --preset windows-x64-debug
    if ($LASTEXITCODE -ne 0) { throw 'CTest failed.' }
} finally { Pop-Location }

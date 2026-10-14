[CmdletBinding()]
param(
    [string]$EngineRoot = 'C:/Program Files/Epic Games/UE_5.8',
    [switch]$SkipBuild,
    [string]$PackagedExecutable,
    [ValidateRange(30, 3600)][int]$TimeoutSeconds = 600
)
$ErrorActionPreference = 'Stop'
if (-not $IsWindows) { throw 'Windows is required.' }
if ($PackagedExecutable -and (-not [IO.Path]::IsPathFullyQualified($PackagedExecutable) -or
    -not (Test-Path -LiteralPath $PackagedExecutable -PathType Leaf) -or
    [IO.Path]::GetExtension($PackagedExecutable) -ne '.exe')) { throw 'PackagedExecutable must be an existing absolute .exe path.' }
$root = Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
$project = Join-Path $root 'unreal/UAVSim/UAVSim.uproject'
$reportDir = Join-Path $root ('reports/product/unreal-' + [DateTime]::UtcNow.ToString('yyyyMMdd-HHmmss-fffffff'))
New-Item -ItemType Directory -Path $reportDir | Out-Null
$result = [ordered]@{ build = 'NOT_TESTED'; automation = 'NOT_TESTED'; runtime = 'NOT_TESTED'; packagedRuntime = 'NOT_TESTED'; visualInspection = 'NOT_TESTED'; error = $null }
$gate = 'build'
function Invoke-ProductProbe([string]$Executable, [string]$Arguments) {
    $process = Start-Process -FilePath $Executable -ArgumentList $Arguments -WindowStyle Hidden -PassThru
    try {
        if (-not $process.WaitForExit($TimeoutSeconds * 1000)) {
            $process.Kill($true)
            throw 'Product validation timed out; only the launched process tree was terminated.'
        }
        if ($process.ExitCode -ne 0) { throw "Product validation process exited $($process.ExitCode)." }
    } finally { $process.Dispose() }
}
function Assert-ProductRuntime([string]$LogPath) {
    $log = Get-Content -LiteralPath $LogPath -Raw
    if ($log -notmatch 'LogD3D12RHI' -or
        $log -notmatch 'UAVSIM_BEGIN_PLAY core_dt=0\.004 model=SM_DefaultDrone' -or
        $log -notmatch 'UAVSIM_RUNTIME_PASS frames=\d+ altitude=[^\r\n]+ pause/reset/save/replay verified') {
        throw "Required DX12/model/flight/session evidence missing: $LogPath"
    }
}
try {
    if (-not $SkipBuild) {
        & "$root/tools/bootstrap/build_product_windows.ps1" -EngineRoot $EngineRoot
        $result.build = 'PASS'
    }
    $gate = 'automation'
    $common = "`"$project`" -unattended -NoSound -nosplash -NoLiveCoding -NoEpicPortal"
    $automationDir = Join-Path $reportDir 'automation'
    $editorCmd = Join-Path $EngineRoot 'Engine/Binaries/Win64/UnrealEditor-Cmd.exe'
    Invoke-ProductProbe $editorCmd "$common -nullrhi -ExecCmds=`"Automation RunTests UAVSim.Product`" -TestExit=`"Automation Test Queue Empty`" -ReportExportPath=`"$automationDir`" -abslog=`"$reportDir/automation.log`""
    $index = Get-Content (Join-Path $automationDir 'index.json') -Raw | ConvertFrom-Json
    if ($index.failed -ne 0 -or $index.notRun -ne 0 -or $index.inProcess -ne 0) { throw 'Product automation report is incomplete or failed.' }
    foreach ($name in @('UAVSim.Product.AdapterAndLifecycle', 'UAVSim.Product.ReplayValidation')) {
        $test = @($index.tests | Where-Object fullTestPath -eq $name)
        if ($test.Count -ne 1 -or $test[0].state -ne 'Success' -or $test[0].errors -ne 0) { throw "Automation acceptance failed or absent: $name" }
    }
    if (@($index.tests | Where-Object { $_.state -ne 'Success' -or $_.errors -gt 0 }).Count) { throw 'Product automation contains failures.' }
    $result.automation = 'PASS'
    $gate = 'runtime'
    $runtime = '-UAVSimSmoke -dx12 -windowed -ResX=1920 -ResY=1080 -unattended -NoSound -NoEpicPortal -ExecCmds="sg.ViewDistanceQuality 1,sg.ShadowQuality 1,sg.EffectsQuality 1,r.ScreenPercentage 50,t.MaxFPS 60"'
    $gameLog = Join-Path $reportDir 'runtime.log'
    Invoke-ProductProbe (Join-Path $EngineRoot 'Engine/Binaries/Win64/UnrealEditor.exe') "$common /Game/Maps/UAVSim_Test -game $runtime -abslog=`"$gameLog`""
    Assert-ProductRuntime $gameLog
    $result.runtime = 'PASS'
    if ($PackagedExecutable) {
        $gate = 'packagedRuntime'
        $packaged = (Resolve-Path -LiteralPath $PackagedExecutable).Path
        $packageLog = Join-Path $reportDir 'packaged-runtime.log'
        Invoke-ProductProbe $packaged "$runtime -abslog=`"$packageLog`""
        Assert-ProductRuntime $packageLog
        $result.packagedRuntime = 'PASS'
    }
    Write-Output "Product automation and DX12 flight/session validation PASS; evidence: $reportDir"
} catch {
    $result[$gate] = 'FAIL'
    $result.error = $_.Exception.Message
    throw
} finally {
    $result | ConvertTo-Json | Set-Content (Join-Path $reportDir 'validation.json') -Encoding utf8
}

[CmdletBinding()]
param(
    [string]$EngineRoot = 'C:/Program Files/Epic Games/UE_5.8',
    [switch]$SkipBuild,
    [int]$TimeoutSeconds = 600
)
$ErrorActionPreference = 'Stop'
if (-not $IsWindows) { throw 'Windows is required.' }
$root = Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
$project = Join-Path $root 'unreal/P0Probe/P0Probe.uproject'
$reportDir = Join-Path $root 'reports/unreal'
New-Item -ItemType Directory -Force $reportDir | Out-Null
$result = [ordered]@{ startedUtc = [DateTime]::UtcNow.ToString('o'); build = 'NOT_TESTED'; automation = 'NOT_TESTED'; runtime = 'NOT_TESTED'; graphicalEditor = 'NOT_TESTED'; visualInspection = 'NOT_TESTED'; error = $null; measurements = @() }
function Invoke-EditorProbe([string]$Name, [string]$Arguments, [string]$Editor) {
    $timer = [Diagnostics.Stopwatch]::StartNew()
    $process = Start-Process -FilePath $Editor -ArgumentList $Arguments -WindowStyle Hidden -PassThru
    $peak = 0L
    $cpuSeconds = 0.0
    $gpuSamples = @()
    $nextGpuSample = 0.0
    try {
        while (-not $process.HasExited) {
            $process.Refresh()
            $peak = [Math]::Max($peak, $process.WorkingSet64)
            $cpuSeconds = $process.TotalProcessorTime.TotalSeconds
            if ($timer.Elapsed.TotalSeconds -ge $nextGpuSample) {
                $gpu = & nvidia-smi --query-gpu=memory.used,utilization.gpu --format=csv,noheader,nounits 2>$null
                if ($LASTEXITCODE -eq 0) { $gpuSamples += [ordered]@{ seconds = $timer.Elapsed.TotalSeconds; memoryMiBAndUtilizationPercent = "$gpu" } }
                $nextGpuSample = $timer.Elapsed.TotalSeconds + 1
            }
            if ($timer.Elapsed.TotalSeconds -gt $TimeoutSeconds) {
                $process.Kill($true)
                throw "$Name timed out; only the launched process tree was terminated."
            }
            Start-Sleep -Milliseconds 200
        }
        $process.WaitForExit()
        $result.measurements += [ordered]@{ name = $Name; seconds = $timer.Elapsed.TotalSeconds; sampledProcessWorkingSetBytes = $peak; lastObservedCpuSeconds = $cpuSeconds; hostGpuSamples = $gpuSamples; exitCode = $process.ExitCode }
        if ($process.ExitCode -ne 0) { throw "$Name exited $($process.ExitCode)." }
    } finally { $process.Dispose() }
}
try {
    $version = Get-Content (Join-Path $EngineRoot 'Engine/Build/Build.version') -Raw | ConvertFrom-Json
    if ($version.MajorVersion -ne 5 -or $version.MinorVersion -ne 8) { throw 'Fixture requires user-selected UE 5.8.' }
    if (-not $SkipBuild) {
        & (Join-Path $EngineRoot 'Engine/Build/BatchFiles/Build.bat') P0ProbeEditor Win64 Development "-Project=$project" -WaitMutex -NoHotReloadFromIDE -MaxParallelActions=2 *> (Join-Path $reportDir 'build.log')
        if ($LASTEXITCODE -ne 0) { $result.build = 'FAIL'; throw 'Unreal C++ build failed; inspect reports/unreal/build.log.' }
        $result.build = 'PASS'
    }
    $editor = Join-Path $EngineRoot 'Engine/Binaries/Win64/UnrealEditor-Cmd.exe'
    $autoDir = Join-Path $reportDir ('automation-' + (Get-Date -Format 'yyyyMMdd-HHmmss'))
    $autoLog = Join-Path $reportDir 'automation.log'
    $common = "`"$project`" -unattended -nosplash -NoSound -NoLiveCoding -NoEpicPortal"
    $medium = 'sg.ViewDistanceQuality 1,sg.AntiAliasingQuality 1,sg.ShadowQuality 1,sg.GlobalIlluminationQuality 1,sg.ReflectionQuality 1,sg.PostProcessQuality 1,sg.TextureQuality 1,sg.EffectsQuality 1,sg.FoliageQuality 1,sg.ShadingQuality 1,sg.LandscapeQuality 1'
    Invoke-EditorProbe 'Editor automation' "$common -nullrhi -ExecCmds=`"Automation RunTests UAVSim.P0.LevelActor`" -TestExit=`"Automation Test Queue Empty`" -ReportExportPath=`"$autoDir`" -abslog=`"$autoLog`"" $editor
    $index = Get-Content (Join-Path $autoDir 'index.json') -Raw | ConvertFrom-Json
    $test = @($index.tests | Where-Object { $_.fullTestPath -eq 'UAVSim.P0.LevelActor' })
    if ($test.Count -ne 1 -or $test[0].state -ne 'Success') { $result.automation = 'FAIL'; throw 'Expected automation test did not succeed.' }
    $result.automation = 'PASS'
    # Run the saved scene with the real D3D12 renderer, separately from headless tests.
    $gameLog = Join-Path $reportDir 'runtime.log'
    $graphical = Join-Path $EngineRoot 'Engine/Binaries/Win64/UnrealEditor.exe'
    Invoke-EditorProbe 'DX12 simple scene' "$common /Game/P0Smoke -game -dx12 -windowed -ResX=1920 -ResY=1080 -ExecCmds=`"$medium`" -abslog=`"$gameLog`"" $graphical
    $log = Get-Content $gameLog -Raw
    if ($log -notmatch 'P0_BEGIN_PLAY' -or $log -notmatch 'P0_RUNTIME_PASS ticks=180' -or $log -notmatch 'LogD3D12RHI') {
        $result.runtime = 'FAIL'; throw 'Real level BeginPlay/ticks/D3D12 evidence missing.'
    }
    $result.runtime = 'PASS'
    $uiDir = Join-Path $reportDir ('graphical-' + (Get-Date -Format 'yyyyMMdd-HHmmss'))
    $uiLog = Join-Path $reportDir 'graphical-editor.log'
    Invoke-EditorProbe 'Graphical Editor startup and level' "$common -dx12 -windowed -ResX=1920 -ResY=1080 -ExecCmds=`"$medium,Automation RunTests UAVSim.P0.LevelActor`" -TestExit=`"Automation Test Queue Empty`" -ReportExportPath=`"$uiDir`" -abslog=`"$uiLog`"" $graphical
    $uiReport = Get-Content (Join-Path $uiDir 'index.json') -Raw | ConvertFrom-Json
    $uiTest = @($uiReport.tests | Where-Object { $_.fullTestPath -eq 'UAVSim.P0.LevelActor' })
    if ($uiTest.Count -ne 1 -or $uiTest[0].state -ne 'Success') { $result.graphicalEditor = 'FAIL'; throw 'Graphical Editor test failed.' }
    $result.graphicalEditor = 'PASS'
    Write-Output 'Unreal compiler, Editor automation, DX12 level runtime and graphical Editor startup PASS. Manual visual inspection remains NOT_TESTED.'
} catch {
    $result.error = $_.Exception.Message
    Write-Warning $result.error
    throw
} finally {
    $result.finishedUtc = [DateTime]::UtcNow.ToString('o')
    $result | ConvertTo-Json -Depth 8 | Set-Content (Join-Path $reportDir 'validation.json') -Encoding utf8
}

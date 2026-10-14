[CmdletBinding()]
param([switch]$SkipBuild)
$ErrorActionPreference = 'Stop'
if (-not $IsWindows) { throw 'Windows is required; use the equivalent headless commands on macOS.' }
$root = Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
if (-not $SkipBuild) { & "$root/tools/bootstrap/build_windows.ps1" }
$exe = Join-Path $root 'build/windows-x64-debug/uavsim_headless.exe'
$vehicle = Join-Path $root 'configs/vehicles/generic_quadrotor_x01.json'
$sessionRoot = Join-Path $root ('reports/product/' + [DateTime]::UtcNow.ToString('yyyyMMdd-HHmmss-fffffff'))
New-Item -ItemType Directory -Path $sessionRoot -Force | Out-Null
Start-Transcript -Path (Join-Path $sessionRoot 'validation.log') | Out-Null
try {
    & $exe validate --vehicle $vehicle
    if ($LASTEXITCODE -ne 0) { throw 'Vehicle configuration validation failed.' }
    & $exe hover --vehicle $vehicle --duration 30 --output (Join-Path $sessionRoot 'hover')
    if ($LASTEXITCODE -ne 0) { throw '30-second hover acceptance failed.' }
    & $exe replay --vehicle $vehicle --session (Join-Path $sessionRoot 'hover') --output (Join-Path $sessionRoot 'replay')
    if ($LASTEXITCODE -ne 0) { throw 'Recorded-input replay failed.' }
    & $exe scenario --scenario takeoff-land --vehicle $vehicle --duration 30 --output (Join-Path $sessionRoot 'takeoff-land')
    if ($LASTEXITCODE -ne 0) { throw 'Takeoff/landing acceptance failed.' }
    $metadata = Get-Content -Raw -LiteralPath (Join-Path $sessionRoot 'hover/metadata.json') | ConvertFrom-Json
    if ($metadata.result -ne 'PASS' -or $metadata.steps -ne 7500 -or $metadata.max_hover_altitude_error_m -ge 0.2) { throw 'Hover metadata does not satisfy acceptance.' }
    $rows = @(Import-Csv -LiteralPath (Join-Path $sessionRoot 'hover/telemetry.csv'))
    if ($rows.Count -ne 7501) { throw 'Telemetry row count must include initial state and 7500 steps.' }
    $hash = (Get-FileHash -LiteralPath $vehicle -Algorithm SHA256).Hash.ToLowerInvariant()
    if ($hash -ne $metadata.configuration_sha256) { throw 'Configuration SHA256 does not match external verification.' }
    & $exe hover --vehicle $vehicle --duration NaN --output (Join-Path $sessionRoot 'invalid-duration')
    if ($LASTEXITCODE -eq 0) { throw 'Nonfinite duration was accepted.' }
    & $exe hover --vehicle $vehicle --duration 1 --output (Join-Path $sessionRoot 'hover')
    if ($LASTEXITCODE -eq 0) { throw 'Existing session was overwritten.' }
    $badHash = Join-Path $sessionRoot 'invalid-hash-recording'
    New-Item -ItemType Directory -Path $badHash | Out-Null
    $metadata.configuration_sha256 = 'invalid'
    $metadata | ConvertTo-Json -Depth 10 | Set-Content -LiteralPath (Join-Path $badHash 'metadata.json') -Encoding utf8
    & $exe replay --vehicle $vehicle --session $badHash --output (Join-Path $sessionRoot 'invalid-hash-replay')
    if ($LASTEXITCODE -eq 0) { throw 'Incompatible configuration hash was accepted.' }
    $badState = Join-Path $sessionRoot 'invalid-state-recording'
    New-Item -ItemType Directory -Path $badState | Out-Null
    Copy-Item -LiteralPath (Join-Path $sessionRoot 'hover/metadata.json') -Destination (Join-Path $badState 'metadata.json')
    $record = Get-Content -LiteralPath (Join-Path $sessionRoot 'hover/inputs.jsonl') -TotalCount 1 | ConvertFrom-Json
    $record.state.position_m[0] = 1.0
    $record | ConvertTo-Json -Depth 10 -Compress | Set-Content -LiteralPath (Join-Path $badState 'inputs.jsonl') -Encoding utf8
    & $exe replay --vehicle $vehicle --session $badState --output (Join-Path $sessionRoot 'invalid-state-replay')
    if ($LASTEXITCODE -eq 0) { throw 'Changed recorded trajectory was accepted.' }
    $badNul = Join-Path $sessionRoot 'invalid-nul-recording'
    New-Item -ItemType Directory -Path $badNul | Out-Null
    Copy-Item -LiteralPath (Join-Path $sessionRoot 'hover/metadata.json') -Destination (Join-Path $badNul 'metadata.json')
    $recordingText = [IO.File]::ReadAllText((Join-Path $sessionRoot 'hover/inputs.jsonl'))
    $firstNewline = $recordingText.IndexOf("`n")
    if ($firstNewline -lt 0) { throw 'Expected a multiline input recording.' }
    $corruptRecording = $recordingText.Insert($firstNewline, ([string][char]0 + 'junk'))
    [IO.File]::WriteAllText((Join-Path $badNul 'inputs.jsonl'), $corruptRecording, [Text.UTF8Encoding]::new($false))
    $nulResult = & $exe replay --vehicle $vehicle --session $badNul --output (Join-Path $sessionRoot 'invalid-nul-replay') 2>&1
    $nulExitCode = $LASTEXITCODE
    $nulResult | Write-Output
    if ($nulExitCode -eq 0 -or ($nulResult -join "`n") -notmatch 'embedded NUL byte') { throw 'Embedded NUL replay corruption was not explicitly rejected.' }
    Write-Output "PASS product headless validation; evidence: $sessionRoot"
} finally { Stop-Transcript | Out-Null }
exit 0

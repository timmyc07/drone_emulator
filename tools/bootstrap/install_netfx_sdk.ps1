[CmdletBinding()]
param()
$ErrorActionPreference = 'Stop'
$root = Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
New-Item -ItemType Directory -Force (Join-Path $root 'reports/install') | Out-Null
$report = Join-Path $root 'reports/install/vs-netfx.json'
$result = [ordered]@{ startedUtc = [DateTime]::UtcNow.ToString('o'); result = 'RUNNING'; exitCode = $null; sdkPresent = $false; error = $null }
try {
    $principal = [Security.Principal.WindowsPrincipal]::new([Security.Principal.WindowsIdentity]::GetCurrent())
    if (-not $principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)) { throw 'Approved administrator process required.' }
    $sdkKey = 'HKLM:/SOFTWARE/WOW6432Node/Microsoft/Microsoft SDKs/NETFXSDK/4.8'
    if (-not (Test-Path $sdkKey)) {
        $directory = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio/Installer'
        $vs = & (Join-Path $directory 'vswhere.exe') -latest -products '*' -version '[17.0,18.0)' -format json | ConvertFrom-Json
        if (-not $vs.installationPath -or -not $vs.channelId) { throw 'VS2022 instance/channel not found.' }
        Write-Host 'Adding only the approved .NET Framework 4.8 SDK; no automatic restart.'
        $arguments = "modify --installPath `"$($vs.installationPath)`" --channelId $($vs.channelId) --add Microsoft.Net.Component.4.8.SDK --quiet --norestart"
        $process = Start-Process (Join-Path $directory 'setup.exe') -ArgumentList $arguments -WindowStyle Hidden -Wait -PassThru -WorkingDirectory $root
        $result.exitCode = $process.ExitCode
        if ($process.ExitCode -notin @(0, 3010)) { throw "VS component installer exited $($process.ExitCode)." }
    }
    $sdk = (Get-ItemProperty -LiteralPath $sdkKey).KitsInstallationFolder
    $result.sdkPresent = Test-Path (Join-Path $sdk 'Include/um/mscoree.h')
    if (-not $result.sdkPresent) { throw 'Installer returned but required mscoree.h is missing.' }
    $result.result = 'PASS'
} catch {
    $result.result = 'FAIL'
    $result.error = $_.Exception.Message
    Write-Warning $result.error
    throw
} finally {
    $result.finishedUtc = [DateTime]::UtcNow.ToString('o')
    $result | ConvertTo-Json | Set-Content -LiteralPath $report -Encoding utf8
}

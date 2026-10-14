[CmdletBinding()]
param([switch]$ElevatedStage)

$ErrorActionPreference = 'Stop'
if ([Environment]::OSVersion.Platform -ne 'Win32NT') { throw 'Windows is required.' }
$root = Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
$installDir = Join-Path $root 'reports/install'
$null = New-Item -ItemType Directory -Path $installDir -Force
$statusPath = Join-Path $installDir 'wsl-prepare.json'
$status = [ordered]@{
    startedUtc = [DateTime]::UtcNow.ToString('o')
    stage = $(if ($ElevatedStage) { 'install' } else { 'download' })
    result = 'RUNNING'; version = $null; url = $null; msi = $null
    signature = $null; msiExitCode = $null; dismExitCode = $null
    rebootRequired = $false; ubuntu = 'DEFERRED'; error = $null
}
try {
    if ($ElevatedStage) {
        $principal = New-Object Security.Principal.WindowsPrincipal([Security.Principal.WindowsIdentity]::GetCurrent())
        if (-not $principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)) {
            throw 'ElevatedStage requires an administrator process; no elevation is launched by this script.'
        }
        $prepared = Get-Content -LiteralPath $statusPath -Raw | ConvertFrom-Json
        if ($prepared.result -ne 'READY' -or $prepared.stage -ne 'download') { throw 'Run the download stage successfully first.' }
        $status.version = $prepared.version
        $status.url = $prepared.url
        $status.msi = $prepared.msi
        $msiPath = [IO.Path]::GetFullPath($prepared.msi)
        if ((Split-Path $msiPath -Parent) -ne [IO.Path]::GetFullPath($installDir) -or
            (Split-Path $msiPath -Leaf) -notmatch '^wsl\.[0-9.]+\.x64\.msi$') {
            throw 'Prepared installer must be a WSL x64 MSI inside reports/install.'
        }
    } else {
        Write-Host 'Resolving the official stable WSL release (no installation).'
        $release = Invoke-RestMethod -Uri 'https://api.github.com/repos/microsoft/WSL/releases/latest' -TimeoutSec 60
        if ($release.prerelease -or $release.draft) { throw 'GitHub returned a non-stable release.' }
        $assets = @($release.assets | Where-Object { $_.name -match '^wsl\.[0-9.]+\.x64\.msi$' })
        if ($assets.Count -ne 1) { throw 'Expected exactly one official WSL x64 MSI asset.' }
        $asset = $assets[0]
        if ($asset.browser_download_url -notlike 'https://github.com/microsoft/WSL/releases/download/*') {
            throw 'Unexpected WSL download origin.'
        }
        $status.version = $release.tag_name
        $status.url = $asset.browser_download_url
        $msiPath = Join-Path $installDir $asset.name
        $status.msi = $msiPath
        if (-not (Test-Path -LiteralPath $msiPath)) {
            $partialPath = "$msiPath.partial"
            Invoke-WebRequest -Uri $status.url -OutFile $partialPath -TimeoutSec 900
            Move-Item -LiteralPath $partialPath -Destination $msiPath
        }
    }
    $signature = Get-AuthenticodeSignature -LiteralPath $msiPath
    $status.signature = [ordered]@{ status = [string]$signature.Status; subject = $signature.SignerCertificate.Subject }
    if ($signature.Status -ne 'Valid' -or $signature.SignerCertificate.Subject -notmatch '(^|,\s*)O=Microsoft Corporation(,|$)') {
        throw 'Installer must have a valid Microsoft Corporation Authenticode signature.'
    }
    if ($ElevatedStage) {
        Write-Host "Installing verified WSL $($status.version); automatic restart is disabled."
        $msiLog = Join-Path $installDir 'wsl-msi.log'
        $process = Start-Process -FilePath 'msiexec.exe' -ArgumentList "/i `"$msiPath`" /qn /norestart /log `"$msiLog`"" -WindowStyle Hidden -Wait -PassThru
        $status.msiExitCode = $process.ExitCode
        if ($process.ExitCode -notin @(0, 3010)) { throw "WSL MSI failed with exit code $($process.ExitCode)." }
        if ($process.ExitCode -eq 3010) { $status.rebootRequired = $true }
        Write-Host 'Enabling VirtualMachinePlatform without restarting Windows.'
        $dismLog = Join-Path $installDir 'wsl-dism.log'
        & dism.exe /online /enable-feature /featurename:VirtualMachinePlatform /all /norestart "/LogPath:$dismLog" *> (Join-Path $installDir 'wsl-dism-output.txt')
        $status.dismExitCode = $LASTEXITCODE
        if ($LASTEXITCODE -notin @(0, 3010)) { throw "VirtualMachinePlatform failed with exit code $LASTEXITCODE." }
        if ($LASTEXITCODE -eq 3010) { $status.rebootRequired = $true }
        $feature = Get-WindowsOptionalFeature -Online -FeatureName VirtualMachinePlatform
        if ([string]$feature.State -match 'Pending') { $status.rebootRequired = $true }
        $status.result = $(if ($status.rebootRequired) { 'REBOOT_PENDING' } else { 'INSTALLED' })
        Write-Host "WSL preparation: $($status.result). Ubuntu initialization remains deferred; no restart was issued."
    } else {
        $status.result = 'READY'
        Write-Host "Verified WSL $($status.version). Run this script with -ElevatedStage in an approved administrator process."
    }
} catch {
    $status.result = 'FAIL'
    $status.error = $_.Exception.Message
    Write-Warning $status.error
    throw
} finally {
    $status.finishedUtc = [DateTime]::UtcNow.ToString('o')
    $status | ConvertTo-Json -Depth 5 | Set-Content -LiteralPath $statusPath -Encoding utf8
}

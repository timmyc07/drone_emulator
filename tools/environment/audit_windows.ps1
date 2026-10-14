param(
    [string]$ReportDirectory = (Join-Path $PSScriptRoot '../../reports/environment')
)

# Inspect the host without installing software or changing machine configuration.
$ErrorActionPreference = 'Stop'
$checks = [ordered]@{}
function Add-Check([string]$Name, [scriptblock]$Action, [string]$SuccessStatus = 'PASS') {
    try {
        $data = & $Action
        $status = $SuccessStatus
        if ($null -eq $data -or @($data).Count -eq 0) { $status = 'NOT_INSTALLED' }
        if ($data -is [System.Collections.IDictionary] -and $data.Contains('exitCode') -and $data.exitCode -ne 0) {
            $output = "$($data.stdout) $($data.stderr)"
            $status = if ($output -match 'E_ACCESSDENIED|存取被拒|access.+denied') { 'NOT_TESTED' } elseif ($output -match '未安裝|not installed') { 'NOT_INSTALLED' } elseif ($Name -like 'wsl_*' -or $Name -eq 'pythonRegistered') { 'WARNING' } else { 'FAIL' }
        }
        if ($Name -eq 'pythonRegistered' -and "$($data.stdout) $($data.stderr)" -match 'No installed Pythons') { $status = 'WARNING' }
        if ($Name -eq 'unreal' -and @($data.engines).Count -eq 0) { $status = 'NOT_INSTALLED' }
        $checks[$Name] = [ordered]@{ status = $status; data = $data }
    } catch {
        $message = $_.Exception.Message
        $status = if ($message -match 'not on PATH') { 'WARNING' } elseif ($message -match 'not found|is missing') { 'NOT_INSTALLED' } else { 'NOT_TESTED' }
        $checks[$Name] = [ordered]@{ status = $status; error = $message }
    }
}
function Invoke-Version([string]$Path, [string]$Arguments, [switch]$Unicode) {
    $start = [System.Diagnostics.ProcessStartInfo]::new()
    $start.FileName = $Path
    $start.Arguments = $Arguments
    $start.UseShellExecute = $false
    $start.CreateNoWindow = $true
    $start.RedirectStandardOutput = $true
    $start.RedirectStandardError = $true
    if ($Unicode) {
        $start.StandardOutputEncoding = [System.Text.Encoding]::Unicode
        $start.StandardErrorEncoding = [System.Text.Encoding]::Unicode
    }
    $process = [System.Diagnostics.Process]::new()
    $process.StartInfo = $start
    try {
        [void]$process.Start()
        $stdoutTask = $process.StandardOutput.ReadToEndAsync()
        $stderrTask = $process.StandardError.ReadToEndAsync()
        if (-not $process.WaitForExit(30000)) {
            $process.Kill()
            throw "Version/status command timed out after 30 seconds: $Path"
        }
        [ordered]@{ path = $Path; exitCode = $process.ExitCode; stdout = $stdoutTask.GetAwaiter().GetResult().Trim(); stderr = $stderrTask.GetAwaiter().GetResult().Trim() }
    } finally { $process.Dispose() }
}

Add-Check os {
    $version = Get-ItemProperty 'HKLM:/SOFTWARE/Microsoft/Windows NT/CurrentVersion'
    [ordered]@{ registryProductName = $version.ProductName; displayVersion = $version.DisplayVersion; build = "$($version.CurrentBuild).$($version.UBR)"; is64Bit = [Environment]::Is64BitOperatingSystem; note = 'Registry ProductName may retain a legacy Windows 10 name on Windows 11.' }
}
Add-Check cpu {
    [ordered]@{ name = (Get-ItemProperty 'HKLM:/HARDWARE/DESCRIPTION/System/CentralProcessor/0' -Name ProcessorNameString).ProcessorNameString.Trim(); logicalProcessors = [Environment]::ProcessorCount }
}
Add-Check cimHardware {
    Get-CimInstance Win32_ComputerSystem | Select-Object TotalPhysicalMemory, HypervisorPresent
}
Add-Check memory {
    if (-not ('UavAudit.NativeMemory' -as [type])) {
        Add-Type -TypeDefinition @'
using System;
using System.Runtime.InteropServices;
namespace UavAudit {
    [StructLayout(LayoutKind.Sequential)]
    public struct MemoryStatus {
        public uint Length, Load;
        public ulong TotalPhysical, AvailablePhysical, TotalPageFile, AvailablePageFile,
            TotalVirtual, AvailableVirtual, AvailableExtendedVirtual;
    }
    public static class NativeMemory {
        [DllImport("kernel32.dll", SetLastError=true)]
        public static extern bool GlobalMemoryStatusEx(ref MemoryStatus status);
    }
}
'@
    }
    $memory = [UavAudit.MemoryStatus]::new()
    $memory.Length = [Runtime.InteropServices.Marshal]::SizeOf($memory)
    if (-not [UavAudit.NativeMemory]::GlobalMemoryStatusEx([ref]$memory)) { throw 'GlobalMemoryStatusEx failed.' }
    [ordered]@{ source = 'GlobalMemoryStatusEx fallback'; totalPhysicalBytes = $memory.TotalPhysical; availablePhysicalBytes = $memory.AvailablePhysical }
}
Add-Check drives {
    @([System.IO.DriveInfo]::GetDrives() | Where-Object { $_.IsReady -and $_.DriveType -eq 'Fixed' } | Select-Object Name, DriveFormat, TotalSize, AvailableFreeSpace)
}
Add-Check shell { [ordered]@{ version = $PSVersionTable.PSVersion.ToString(); edition = $PSVersionTable.PSEdition; executable = (Get-Process -Id $PID).Path } }
foreach ($presenceTool in @('code', 'gh', 'codex')) {
    Add-Check "presence_$presenceTool" {
        $command = Get-Command $presenceTool -ErrorAction SilentlyContinue
        if (-not $command) { throw "$presenceTool is not on PATH; installation elsewhere is not excluded." }
        [ordered]@{ path = $command.Source; commandType = "$($command.CommandType)"; scope = 'Presence only; application was not launched.' }
    }
}
Add-Check projectVenv {
    $venv = Join-Path $PSScriptRoot '../../.venv/Scripts/python.exe'
    if (-not (Test-Path -LiteralPath $venv)) { throw 'Project venv Python not found.' }
    Invoke-Version ([IO.Path]::GetFullPath($venv)) '--version'
}
Add-Check pythonRegistered {
    $launcher = Get-Command py -ErrorAction SilentlyContinue
    if (-not $launcher) { throw 'Python launcher not found.' }
    Invoke-Version $launcher.Source '-0p'
}

$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio/Installer/vswhere.exe'
$vsRoots = @()
Add-Check visualStudio {
    if (-not (Test-Path -LiteralPath $vswhere)) { throw 'vswhere.exe not found.' }
    $result = Invoke-Version $vswhere '-all -products * -format json'
    if ($result.exitCode -ne 0) { throw "vswhere exited $($result.exitCode): $($result.stderr)" }
    $instances = @($result.stdout | ConvertFrom-Json)
    $script:vsRoots = @($instances | ForEach-Object installationPath)
    @($instances | Select-Object installationPath, installationVersion, isComplete, isLaunchable, isRebootRequired, displayName)
}
Add-Check msvc {
    @($vsRoots | ForEach-Object {
        $toolsetRoot = Join-Path $_ 'VC/Tools/MSVC'
        if (Test-Path -LiteralPath $toolsetRoot) {
            Get-ChildItem -LiteralPath $toolsetRoot -Directory | ForEach-Object {
                $compiler = Join-Path $_.FullName 'bin/Hostx64/x64/cl.exe'
                [ordered]@{ toolset = $_.Name; compiler = $compiler; note = 'Compiler banner/usage probe only; compilation is verified separately. A usage exit is expected without source input.'; version = if (Test-Path -LiteralPath $compiler) { Invoke-Version $compiler '' } else { $null } }
            }
        }
    })
} 'WARNING'
Add-Check windowsSdk {
    $include = Join-Path ${env:ProgramFiles(x86)} 'Windows Kits/10/Include'
    @(Get-ChildItem -LiteralPath $include -Directory | Select-Object Name, FullName)
}

$toolArguments = [ordered]@{ git = '--version'; python = '--version'; cmake = '--version'; ninja = '--version'; dotnet = '--version' }
foreach ($tool in $toolArguments.Keys) {
    $toolName = $tool
    Add-Check "tool_$toolName" {
        $command = Get-Command $toolName -ErrorAction SilentlyContinue
        if (-not $command) { throw "$toolName is not on PATH." }
        Invoke-Version $command.Source $toolArguments[$toolName]
    }
}
foreach ($root in $vsRoots) {
    foreach ($bundled in @(
        @{ name = 'cmake'; relative = 'Common7/IDE/CommonExtensions/Microsoft/CMake/CMake/bin/cmake.exe'; args = '--version' },
        @{ name = 'ninja'; relative = 'Common7/IDE/CommonExtensions/Microsoft/CMake/Ninja/ninja.exe'; args = '--version' }
    )) {
        $bundledPath = Join-Path $root $bundled.relative
        Add-Check "vs_$($bundled.name)" {
            if (-not (Test-Path -LiteralPath $bundledPath)) { throw "Bundled executable not found: $bundledPath" }
            Invoke-Version $bundledPath $bundled.args
        }
    }
}
Add-Check nvidia {
    $command = Get-Command nvidia-smi -ErrorAction Stop
    Invoke-Version $command.Source '--query-gpu=name,driver_version,memory.total --format=csv,noheader'
}
Add-Check unreal {
    # Search standard install roots only; do not recursively scan personal folders.
    $roots = @((Join-Path $env:ProgramFiles 'Epic Games'), 'D:/Epic Games', 'D:/UnrealEngine')
    $candidates = @($roots | Where-Object { Test-Path -LiteralPath $_ } | ForEach-Object { Get-ChildItem -LiteralPath $_ -Directory | ForEach-Object FullName })
    $manifest = Join-Path $env:ProgramData 'Epic/UnrealEngineLauncher/LauncherInstalled.dat'
    $manifestReadError = $null
    if (Test-Path -LiteralPath $manifest) {
        try {
            $installed = Get-Content -LiteralPath $manifest -Raw | ConvertFrom-Json
            $candidates += @($installed.InstallationList | Where-Object AppName -like 'UE_*' | ForEach-Object InstallLocation)
        } catch { $manifestReadError = $_.Exception.Message }
    }
    $engines = @($candidates | Select-Object -Unique | ForEach-Object {
            $engineRoot = $_
            $buildFile = Join-Path $engineRoot 'Engine/Build/Build.version'
            if (Test-Path -LiteralPath $buildFile) {
                [ordered]@{ root = $engineRoot; build = Get-Content -LiteralPath $buildFile -Raw | ConvertFrom-Json; editorPresent = Test-Path -LiteralPath (Join-Path $engineRoot 'Engine/Binaries/Win64/UnrealEditor.exe') }
            }
    })
    [ordered]@{ engines = $engines; version57Present = @($engines | Where-Object { $_.build.MajorVersion -eq 5 -and $_.build.MinorVersion -eq 7 }).Count -gt 0; manifestReadError = $manifestReadError; scope = 'Standard roots plus Launcher registered UE engines; presence/version only, no editor launch. User selected installed 5.8.3 for acceptance.' }
}
$wslPath = Join-Path $env:ProgramFiles 'WSL/wsl.exe'
if (-not (Test-Path -LiteralPath $wslPath)) { $wslPath = Join-Path $env:SystemRoot 'System32/wsl.exe' }
foreach ($argument in @('--version', '--status', '--list --verbose')) {
    Add-Check "wsl_$($argument.Replace('--', '').Replace(' ', '_'))" {
        Invoke-Version $wslPath $argument -Unicode
    }
}
Add-Check configPresence {
    [ordered]@{ wslConfigPresent = Test-Path -LiteralPath (Join-Path $env:USERPROFILE '.wslconfig'); note = 'Presence only. Configuration content was not read or changed.' }
}

$report = [ordered]@{ schemaVersion = 2; generatedAtUtc = [DateTime]::UtcNow.ToString('o'); scope = 'Read-only Windows host inventory. No installation, elevation, reboot, distribution launch or settings changes.'; checks = $checks }
New-Item -ItemType Directory -Path $ReportDirectory -Force | Out-Null
$jsonPath = Join-Path $ReportDirectory 'windows-audit.json'
$markdownPath = Join-Path $ReportDirectory 'windows-audit.md'
$report | ConvertTo-Json -Depth 16 | Set-Content -LiteralPath $jsonPath -Encoding utf8
$lines = @('# Windows environment audit', '', "Generated (UTC): $($report.generatedAtUtc)", '', $report.scope, '', 'PASS describes the specific inventory probe, not full integration acceptance. WARNING records limited evidence or an expected usage/pending result. Nonzero exits and missing probes are never PASS.', '')
foreach ($entry in $checks.GetEnumerator()) {
    $lines += "## $($entry.Key): $($entry.Value.status)"
    $lines += ''
    $lines += '```json'
    $lines += ($entry.Value | ConvertTo-Json -Depth 14)
    $lines += '```'
    $lines += ''
}
$lines | Set-Content -LiteralPath $markdownPath -Encoding utf8
Write-Output "Reports: $jsonPath ; $markdownPath"

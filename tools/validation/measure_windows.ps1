[CmdletBinding()]
param([int]$TimeoutSeconds = 180, [int]$SampleMilliseconds = 100)
$ErrorActionPreference = 'Stop'
if (-not $IsWindows) { throw 'Windows is required.' }
if ($TimeoutSeconds -lt 1 -or $SampleMilliseconds -lt 20) { throw 'Invalid measurement interval or timeout.' }
$root = Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
$reportDirectory = Join-Path $root 'reports/performance'
New-Item -ItemType Directory -Path $reportDirectory -Force | Out-Null

# Toolhelp snapshots expose parent IDs without the CIM permissions required by WMI.
if (-not ('UavMeasure.Native' -as [type])) {
    Add-Type -TypeDefinition @'
using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
namespace UavMeasure {
 [StructLayout(LayoutKind.Sequential, CharSet=CharSet.Unicode)]
 public struct Entry {
  public uint Size, Usage, Id; public UIntPtr Heap;
  public uint Module, Threads, Parent; public int Priority; public uint Flags;
  [MarshalAs(UnmanagedType.ByValTStr, SizeConst=260)] public string Name;
 }
 [StructLayout(LayoutKind.Sequential)]
 public struct Memory {
  public uint Length, Load;
  public ulong Total, Available, TotalPage, AvailablePage, TotalVirtual, AvailableVirtual, Extended;
 }
 public static class Native {
  [DllImport("kernel32.dll", SetLastError=true)] static extern IntPtr CreateToolhelp32Snapshot(uint flags, uint pid);
  [DllImport("kernel32.dll", CharSet=CharSet.Unicode, SetLastError=true)] static extern bool Process32FirstW(IntPtr snapshot, ref Entry entry);
  [DllImport("kernel32.dll", CharSet=CharSet.Unicode, SetLastError=true)] static extern bool Process32NextW(IntPtr snapshot, ref Entry entry);
  [DllImport("kernel32.dll")] static extern bool CloseHandle(IntPtr handle);
  [DllImport("kernel32.dll", SetLastError=true)] public static extern bool GlobalMemoryStatusEx(ref Memory memory);
  public static Entry[] Processes() {
   IntPtr snapshot=CreateToolhelp32Snapshot(2,0);
   if(snapshot==new IntPtr(-1)) throw new System.ComponentModel.Win32Exception();
   try {
    Entry entry=new Entry(); entry.Size=(uint)Marshal.SizeOf(entry);
    if(!Process32FirstW(snapshot,ref entry)) throw new System.ComponentModel.Win32Exception();
    var entries=new List<Entry>();
    do { entries.Add(entry); } while(Process32NextW(snapshot,ref entry));
    return entries.ToArray();
   } finally { CloseHandle(snapshot); }
  }
 }
}
'@
}
$memory = [UavMeasure.Memory]::new()
$memory.Length = [Runtime.InteropServices.Marshal]::SizeOf($memory)
$memoryBaseline = if ([UavMeasure.Native]::GlobalMemoryStatusEx([ref]$memory)) {
    @{ status = 'PASS'; totalBytes = $memory.Total; availableBytes = $memory.Available; usedBytes = $memory.Total - $memory.Available; context = 'Active user session at measurement start; not an idle baseline.' }
} else { @{ status = 'NOT_TESTED'; error = 'GlobalMemoryStatusEx failed.' } }
$drives = @([IO.DriveInfo]::GetDrives() | Where-Object { $_.IsReady -and $_.DriveType -eq 'Fixed' } | Select-Object Name, TotalSize, AvailableFreeSpace)
$gpu = @{ status = 'NOT_TESTED' }
try {
    $gpuOutput = & nvidia-smi --query-gpu=name,driver_version,memory.used,memory.total,utilization.gpu --format=csv,noheader 2>&1
    $gpu = @{ status = if ($LASTEXITCODE -eq 0) { 'PASS' } else { 'FAIL' }; exitCode = $LASTEXITCODE; output = ($gpuOutput -join "`n"); context = 'Active user session before build; not an Unreal workload.' }
} catch { $gpu.error = $_.Exception.Message }

$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio/Installer/vswhere.exe'
$vs = & $vswhere -latest -products '*' -version '[17.0,18.0)' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if (-not $vs) { throw 'VS2022 C++ tools are unavailable.' }
$cmake = Join-Path $vs 'Common7/IDE/CommonExtensions/Microsoft/CMake/CMake/bin/cmake.exe'
& $cmake --build (Join-Path $root 'build/windows-x64-debug') --target clean *> (Join-Path $reportDirectory 'clean.log')
if ($LASTEXITCODE -ne 0) { throw 'Clean target failed; see reports/performance/clean.log.' }

$stdout = Join-Path $reportDirectory 'build.stdout.log'
$stderr = Join-Path $reportDirectory 'build.stderr.log'
$pwsh = (Get-Process -Id $PID).Path
$script = Join-Path $root 'tools/bootstrap/build_windows.ps1'
$watch = [Diagnostics.Stopwatch]::StartNew()
$process = Start-Process -FilePath $pwsh -ArgumentList @('-NoProfile', '-File', "`"$script`"") -WorkingDirectory $root -WindowStyle Hidden -RedirectStandardOutput $stdout -RedirectStandardError $stderr -PassThru
$owned = [Collections.Generic.HashSet[int]]::new()
[void]$owned.Add($process.Id)
$cpuByPid = @{}
$maxWorkingSet = 0L
$samples = 0
$trackingError = $null
$timedOut = $false
while (-not $process.HasExited) {
    if ($watch.Elapsed.TotalSeconds -gt $TimeoutSeconds) {
        $timedOut = $true
        # Terminate only this measurement's process tree, never another build.
        & taskkill.exe /PID $process.Id /T /F *> (Join-Path $reportDirectory 'timeout.log')
        break
    }
    if (-not $trackingError) {
        try {
            $entries = [UavMeasure.Native]::Processes()
            do {
                $added = $false
                foreach ($entry in $entries) {
                    if ($owned.Contains([int]$entry.Parent) -and $owned.Add([int]$entry.Id)) { $added = $true }
                }
            } while ($added)
            $sum = 0L
            foreach ($ownedId in $owned) {
                try {
                    $child = [Diagnostics.Process]::GetProcessById($ownedId)
                    $sum += $child.WorkingSet64
                    $cpuByPid[$ownedId] = $child.TotalProcessorTime.TotalSeconds
                    $child.Dispose()
                } catch [ArgumentException] { } # A child can exit between snapshot and sample.
            }
            $maxWorkingSet = [Math]::Max($maxWorkingSet, $sum)
            $samples++
        } catch { $trackingError = $_.Exception.Message }
    }
    Start-Sleep -Milliseconds $SampleMilliseconds
    $process.Refresh()
}
$watch.Stop()
$process.WaitForExit()
$exitCode = $process.ExitCode
$process.Dispose()
$cpuSeconds = ($cpuByPid.Values | Measure-Object -Sum).Sum
$report = [ordered]@{
    schemaVersion = 1; generatedAtUtc = [DateTime]::UtcNow.ToString('o')
    build = @{ status = if ($timedOut) { 'FAIL' } elseif ($exitCode -eq 0) { 'PASS' } else { 'FAIL' }; exitCode = $exitCode; timedOut = $timedOut; wallSeconds = $watch.Elapsed.TotalSeconds; command = 'tools/bootstrap/build_windows.ps1'; scope = 'Clean rebuild: developer environment setup, configure, compile/link and CTest.' }
    processSampling = @{ status = if ($trackingError -or $samples -eq 0) { 'NOT_TESTED' } else { 'PASS' }; error = $trackingError; samples = $samples; requestedIntervalMilliseconds = $SampleMilliseconds; sampledMaxAggregateWorkingSetBytes = if ($trackingError) { $null } else { $maxWorkingSet }; sampledCumulativeCpuSeconds = if ($trackingError) { $null } else { $cpuSeconds }; observedProcessCount = $owned.Count; limitation = 'Sampled owned process tree only. Short-lived children and between-sample peaks can be missed. CPU sums last observed CPU time per process; not exact total CPU or exact peak RAM. Shared pages may be counted per process.' }
    activeSessionMemory = $memoryBaseline; diskBeforeBuild = $drives; gpuBeforeBuild = $gpu
    unrealScene = @{ status = 'NOT_TESTED'; reason = 'This measurement only runs the headless core build.' }
}
$report | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath (Join-Path $reportDirectory 'windows-baseline.json') -Encoding utf8
@('# Windows local performance observations', '', "UTC: $($report.generatedAtUtc)", '', 'This is an active-session headless build measurement, not an idle-system or Unreal scene baseline.', '', '```json', ($report | ConvertTo-Json -Depth 8), '```') | Set-Content -LiteralPath (Join-Path $reportDirectory 'windows-baseline.md') -Encoding utf8
Write-Output "Build $($report.build.status): $($watch.Elapsed.TotalSeconds) seconds; sampled RAM $maxWorkingSet bytes; $samples samples. Reports: $reportDirectory"
if ($report.build.status -ne 'PASS') { throw 'Measured build failed; inspect ignored performance logs.' }

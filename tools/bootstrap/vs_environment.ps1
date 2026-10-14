function Get-VsEnvironmentLines([string]$DeveloperScript) {
    if (-not (Test-Path -LiteralPath $DeveloperScript -PathType Leaf)) { throw 'Visual Studio developer script is missing.' }
    $start = [Diagnostics.ProcessStartInfo]::new()
    $start.FileName = $env:ComSpec
    # Raw cmd syntax avoids PowerShell requoting paths under Program Files.
    $start.Arguments = '/d /s /u /c "call "' + $DeveloperScript + '" -arch=x64 -host_arch=x64 >nul && set"'
    $start.UseShellExecute = $false
    $start.CreateNoWindow = $true
    $start.RedirectStandardOutput = $true
    $start.RedirectStandardError = $true
    $start.StandardOutputEncoding = [Text.Encoding]::Unicode
    $start.StandardErrorEncoding = [Text.Encoding]::Unicode
    $process = [Diagnostics.Process]::new()
    $process.StartInfo = $start
    try {
        [void]$process.Start()
        $stdout = $process.StandardOutput.ReadToEndAsync()
        $stderr = $process.StandardError.ReadToEndAsync()
        if (-not $process.WaitForExit(60000)) {
            $process.Kill($true)
            throw 'Visual Studio developer environment setup timed out.'
        }
        $output = $stdout.GetAwaiter().GetResult()
        [void]$stderr.GetAwaiter().GetResult()
        if ($process.ExitCode -ne 0) { throw "Visual Studio developer environment setup failed (exit $($process.ExitCode))." }
        # Environment output can contain credentials; return it internally, never log it.
        return $output -split '\r?\n'
    } finally { $process.Dispose() }
}

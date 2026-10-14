$ErrorActionPreference = 'Stop'
. "$PSScriptRoot/../bootstrap/vs_environment.ps1"
$root = Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
$folder = Join-Path $root ('build/VS environment (regression) ' + [guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $folder | Out-Null
$pass = Join-Path $folder 'developer setup.cmd'
$fail = Join-Path $folder 'failed setup.cmd'
"@echo off`r`nset UAVSIM_ENV_PROBE=spaces=and parentheses work`r`nexit /b 0" | Set-Content -LiteralPath $pass -Encoding ascii
"@echo off`r`nset UAVSIM_ENV_PROBE=must not escape`r`nexit /b 7" | Set-Content -LiteralPath $fail -Encoding ascii
$before = [Environment]::GetEnvironmentVariable('UAVSIM_ENV_PROBE', 'Process')
$lines = Get-VsEnvironmentLines $pass
if ($lines -notcontains 'UAVSIM_ENV_PROBE=spaces=and parentheses work') { throw 'Space/parenthesis path or environment value parsing failed.' }
if ([Environment]::GetEnvironmentVariable('UAVSIM_ENV_PROBE', 'Process') -ne $before) { throw 'Child environment escaped into parent.' }
$rejected = $false
try { [void](Get-VsEnvironmentLines $fail) } catch { $rejected = $_.Exception.Message -match 'exit 7' }
if (-not $rejected) { throw 'Developer setup failure was accepted.' }
Write-Output 'PASS Visual Studio environment paths, child isolation and nonzero exit handling.'

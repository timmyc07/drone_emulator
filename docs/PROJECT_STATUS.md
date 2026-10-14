# Project Status

**Session date:** 2026-10-14 (Asia/Taipei)
**Current host:** Machine B, Windows 11 Pro 24H2 x64, build 26100.9457.
**Current phase:** P0. Windows local mandatory gates PASS; complete dual-platform P0 remains incomplete because Machine A still needs Python 3.12 and merged-source verification.

## Current Windows results

| Stage/gate | Actual result |
| --- | --- |
| P0.0 inventory | PASS: Ryzen 7 7700, 8 cores/16 logical CPUs; two 16 GiB RAM modules, 32 GiB installed, 30.88 GiB OS usable; RTX 5070, driver 617.14, 12,227 MiB VRAM |
| P0.2 / P0-B | PASS: existing VS Build Tools 17.14.37710.0, MSVC 19.44.35229, SDK 26100; C++20 compile/link, 3/3 CTests, Python 3.12.10 local venv, pip 25.0.1 |
| P0.3 / P0-C | PASS: user-selected existing UE 5.8.3, real C++ Editor target build, headless level/actor automation, graphical Editor startup/test, and saved DX12 scene with 180 game-world actor ticks; manual visual inspection NOT_TESTED |
| P0.4 | WSL 3.0.3 installed; VirtualMachinePlatform REBOOT_PENDING (DISM 3010); Ubuntu 22.04 DEFERRED until separately approved reboot |
| P0.6 local | PASS: original uavsim_smoke plus cpp20_smoke and invalid-argument test; information executable reports version 0.1.0; Python venv/import/filesystem smoke and pip checks PASS |
| P0.7 local | PASS for actual sampled Windows build/Editor/scene observations; true idle baseline and exact process-tree peaks NOT_TESTED |
| P0-A Mac | Prior C++/CTest evidence preserved below; Python 3.12 missing and merged source NOT_TESTED on Machine A |
| Hosted CI | NOT_TESTED: push completed, but no hosted success was confirmed; connected commit status query returned no statuses and the workflow wrapper only lists PR-triggered runs |
| Remote synchronization | PASS: implementation commit `436d1c7` pushed normally; `git ls-remote` matched local SHA `436d1c76b81dddde5710b11718cb6870917cf744`; remote history preserved |

## Changes and commands executed

- Initial local directory was empty. After the remote appeared during the session, fetched `origin/main` at `a7b3016` and completed three additional integration reviews. Preserved original core sources, version header, smoke test, master plan and Mac evidence; backed up overlapping local scaffold files before integration.
- Reused existing tools. No Unreal installation, compiler replacement, GPU-driver update, global PATH/Git identity change, `.wslconfig` change, or restart was performed. Repository-local Git identity uses the verified GitHub account's noreply address.
- Approved system changes: Microsoft-signed WSL 3.0.3 MSI (exit 0), VirtualMachinePlatform enable with `/norestart` (3010), and component-only VS `.NET Framework 4.8 SDK` repair (exit 0, required header verified).
- Executed `tools/environment/audit_windows.ps1`, `tools/validation/validate_windows.ps1`, `tools/validation/measure_windows.ps1`, and `tools/validation/validate_unreal.ps1` on this host.
- Core script executed `cmake --preset windows-x64-debug`, `cmake --build --preset windows-x64-debug`, and `ctest --preset windows-x64-debug` after importing only process-local VS developer settings. Corrected the prior Ninja single-config executable path.
- Unreal used `Build.bat P0ProbeEditor Win64 Development -Project=... -MaxParallelActions=2`, then Editor automation with JSON report inspection and separate DX12 game/editor runs. Lightweight generated cube level, 1920x1080 development resolution, Medium scalability groups, disabled Lumen/ray tracing.
- Checked all PowerShell syntax, preset/project JSON, original-source preservation, ignore rules and Git whitespace. Mac shell scripts retain their executable Git modes; their updated execution remains unverified on Machine A.
- Disabled the unused AndroidFileServer plugin in the isolated fixture, excluded Editor-generated input defaults, and reran the focused Editor level/actor automation test against the final configuration: PASS. Only intentional project settings were committed.

## Recovery evidence

The restricted sandbox initially caused MSVC C1902 during the compiler probe; the same installed compiler succeeded outside it. The first Unreal build failed because SwarmInterface could not locate NetFxSDK. After the approved component repair, actual Unreal compilation and tests succeeded. Both failures and successful retries are retained locally. Early 2-test scaffold results are historical; final integrated core acceptance is 3/3.

## Windows resource baseline

The integrated clean core workflow took **5.5058 seconds**, with 38 requested-100ms samples across 19 observed owned processes. Sampled maximum aggregate working set was **285,560,832 bytes (272.33 MiB)**; sampled cumulative CPU was **3.28125 seconds**. Active-session RAM used **16,481,378,304 / 33,157,967,872 bytes**; this was not an idle measurement. Prebuild GPU use was **1,559 / 12,227 MiB**, 6% utilization. C: free 450,232,631,296 bytes; D: free 861,367,222,272 bytes at measurement start.

Unreal measurements and limitations are in the final resource summary below. First successful cold project build took 61.47 seconds. GPU samples are host-wide and can include other applications and shader workers; process working-set samples can miss peaks and do not include all child processes. No FPS or flight-simulation performance is claimed.

## Evidence locations and next actions

Private detailed reports are ignored under `reports/environment`, `reports/validation`, `reports/performance`, `reports/unreal` and `reports/install`. Initial audits, failed attempts, bootstrap-only baselines and successful reports are preserved. The previous tracked Windows placeholder baseline is replaced by this sanitized summary; historical Mac reports remain unchanged.

Next: request separate reboot approval after current work is saved; after reboot inspect WSL and complete the already-approved Ubuntu 22.04 installation and initialization. Resource limits (8 GB RAM / 4 GB swap) are proposals, not applied settings. Verify Mac Python 3.12 and the merged source independently. Do not begin flight dynamics or SITL installation in this task.

## Governing plan

[CODEX MASTER PROMPT V2.0](MASTER_PLAN.md) remains unchanged. The user's explicit UE 5.8 selection supersedes the earlier 5.7 target for this environment verification.

## Prior Machine A session (historical, imported from a7b3016)

The following is the original Mac status record. Its Windows/CI NOT_TESTED statements described that earlier session; the Windows results above supersede them. These Mac results were not rerun from this host, and Python 3.9.6 does not satisfy the target 3.12 requirement.

# Project Status

**Session date:** 2026-10-14 (Asia/Taipei)
**Current host:** Apple Silicon MacBook Pro, macOS 26.6.2 arm64

## Verified

- Native Apple M5 Pro, 15 logical CPUs, 24 GiB-class memory, 621 GiB available disk.
- Apple Command Line Tools and Apple Clang 21.0.0 detected.
- Git 2.50.1 detected.
- Project-local CMake 4.4.4 and Ninja 1.13.2 installed in `.venv`.
- C++20 configure/build completed with Apple Clang.
- CTest: 1/1 test passed.
- Python venv creation and smoke test passed under Python 3.9.6.
- Audit JSON, CMake presets, platform scripts, CI workflow, and documentation created.

## Missing or deferred

- Python 3.12: NOT_INSTALLED/NOT_TESTED.
- Homebrew, full Xcode, VS Code, GitHub CLI: NOT_INSTALLED or unavailable on PATH.
- Windows MSVC, RTX 5070, Unreal Engine, WSL2: NOT_TESTED.
- GitHub Actions: NOT_TESTED until workflow is triggered.

## Next executable task

Run `docs/WINDOWS_CONTINUATION.md` on Machine B, then trigger CI and attach its actual logs.
## Primary plan

The governing product plan is [CODEX MASTER PROMPT V2.0](MASTER_PLAN.md).

## Final Unreal resource observations

Measured after applying all Medium scalability groups. Durations include process startup, testing and exit; RAM is sampled single-process working set, not exact peak or full process-tree memory. GPU readings are host-wide.

| Phase | Duration (s) | Sampled RAM (MiB) | Last sampled CPU (s) | Maximum sampled host VRAM (MiB) |
| --- | ---: | ---: | ---: | ---: |
| Editor automation | 50.971 | 2594.73 | 20.500 | 11390 / 12227 |
| DX12 simple scene | 21.507 | 2019.23 | 17.531 | 11515 / 12227 |
| Graphical Editor startup and level | 39.457 | 3395.12 | 32.703 | 11437 / 12227 |

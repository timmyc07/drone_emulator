<!-- Historical checklist prepared on Mac and retained for provenance; current continuation below supersedes completed or changed steps. -->
# Windows Continuation Checklist

This checklist is prepared on macOS and has **not** been executed on Windows.

1. Open PowerShell in the repository and run `tools/environment/audit_windows.ps1`; save output as `reports/environment/windows-audit.json`.
2. Confirm Windows 11 x64, 32 GB RAM target, RTX 5070 model/VRAM, NVIDIA driver, Visual Studio 2022, MSVC, Windows SDK, CMake, Ninja, Git, Python 3.12, and WSL2 state.
3. Do not install or modify Visual Studio, WSL, Unreal Engine, drivers, optional Windows features, or machine-wide environment variables without explicit approval.
4. Use `tools/bootstrap/build_windows.ps1`, then `ctest --preset windows-x64-debug --output-on-failure`.
5. Run `python tools/validation/python_smoke.py` in the approved project venv.
6. If Unreal Engine 5.7 is already present, record its exact version and perform the minimal editor/C++ project test. If absent, mark P0-C BLOCKED and request approval with the component checklist.
7. Inspect `.wslconfig` without overwriting it; back up and propose changes only with approval.
8. Record actual results in `docs/PROJECT_STATUS.md` and `reports/performance/windows-baseline.md`.

## Current Windows continuation — 2026-10-14

The preceding checklist is historical, imported from Machine A commit `a7b3016`; its NOT_TESTED statements describe that earlier Mac session. This Windows session verified the existing MSVC/Python tools and selected UE 5.8.3. The .NET Framework 4.8 SDK repair succeeded, followed by Unreal C++ build, headless automation, graphical Editor automation, and a DX12 game-world test with 180 ticks. Manual visual inspection remains NOT_TESTED.

Use PowerShell 7 from the repository root. Run `tools/environment/audit_windows.ps1` and `tools/validation/validate_windows.ps1`; the latter uses the maintained build script and platform preset, then the local Python 3.12 environment. Use `tools/validation/validate_unreal.ps1` for the isolated Unreal smoke project, following its current parameters. Capture actual exit codes, logs and test counts; final integration results are in PROJECT_STATUS.

WSL 3.0.3 MSI installation completed with exit 0. Virtual Machine Platform enable returned 3010 (REBOOT_PENDING). Request separate user approval before rebooting; no reboot has been performed in this session. After approved restart:

1. Verify `wsl --version`, `wsl --status`, and `wsl --list --verbose`.
2. Continue the already-approved Ubuntu 22.04 installation if absent; verify it runs as WSL2. Complete user account initialization interactively without logging credentials.
3. Inspect existing `.wslconfig`; back up and obtain approval for any concrete change. Suggested 8 GB memory / 4 GB swap limits remain proposals until applied and verified.
4. Verify actual distribution/memory state. Do not install PX4/ArduPilot during P0.
5. Update status, dependencies, decisions, and necessary test evidence; commit and push without overwriting remote work.

Separately, Machine A needs Python 3.12 and a fresh merged-source build/test. Preserve previous Mac CTest 1/1 and Python 3.9.6 evidence as historical results. CI configuration requires an actual successful run to claim PASS.

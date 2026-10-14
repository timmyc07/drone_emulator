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

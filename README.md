# drone_emulator

Universal UAV Simulator (UAVSim) is a cross-platform UAV simulation product. P0 validates a C++20 headless core and a small Unreal fixture; it contains no flight dynamics. The governing product plan is [CODEX MASTER PROMPT V2.0](docs/MASTER_PLAN.md).

See [current results](docs/PROJECT_STATUS.md), [P0 plan](docs/P0_PLAN.md), [dependencies](docs/DEPENDENCIES.md), and [engineering decisions](docs/DECISIONS.md).

## Windows

Use PowerShell 7 with existing VS 2022 C++ Build Tools and Python 3.12:

```powershell
./tools/environment/audit_windows.ps1
./tools/validation/validate_windows.ps1
./tools/validation/measure_windows.ps1
```

The build script discovers VS and temporarily imports its developer environment into the process. It does not change global PATH. Build outputs use `build/windows-x64-debug`; Python uses `.venv`. Detailed machine reports and generated Unreal assets are ignored by Git. Run MSVC outside a restrictive sandbox if its debug compiler probe fails with C1902; preserve the failure and retry result.

## macOS ARM64

With native Apple Clang, CMake >=3.20, Ninja and Python 3.12 already installed:

```sh
sh tools/bootstrap/build_macos.sh
sh tools/validation/validate_macos.sh
```

The prior Machine A session's Apple Clang/CTest evidence is preserved; Python 3.12 and these updated scripts require a new Mac verification. GitHub Actions targets Windows x64 and macOS ARM64 using supported `windows-2022` and `macos-14` labels; workflow configuration alone is not CI success.

## Unreal and WSL

The current user-selected Unreal target is the existing **5.8.3**, superseding the Master Prompt's 5.7 baseline. `unreal/P0Probe` is a disposable validation fixture using the installed engine's blank-project conventions. Generated `Content/P0Smoke.umap` is created by the editor automation test and is ignored. Existing Git LFS asset rules are preserved; verify Git LFS before adding future product binaries.

```powershell
./tools/validation/validate_unreal.ps1
```

The fixture builds, tests and saves a small editor level, runs it through DX12 for 180 actor ticks, and checks graphical Editor startup. Each process exits automatically; logs and sampled resources are saved under `reports/unreal`. It is unrelated to the future P2 flight implementation.

`tools/bootstrap/prepare_wsl.ps1` downloads a signed official stable WSL MSI into ignored reports. Its `-ElevatedStage` requires prior installation approval and a normal administrator process. It never restarts Windows or changes `.wslconfig`; Ubuntu setup is deferred until prerequisites and reboot are resolved. Do not install PX4 or ArduPilot in P0.

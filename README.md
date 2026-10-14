# drone_emulator

Universal UAV Simulator (UAVSim) is moving to a browser drone game at the user's request. It reuses the independent C++20 quadrotor core through WebAssembly and Three.js rendering, with the user's supplied drone model as the default aircraft. The existing core provides RK4 rigid-body dynamics, four motor models, flight controls, strict vehicle JSON, telemetry and replay. The browser's current verification state is recorded in [PROJECT_STATUS.md](docs/PROJECT_STATUS.md); earlier native/Unreal tests do not establish browser acceptance.

Follow [the browser build and launch guide](docs/WEB_GAME.md). Current delivery uses a local server at `http://127.0.0.1:4173/`. The forest is independently created; Electric Dreams is UE-only and is excluded. The user's drone assets stay local because redistribution permission is unverified.

On Windows, the current WebAssembly build and runtime checks passed, including 5,000 steps compared with native physics. Chrome and the in-app browser rendered the supplied drone; basic flight, narrow layout, JSON/CSV downloads and recorded-state playback passed. Remaining browser acceptance and platform limits are listed in [PROJECT_STATUS.md](docs/PROJECT_STATUS.md).

The latest target override in [MASTER_PLAN.md](docs/MASTER_PLAN.md) supersedes the original Unreal desktop requirements. Unreal source and the desktop commands below remain historical, with further Unreal builds, packaging and migration suspended.

See [current results](docs/PROJECT_STATUS.md), [P0 plan](docs/P0_PLAN.md), [dependencies](docs/DEPENDENCIES.md), and [engineering decisions](docs/DECISIONS.md).

## Windows

Use PowerShell 7 with existing VS 2022 C++ Build Tools and Python 3.12:

```powershell
./tools/validation/validate_windows.ps1
./tools/validation/validate_product.ps1
```

The build script discovers VS and temporarily imports its developer environment into the process. It does not change global PATH. Build outputs use `build/windows-x64-debug`; Python uses `.venv`. Detailed machine reports and generated Unreal assets are ignored by Git. Run MSVC outside a restrictive sandbox if its debug compiler probe fails with C1902; preserve the failure and retry result.

The product validator builds and runs the CTests, a 30-second hover, takeoff/landing, recorded-input replay, telemetry checks and invalid-input cases. Each run creates a new ignored `reports/product` directory. See [P1 acceptance](docs/P1_ACCEPTANCE.md) for commands and thresholds. `uavsim_headless.exe --help` lists individual commands; `--output` requires a new session directory to avoid overwriting recordings.

The fictional reference aircraft uses documented engineering estimates, not validated real-aircraft data. All dynamics use SI units, ENU world and FLU body axes. Gravity, calm/configurable wind, quadratic drag and simplified flat ground contact are modeled; terrain/obstacle collisions, batteries and sensors are not.

## macOS ARM64

With native Apple Clang, CMake >=3.20, Ninja and Python 3.12 already installed:

```sh
bash tools/validation/validate_macos.sh
```

Hosted Windows and macOS ARM64 CI passed product commit `7f21881c1b8adbe252f74793250bbb262b1df49b` in [run 38542931069](https://github.com/timmyc07/drone_emulator/actions/runs/38542931069): Windows 6/6 CTests, macOS 5/5, and Python 3.12.10 smoke checks on both. The extra Windows test verifies developer-environment path handling, including the repaired earlier CI failure. These jobs validate the portable core; the user's Machine A still needs its own Python 3.12 setup and current-source validation.

## Historical desktop simulator — suspended

To run the verified local prototype without Editor, follow [Windows package instructions](docs/WINDOWS_PACKAGE.md). The package remains local because asset redistribution terms are unverified.

Open `unreal/UAVSim/UAVSim.uproject` with the selected Unreal Engine **5.8.3**. The product plugin compiles the same portable core, disables Chaos on the vehicle, advances at a fixed 0.004-second step and interpolates rendering. The responsive menu exposes aircraft/environment selection, graphics/resolution, display units, sensitivity, key bindings and flight controls. Default keys: W/S pitch, A/D roll, Q/E yaw, Space/Ctrl climb, F arm, M mode, P pause, R reset and Esc menu. Actual gamepad hardware remains unverified.

```powershell
./tools/validation/validate_product_unreal.ps1
```

Pass `-EngineRoot` if the selected engine is installed elsewhere. `-SkipBuild` validates an already built product without claiming a new build. `-PackagedExecutable` accepts an existing absolute `.exe` path and additionally tests that standalone application. The validator checks exact Unreal automation results plus DX12/model/flight/session markers; its timestamped reports retain failures and keep manual visual inspection separate.

The user-provided FBX and six textures were imported locally. Historical lightweight map reproduction uses `tools/unreal/create_product_level.py`; asset setup uses `tools/unreal/setup_product_assets.py` and `tools/unreal/import_drone.py`. These scripts run in Unreal's Python environment. Electric Dreams was acquired locally; integration is suspended under the browser target. See [asset status and licenses](docs/ASSETS.md); raw third-party assets remain excluded from GitHub.

The desktop saves CSV and recorded-state playback files under its writable `Saved/Sessions` directory. UI playback displays recorded states; headless replay re-executes recorded inputs and checks the trajectory. Neither establishes real-aircraft fidelity. A packaged distribution must pass its own standalone acceptance before being described as released.

## Infrastructure fixture and WSL

`unreal/P0Probe` remains an isolated infrastructure fixture. Its generated `Content/P0Smoke.umap` is ignored, and its actor exits after 180 ticks; it is not the product front end.

```powershell
./tools/validation/validate_unreal.ps1
```

The fixture builds, tests and saves a small editor level, runs it through DX12 for 180 actor ticks, and checks graphical Editor startup. Logs and sampled resources are saved under `reports/unreal`; product evidence uses `reports/product`.

`tools/bootstrap/prepare_wsl.ps1` downloads a signed official stable WSL MSI into ignored reports. Its `-ElevatedStage` requires prior installation approval and a normal administrator process. It never restarts Windows or changes `.wslconfig`; Ubuntu setup is deferred until prerequisites and reboot are resolved. Do not install PX4 or ArduPilot in P0.

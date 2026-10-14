# Roadmap

## Active browser delivery — 2026-10-14

The user replaced the Unreal desktop target with a browser drone game using the same supplied ZIP drone. The previous desktop sequence below is suspended. The C++20 headless core remains the shared physics implementation; browser work does not require another Unreal build or Electric Dreams migration.

1. Compile the existing core to WebAssembly with project-local Emscripten and verify finite state, hover and input/control behavior.
2. Build and inspect the responsive Three.js game, independent forest, supplied default model and its materials; verify loading errors visibly block an invalid start.
3. Exercise flight controls, pause/reset, gameplay and responsive layout in an actual browser; keep hardware gamepad support unclaimed until tested.
4. Verify the local-only static server and prepared output boundary, preserve dependency notices and exclude unlicensed-for-redistribution user assets from GitHub.
5. Run native regression checks, record actual browser/build evidence, update source documentation and synchronize source changes. Public hosting and broad P4–P7 expansion are outside this browser delivery.

See [WEB_GAME.md](WEB_GAME.md) and [PROJECT_STATUS.md](PROJECT_STATUS.md). Browser results are NOT_TESTED until executed and captured.

## Historical desktop roadmap — suspended

Current focus: complete the usable Windows product acceptance after the verified P1 headless core. Unreal 5.8.3 is the selected engine. P2/P3 automated product evidence and the first standalone package/runtime have passed. Interactive repairs/regression, clean-machine release acceptance and the requested Electric Dreams environment still require completion. [PROJECT_STATUS.md](PROJECT_STATUS.md) records current commands/results; the full product is not complete.

Next executable gates:

1. Finish product Unreal build/automation/runtime after final repairs, then exercise flight, settings persistence, save/playback and exit interactively.
2. Rebuild and revalidate the standalone Windows package after the observed UI/lighting repairs. The first package already passed launch/runtime, JSON and notice checks; verify runtime prerequisites and recovery on a clean machine.
3. Complete Electric Dreams acquisition, map integration/cooking and actual resource/performance validation on the 32 GiB Windows host.
4. Keep hosted core CI passing as source changes continue. Product commit `7f21881c1b8adbe252f74793250bbb262b1df49b` already passed Windows 6/6 and macOS ARM64 5/5 CTests in [run 38542931069](https://github.com/timmyc07/drone_emulator/actions/runs/38542931069).
5. Keep actual gamepad verification, Machine A setup and WSL initialization explicit. Define and execute later-phase gates before implementing those phases.

## Product phases

The full governing requirements and phase gates remain in [MASTER_PLAN.md](MASTER_PLAN.md).

| Phase | Deliverable and acceptance |
| --- | --- |
| P0 Infrastructure | Repository, both platform instructions, environment evidence, builds/tests, CI configuration and recorded cross-platform results; honor strict host/Unreal gates in P0_PLAN |
| P1 Flight Dynamics | Headless math, 6-DOF, propulsion/environment, reference quadrotor/configuration, telemetry; required physics tests and hover pass |
| P2 Flyable Unreal Prototype | C++ bridge, visible quadrotor/simple level, input/camera/HUD, start/pause/reset; controls affect actual validated dynamics |
| P3 Usable Simulator | Menus, aircraft/environment selection, settings/gamepad, modes, recording/playback, Windows package; user can run without Editor |
| P4 Multiple Vehicle Classes | PLANNED. At least one tested configuration per promised additional family; validate backend selection, trim/equilibrium, units, control response, convergence and lifecycle; one authoritative backend per step |
| P5 Advanced Simulation | PLANNED. Sensors require known-input/noise/timing checks; weather/aerodynamics need documented limits; PX4 and ArduPilot each require an actual compatible SITL connection, time/coordinate checks, commands, telemetry and disconnect/failsafe tests |
| P6 Vehicle Library | PLANNED. Configurations load without core recompilation where supported; parameter-level sources/confidence/unknowns and asset licenses are recorded; registry/asset failure tests pass; no unsupported real-aircraft fidelity claims |
| P7 Product Release | PLANNED. Versioned installer/portable artifact runs on a clean target environment; prerequisites, third-party notices, writable sessions, missing-resource/crash recovery, checksums and upgrade instructions verified; full user workflow passes |

Do not advance past critical failed tests. Keep historical P0 validation separate from product implementation; later-phase infrastructure or placeholder interfaces do not complete their milestone. Optional deferrals must be explicit.

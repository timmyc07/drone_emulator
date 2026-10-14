# Roadmap

Current focus: complete the usable Windows product acceptance after the verified P1 headless core. Unreal 5.8.3 is the selected engine. P2/P3 automated product evidence exists, but interactive acceptance, a standalone package and the requested Electric Dreams environment still require completion. [PROJECT_STATUS.md](PROJECT_STATUS.md) records current commands/results; the full product is not complete.

Next executable gates:

1. Finish product Unreal build/automation/runtime after final repairs, then exercise flight, settings persistence, save/playback and exit interactively.
2. Cook/package Windows and run the packaged executable independently of Editor; confirm required JSON, licenses, assets and runtime dependencies are present.
3. Complete Electric Dreams acquisition, map integration/cooking and actual resource/performance validation on the 32 GiB Windows host.
4. Push reviewed source and confirm both current hosted CI jobs. Prior P0 macOS CI passed; Windows quoting was repaired locally and needs a hosted result.
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

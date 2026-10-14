# Roadmap

## Active browser delivery — 2026-10-14

The user replaced the Unreal desktop target with a browser drone game using the same supplied ZIP drone. The Unreal source, generated project and desktop tools have since been removed at the user's direction. The C++20 headless core remains the physics implementation; browser work does not require Unreal or Electric Dreams migration.

1. Compile the existing core to WebAssembly with project-local Emscripten and verify finite state, hover and input/control behavior.
2. Build and inspect the responsive Three.js game, independent forest, supplied default model and its materials; verify loading errors visibly block an invalid start.
3. Exercise flight controls, pause/reset, gameplay and responsive layout in an actual browser; keep hardware gamepad support unclaimed until tested.
4. Verify the local-only static server and prepared output boundary, preserve dependency notices and exclude unlicensed-for-redistribution user assets from GitHub.
5. Run native regression checks, record actual browser/build evidence, update source documentation and synchronize source changes. Public hosting and broad P4–P7 expansion are outside this browser delivery.

See [WEB_GAME.md](WEB_GAME.md) and [PROJECT_STATUS.md](PROJECT_STATUS.md) for executed browser results and remaining NOT_TESTED gates.

## Historical desktop roadmap — retired

This desktop roadmap is retained as historical context. The user retired the Unreal product and desktop-only tools on 2026-10-14; its source and generated project content have been removed. [PROJECT_STATUS.md](PROJECT_STATUS.md) preserves earlier execution evidence.

The desktop acceptance gates were retired with the application. Historical results and incomplete gates remain in [PROJECT_STATUS.md](PROJECT_STATUS.md); they are not current development tasks.

## Product phases

The original desktop phase table below and requirements in [MASTER_PLAN.md](MASTER_PLAN.md) are historical; browser acceptance gates are in [WEB_GAME.md](WEB_GAME.md).

| Phase | Deliverable and acceptance |
| --- | --- |
| P0 Infrastructure | Historical desktop/bootstrap requirements; active C++ core CI remains in `.github/workflows/core-ci.yml` |
| P1 Flight Dynamics | Headless math, 6-DOF, propulsion/environment, reference quadrotor/configuration, telemetry; required physics tests and hover pass |
| P2 Flyable Unreal Prototype | C++ bridge, visible quadrotor/simple level, input/camera/HUD, start/pause/reset; controls affect actual validated dynamics |
| P3 Usable Simulator | Menus, aircraft/environment selection, settings/gamepad, modes, recording/playback, Windows package; user can run without Editor |
| P4 Multiple Vehicle Classes | PLANNED. At least one tested configuration per promised additional family; validate backend selection, trim/equilibrium, units, control response, convergence and lifecycle; one authoritative backend per step |
| P5 Advanced Simulation | PLANNED. Sensors require known-input/noise/timing checks; weather/aerodynamics need documented limits; PX4 and ArduPilot each require an actual compatible SITL connection, time/coordinate checks, commands, telemetry and disconnect/failsafe tests |
| P6 Vehicle Library | PLANNED. Configurations load without core recompilation where supported; parameter-level sources/confidence/unknowns and asset licenses are recorded; registry/asset failure tests pass; no unsupported real-aircraft fidelity claims |
| P7 Product Release | PLANNED. Versioned installer/portable artifact runs on a clean target environment; prerequisites, third-party notices, writable sessions, missing-resource/crash recovery, checksums and upgrade instructions verified; full user workflow passes |

Do not advance past critical failed tests. Keep historical P0 validation separate from product implementation; later-phase infrastructure or placeholder interfaces do not complete their milestone. Optional deferrals must be explicit.

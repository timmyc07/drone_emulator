# Roadmap

1. Finish Machine B audit and install only approved Visual Studio/CMake/Ninja/Python components.
2. Validate the Windows C++20 preset, CTest, Python smoke test, GPU/driver, and WSL2 state.
3. Verify Unreal Engine 5.7 compatibility with the selected MSVC toolset after explicit installation approval.
4. Run the GitHub Actions workflow and record its actual result.
5. Start P1 core simulation design after both platform baselines are independently evidenced.

## Current continuation — 2026-10-14

The checklist above is preserved from the prior Machine A session. The user selected existing UE 5.8.3 instead of 5.7. Windows compiler/Python/Unreal evidence is now available; WSL needs an approved reboot before Ubuntu installation. Mac's required Python 3.12 and merged-source verification remain incomplete. See PROJECT_STATUS for final local and CI results.

## Product phases

The full governing requirements and phase gates remain in [MASTER_PLAN.md](MASTER_PLAN.md).

| Phase | Deliverable and acceptance |
| --- | --- |
| P0 Infrastructure | Repository, both platform instructions, environment evidence, builds/tests, CI configuration and recorded cross-platform results; honor strict host/Unreal gates in P0_PLAN |
| P1 Flight Dynamics | Headless math, 6-DOF, propulsion/environment, reference quadrotor/configuration, telemetry; required physics tests and hover pass |
| P2 Flyable Unreal Prototype | C++ bridge, visible quadrotor/simple level, input/camera/HUD, start/pause/reset; controls affect actual validated dynamics |
| P3 Usable Simulator | Menus, aircraft/environment selection, settings/gamepad, modes, recording/playback, Windows package; user can run without Editor |
| P4 Multiple Vehicle Classes | Additional multirotors, fixed-wing, VTOL, rotorcraft and backend adapters; acceptance defined before implementation |
| P5 Advanced Simulation | Sensors/weather/aerodynamics, PX4/ArduPilot SITL, environments; dependency compatibility and acceptance defined first |
| P6 Vehicle Library | Public-data configurations, provenance/confidence, non-weaponized approximations and assets; no unsupported fidelity claims |
| P7 Product Release | Installer/portable distribution, dependencies/versioning, error handling, upgrades and release verification |

Do not advance past critical failed tests. Keep P0 free of flight dynamics and SITL installation; optional deferrals must be explicit.

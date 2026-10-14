> Active target override (2026-10-14): the user now requests a browser drone game without Unreal, retaining the same ZIP drone. Reuse the existing C++20 core through project-local WebAssembly tooling and a local-only browser build. Further Unreal build/migration/package tasks are suspended. Original P0 host results below remain historical; they neither block independent browser work nor prove browser acceptance. Current commands and new verification gates are in [WEB_GAME.md](WEB_GAME.md) and [PROJECT_STATUS.md](PROJECT_STATUS.md).

> Retirement update (2026-10-14): the user authorized removal of the Unreal application, Unreal-specific tools, Windows desktop packaging tools and obsolete WSL preparation script. The local drone source assets and historical reports remain. P0/Unreal setup details below are retained as historical records, not current setup instructions.

> Browser validation update: project-local Emscripten 6.0.12 build, WebAssembly runtime/native equivalence and current Windows 6/6 CTests passed. Actual Chrome model rendering and basic flight controls passed. Remaining browser QA, new hosted CI and other-machine acceptance retain separate statuses in PROJECT_STATUS.

> Historical continuation (2026-10-14): Windows bootstrap gates are verified. Product commit `7f21881c1b8adbe252f74793250bbb262b1df49b` passed hosted Windows 6/6 and macOS ARM64 5/5 CTests plus Python 3.12.10 smoke checks in [run 38542931069](https://github.com/timmyc07/drone_emulator/actions/runs/38542931069). The user later authorized P1 and Unreal work, then explicitly retired the Unreal application and desktop tools. The P0-only scope and host statuses below describe the original bootstrap session; see [PROJECT_STATUS.md](PROJECT_STATUS.md) for current scope.

<!-- Mac evidence imported from origin/main a7b3016; not rerun on Windows. Historical statuses below describe that prior host session. Current Windows results follow. -->
# P0 Plan

- **P0.0 Environment Audit:** PASS for the Mac audit tool and JSON report; Windows audit is prepared but NOT_TESTED.
- **P0.1 macOS Bootstrap:** PASS for native arm64 detection, Apple Clang C++20, project-local CMake/Ninja, Python venv smoke test. Python 3.12, Homebrew, and full Xcode remain unavailable.
- **P0.2 Windows Bootstrap:** DEFERRED to Machine B.
- **P0.3 Unreal Verification:** BLOCKED/DEFERRED; no Unreal installation or Windows editor is available in this session.
- **P0.4 WSL2:** DEFERRED; Windows-only and not inspected from this Mac.
- **P0.5 Repository and Build Infrastructure:** PASS locally; files, presets, CTest, ignore rules, and Git history are prepared.
- **P0.6 Automated Verification:** PASS locally for macOS C++20/CTest; Windows and hosted CI are NOT_TESTED.
- **P0.7 Resource Baseline:** Mac disk and build evidence recorded; full peak-memory instrumentation is DEFERRED.

---

## Windows session and integration update — 2026-10-14

# P0 development environment plan

Session date: 2026-10-14 (Asia/Taipei). Scope: Windows host and local P0.6/P0.7 verification; macOS requires a separate session.

## Starting state and authority

The supplied Master Prompt v1.0 is the bootstrap specification. Initial read-only inspection found an empty workspace, no existing `AGENTS.md`, no `docs/P0_PLAN.md`, and no Git repository. This plan initializes the missing documentation; no prior project settings were available to preserve.

The user supplied `https://github.com/timmyc07/drone_emulator.git`; an external read-only remote check succeeded and reported no refs. The user initially approved UE 5.7, necessary Visual Studio components, and WSL2 with Ubuntu 22.04 installation, then selected the existing UE 5.8.3 as the Unreal target. No duplicate Unreal or C++ toolchain installation is needed. The first Unreal build exposed a missing .NET Framework SDK for SwarmInterface; a component-only Visual Studio repair is covered by the approved necessary components. Restart still requires separate confirmation. Preserve all existing configurations and installations.

## Execution and acceptance

| Stage | Work and required evidence | Current constraint |
| --- | --- | --- |
| P0.0 | Read-only Windows audit; structured JSON and Markdown; OS/CPU/RAM/GPU/disk/tools/permissions/network/WSL | Record failed probes independently; personal reports ignored |
| P0.2 | Reused VS 2022/MSVC 14.44, SDK, bundled CMake/Ninja; C++20 and Python 3.12 venv smoke tests | Earlier fixture clean build and 2/2 CTest PASS; Python/pip PASS; merged-source final test count in PROJECT_STATUS |
| P0.3 | Verify selected UE 5.8.3 C++ project build, Editor startup, actor and simple level execution, logs and resource use | PASS: build, headless automation, DX12 game world with 180 ticks, and graphical Editor automation; manual visual inspection NOT_TESTED |
| P0.4 | Inspect WSL/config first; approved WSL2 and Ubuntu 22.04 setup; verify distribution version and resource limits | WSL 3.0.3 MSI exit 0; Virtual Machine Platform DISM exit 3010, REBOOT_PENDING; Ubuntu DEFERRED |
| P0.5 prerequisite | Minimal headless C++20/CMake/CTest infrastructure, durable rules and ignore policy | Empty workspace requires bootstrap to run local tests |
| P0.6 local | Configure/build/test through CMake Presets; Windows validation and Python smoke test | CI configuration is not a successful CI run; macOS NOT_TESTED |
| P0.7 local | Actual RAM/CPU/disk/GPU observations and timed build/process memory sampling | Earlier pre-integration core fixture: 7.462 s wall, 3.0 s CPU, 241.10 MiB sampled aggregate working set; merged source baseline and Unreal measurements are recorded in PROJECT_STATUS |

Historical P0-C Unreal tests passed, including graphical Editor and DX12 execution; manual visual inspection was NOT_TESTED. Unreal and WSL setup are no longer current gates. P0-D required repository/build/rules/ignore/CI files and instructions for both platforms. Prior native Mac compiler/CTest evidence is preserved above, but strict P0-A remained incomplete because that host only verified Python 3.9.6, not required 3.12.

## Concurrent repository integration

During this Windows session the user pushed the existing Machine A scaffold to the supplied remote. Integration uses `origin/main` commit `a7b3016` as its base, preserving its sources, platform presets, prior Mac evidence, and governing `MASTER_PLAN.md`. Earlier Windows fixture tests and measurements describe the pre-integration fixture; see `PROJECT_STATUS.md` for merged-code tests and final metrics. CI needs actual workflow evidence before PASS.

## Configuration and resource policy

- Use project-local build and virtual-environment directories, leaving global PATH untouched.
- Inspect existing `.wslconfig`; preserve unrelated options, create a timestamped backup, and obtain approval for a concrete change. Suggested limits are 8 GB RAM and 4 GB swap; do not present these as applied limits until verified.
- Use a lightweight Unreal scene, initially 1920×1080, Medium scalability, DX12 when supported, reduced/disabled Lumen, and disabled hardware ray tracing.
- Do not install PX4/ArduPilot during P0. Validate their Ubuntu support separately before later installation.
- Save detailed local evidence under ignored `reports/`; keep sanitized results in project documentation.

## Required closeout

Update `PROJECT_STATUS.md` with exact commands, results, installed versus preexisting components, approvals, blockers, and next executable actions. Commit and push to the supplied remote when Git identity/authentication allow it; do not invent identity or create a public remote. Mark incomplete gates explicitly and retain failed-operation logs.
# Browser interaction follow-up — 2026-10-14

Default camera is first-person. Pointer Lock and browser Gamepad API are used without adding a runtime package. Verify physical controls and actual browser rendering before accepting this interaction work. Future scenario mechanics must be separately modeled and validated; briefing tags are not a substitute for wind, sensor, RF, payload or multi-vehicle simulation.

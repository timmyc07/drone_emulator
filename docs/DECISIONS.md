# Product implementation decisions — 2026-10-14

The user authorized implementing the product beyond P0. Three parallel preimplementation reviews covered current official compatibility, existing code/failure risks and project maintenance/acceptance. Additional focused reviews preceded the Windows CI quoting repair, replay validation and closeout script correction. Only reviewed changes above the required confidence threshold were applied; passing builds are still required separately.

- Keep one portable C++20 dynamics implementation, compiled by both CMake and the Unreal plugin. Physics uses SI/ENU/FLU, Hamilton body-to-world quaternions and a 0.004 s fixed step. Unreal only converts/interpolates snapshots; Chaos cannot also move the vehicle. Game-thread ownership avoids unsynchronized cross-thread state.
- Reuse Eigen, nlohmann/json and PicoSHA2 instead of writing matrix decomposition, a JSON parser or a cryptographic hash. Preserve pinned source and notices. GitHub research also evaluated Project AirSim and JSBSim; importing their full platform/backend stacks is unnecessary for the first self-contained quadrotor, and their broader capabilities are not claimed implemented.
- Validate complete positive-definite inertia and rotor allocation, reject nonfinite state/configuration, and stage a step before publishing it. Controllers and motor lag generate movement through dynamics. Ground contact is a documented flat, inelastic approximation; scenery does not imply terrain collision fidelity.
- Require vehicle provenance, assumptions and exact-byte SHA256. Treat the user-provided visual drone as cosmetic, with physical properties explicitly estimated. Keep external raw assets local pending their redistribution terms.
- Distinguish headless input re-execution (version/hash/timestep checks and trajectory comparison) from desktop recorded-state playback. Use explicit format/schema versions and reject malformed/corrupt data; do not describe state playback as deterministic simulation.
- Retain the P0 fixture unchanged in purpose; product implementation lives under `unreal/UAVSim`. Test the lightweight environment first, then complete the user's requested Electric Dreams environment and profile its actual resource use.
- Repair Windows hosted CI by preserving raw `cmd` quoting in a process-local developer-environment helper. Regressions cover spaces/parentheses, child isolation and nonzero exit. Action v7 versions were proven valid by the actual hosted run; changing them would not fix the path bug.
- A validation script containing expected native failures must explicitly `exit 0` only after all checks and transcript cleanup succeed. PowerShell otherwise leaves the final native rejection in `$LASTEXITCODE`, misleading wrappers. Exceptions still skip the success exit. This follows [PowerShell's exit-code semantics](https://learn.microsoft.com/en-us/powershell/module/microsoft.powershell.core/about/about_automatic_variables#lastexitcode) and [upstream discussion](https://github.com/MicrosoftDocs/PowerShell-Docs/issues/9194).

P1 Windows evidence does not complete the whole product. Final interactive operation, packaged launch, Electric Dreams, real gamepad hardware, current cross-platform CI and P4–P7 remain independently tracked. Earlier P0-only restrictions below document the earlier session, not a ban on the subsequently authorized product work.

## Historical bootstrap decisions

<!-- Mac evidence imported from origin/main a7b3016; not rerun on Windows. -->
# Decisions

- The core remains a headless C++20 CMake target so Unreal is optional for P0 core CI.
- CMake and Ninja were installed only inside the project-local `.venv`; Homebrew and full Xcode were not installed without explicit approval.
- The Mac system Python is 3.9.6, so Python 3.12 is recorded as a missing target dependency rather than misreported as available.
- Windows and Unreal statuses remain NOT_TESTED/DEFERRED because this session cannot access Machine B.
- GitHub Actions uses the explicit `macos-14` and `windows-2022` hosted labels; workflow success is not claimed until it runs.

---

## Windows session and integration update — 2026-10-14

# P0 decisions and prechange review

Date: 2026-10-14 (Asia/Taipei).

## Prechange three-review gate

Three parallel read-only agent reviews completed before implementation:

1. Official compatibility: reviewed current Microsoft/Epic/platform documentation and tool compatibility requirements.
2. Existing code/configuration and risks: inspected the initially empty workspace and detected host/tool installations. No existing repository code or deployment settings were available to reuse or overwrite.
3. Conventions/maintenance: read the supplied Master Prompt and Ponytail full skill; verified absence of hidden repository configuration; checked official CMake/CTest documentation and relevant GitHub starters.

These reviews support conservative local bootstrap above the required 90% confidence threshold. They do not substitute for executed build/editor/test evidence. Unknown results remain NOT_TESTED or BLOCKED.

## Reuse and dependency decisions

GitHub research inspected [cpp-best-practices/cmake_template](https://github.com/cpp-best-practices/cmake_template), including its [presets](https://github.com/cpp-best-practices/cmake_template/blob/main/CMakePresets.json), and [Kitware's official CMake tutorial](https://github.com/Kitware/CMake/tree/master/Help/guide/tutorial). Reuse the established target-based build, local binary-directory, and CTest registration patterns. Do not import the entire starter: its package management, UI, sanitizers, and other integrations are unnecessary for P0.

Use native CMake/CTest and the C++20 standard library. Build a small headless compiler/system smoke application with an explicit failing exit status for failed checks. Do not add flight dynamics or couple core acceptance to Unreal. Follow [official CMake Presets](https://cmake.org/cmake/help/latest/manual/cmake-presets.7.html) and [add_test](https://cmake.org/cmake/help/latest/command/add_test.html) documentation; preserve local `CMakeUserPresets.json` through ignore rules.

## Host and installation decisions

- Reuse detected VS 2022 17.14, MSVC 14.44, bundled CMake/Ninja, Windows SDKs, Git, and Python 3.12 before considering installation. Establish compiler compatibility through actual compilation.
- Initially UE 5.7 was required and absent. The user subsequently selected installed UE 5.8.3, superseding that target. Reuse 5.8.3 without a duplicate engine installation. Acceptance still requires actual build/editor/scene evidence.
- Repeated three-review compatibility/code/maintenance checks for the UE 5.8 target before implementation. Installed engine SDK configuration accepts actual compiler product 14.44.35229 (frontend 19.44.35229), despite its legacy 14.44.35207 directory label, and SDK 26100. No C++ compiler replacement is needed.
- The first actual Unreal build failed because SwarmInterface could not find NetFxSDK. Reviewed a minimal repair using existing Visual Studio Installer to add only `Microsoft.Net.Component.4.8.SDK`, with `--quiet --norestart`; preserve existing workloads, avoid forced process closure, and verify registry/libraries before rerunning the build. User approval already covers necessary Visual Studio components. This is a repair plan, not an installation success claim.
- Initial approval covered UE 5.7, required Visual Studio components, and WSL2 with Ubuntu 22.04. Only necessary operations proceed. Microsoft-signed WSL 3.0.3 MSI completed with exit 0; Virtual Machine Platform enable returned DISM 3010, requiring reboot. Ubuntu remains DEFERRED. Computer restart has not been requested or performed and requires separate confirmation.
- Preserve `.wslconfig` if present. Resource proposals are not active settings. WSL shutdown requires approval and must account for running workloads.
- Avoid changing machine-wide environment variables, GPU drivers, security settings, or global Git identity. Existing configuration must not be overwritten.

## Evidence and repository decisions

Detailed machine reports and generated build/Unreal/venv outputs stay ignored. Share sanitized version/result summaries in documentation. Treat build process memory sampling as an observed sample rather than an exact peak. Unreal scene metrics remain NOT_TESTED until the scene actually executes.

The earlier Windows fixture clean build and CTest verification passed 2/2 tests. Observed baseline: 7.462 s wall time, 3.0 s sampled aggregate process CPU, 241.10 MiB sampled aggregate peak working set. These are pre-integration fixture measurements, not the preserved Mac scaffold's merged-source baseline. Final merged tests and metrics belong in PROJECT_STATUS. Python 3.12.10 local venv and pip 25.0.1 smoke checks passed. Samples do not establish an exact peak.

## Integration and final Unreal evidence

The remote changed during this session: the user concurrently pushed the Machine A scaffold. Preserve `origin/main` commit `a7b3016` as the integration base, retaining existing source names, platform presets, documentation, and MASTER_PLAN V2.0 unchanged. Imported Mac results are previous-host evidence, not tests executed by this Windows session. Mac Python 3.9.6 does not satisfy its Python 3.12 target.

The reviewed .NET Framework 4.8 SDK component repair completed with installer exit 0 and SDK header verification. Subsequent UE 5.8.3 C++ build, headless automation, DX12 game-world execution with 180 ticks, and graphical Editor automation passed. Manual visual inspection was not performed. Keep the smoke project isolated from the headless core; its actor is environment verification, not flight dynamics.

Current workflow action versions and runner support were checked against official GitHub sources before changes. Preserve one core workflow and existing platform preset intent; actual CI results must be recorded separately from local Windows or prior Mac results.

The user supplied `https://github.com/timmyc07/drone_emulator.git`; the initial read-only remote query succeeded with no refs. Use that remote without changing its visibility or replacing another configured remote. Git identity was initially absent; verified GitHub profile login/id now supports a repository-local GitHub noreply identity without altering global configuration. Commit/push are required when available, and any unresolved authentication/sandbox issue must be recorded as a blocker.

Prepare macOS workflows without claiming execution. A configured GitHub Actions workflow remains NOT_TESTED until a real run succeeds. Overall P0 remains incomplete while any mandatory platform or Unreal gate lacks verified evidence.

Preserve the remote's original core targets, version header, presets and Git LFS rules. Preset fields only need schema 2, so correct the former schema 6 / CMake 3.20 mismatch without increasing the minimum version. Hosted CI uses current official actions v7; platform runner labels remain the original supported macos-14 and windows-2022. Disable the unused AndroidFileServer plugin in the isolated Unreal fixture and ignore Editor-generated input defaults; retain only intentional project configuration in Git.

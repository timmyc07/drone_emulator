# Project Status

## Active browser pivot — 2026-10-14

The user changed the deliverable to a browser drone game without Unreal and retained the same supplied ZIP drone as the default model. On 2026-10-14 the user authorized removal of the Unreal application and desktop-only tools, including local Unreal build outputs. The source drone and Electric Dreams assets under ignored `local-assets/`, and historical validation reports under ignored `reports/`, remain. Desktop status below is historical evidence; no earlier PASS is carried over to browser acceptance.

## Unreal desktop retirement — 2026-10-14

The user explicitly overrode the earlier source-preservation rule and authorized retiring the old desktop application. Removed the tracked `unreal/` and `tools/unreal/` trees, Unreal build/package/validation tools, obsolete WSL preparation, and Windows desktop instruction pages. The `unreal/` tree contained 639 files totaling 11.31 GiB, including generated build/cache/content files. Kept the browser's C++20 core, native validation, WebAssembly and Three.js code, hosted core CI, original local assets and historical `reports/` evidence.

Before removal, the dirty tracked Unreal/package diffs and two untracked Unreal scripts were copied into ignored `reports/unreal-retirement-worktree-backup-20261014/`. The backup includes a patch for tracked changes and exact copies of the untracked scripts, preserving the user's unfinished work outside active source paths.

Cleanup verification on this Windows host: existing native-core CTest suite PASS (6/6, 2.56 s); browser game-logic and site-build preflight tests PASS; `git diff --check` PASS. The static-server test hit sandbox `EPERM` at Node.js `realpath` for its temporary directory, even after moving `TEMP` under the workspace; no browser server code changed. The earlier successful HTTP boundary evidence remains in `reports/web-final-validation.log`.

Three parallel preimplementation reviews covered current official/GitHub compatibility, core/frontend failure risks and conventions/operations/licensing before browser changes. The reviewed architecture reuses the C++20 core through project-local Emscripten 6.0.12 and Three.js rendering. The local server exposes only prepared `dist/web` files on `127.0.0.1:4173`. The scene is independently created: [Electric Dreams is UE-only](https://www.fab.com/listings/d79688f5-29be-4fb2-a650-2d4a813f5306). Supplied FBX/textures remain local and excluded from GitHub.

| Browser gate | Captured result on this Windows host |
| --- | --- |
| Project-local Emscripten / existing core WebAssembly build | PASS: Emscripten 6.0.12 compiled the existing core; `reports/web-build.log`. Generated `uavsim.mjs` is 83,774 bytes; `uavsim.wasm` is 342,798 bytes |
| WebAssembly runtime and native equivalence | PASS: `reports/web-wasm-smoke.log`; 5,000 native recorded-input steps reproduced with maximum state error `1.4094628242311558e-15` (threshold `1e-9`). Vehicle provenance, 20 s hover, invalid input, pause/reset and failure recovery passed |
| Same-ZIP default drone / materials | PASS in actual Chrome and in-app browser rendering: supplied FBX shown with red/white surfaces; DOM reports `model=supplied-fbx`, `textures=6`. This is browser evidence, separate from the historical Unreal material failure |
| Flight input and session controls | PASS for observed start and approximately 1 m hover for 1 min 38 s, W movement at about 2.2 m/s, Space climb to about 4.6 m, Q heading about 65 degrees, camera switch, reset and pause. Holding P for 1,500 ms left the game paused. Imperial selection changed hover HUD to 3.3 ft |
| Recording downloads and playback | PASS: actual `uavsim-flight.json` download contained 216 frames; `uavsim-telemetry.csv` header and first row were read successfully. Importing the downloaded JSON through the file chooser showed recorded-state playback; both export buttons were disabled during playback. Importing malformed `{}` JSON showed the explicit invalid-format message |
| Responsive layout | PASS in the in-app browser at a 375×812 viewport: menu and live flight HUD fit, with `document.scrollWidth = innerWidth = 375`. Viewport restored after inspection. This does not establish physical phone/touch acceptance |
| Six-ring course completion | NOT_TESTED in full interactive play; basic movement and session controls do not establish completion of all six rings |
| Static server boundary / bundle preflight / gameplay tests | PASS on the final rerun: HTTP method/path/MIME checks, required-file/type preflight and gameplay logic tests. Missing build inputs fail before modifying existing output. Restricted sandbox `realpath` EPERM is an environment limitation, retained separately |
| Browser missing-resource behavior | PASS in an isolated loopback fixture on port 4174: missing supplied FBX showed `無法開始：找不到指定的 QuadDrone FBX 模型。請執行網頁準備程序。`; missing WASM showed the module-fetch abort. Start stayed disabled in both cases. The normal prepared site was preserved |
| Local dependency notices / asset exclusions | PASS: all eight core notice/provenance files and the Three.js LICENSE matched prepared output by SHA256. Git ignore checks covered the source drone, prepared site, generated WASM, vendor and local SDK |
| Current native core regression | PASS: `reports/web-native-regression.log`, Windows configure/build and 6/6 CTests, 2.62 s |
| Hosted core CI for browser-source commit | PASS: commit `9ac524a9be891a6fc26cfc71d61ee55442745763`, [run 38550487896](https://github.com/timmyc07/drone_emulator/actions/runs/38550487896); Windows and macOS jobs both completed successfully. This workflow checks native core, not browser/WASM rendering |
| Public site / raw asset redistribution | Out of scope; local delivery only, drone redistribution license unverified |

Commands and current boundaries: [WEB_GAME.md](WEB_GAME.md). Final browser observations are retained in `reports/web-browser-acceptance.json`, with a desktop screenshot at `reports/web-browser-desktop.jpg`. Browser identities were recorded as Chrome and the Codex in-app browser; exact browser build numbers were not captured. The final validation reran the HTTP, preflight, gameplay and 5,000-step WebAssembly equivalence checks successfully; command evidence is `reports/web-final-validation.log`. Hosted job metadata is retained in `reports/web-ci-jobs.json`. Core equivalence uses finite recorded trajectories; it does not establish calibrated real-aircraft behavior. Full interactive course completion, physical touch/gamepad hardware, other browser engines and other machines remain separate unverified gates.

## Historical desktop product session — 2026-10-14

**Host:** Windows 11 x64, Ryzen 7 7700, 32 GiB installed RAM, RTX 5070. **Source version:** 0.2.0. **Progress:** P1 implemented and verified on this Windows host; P2/P3 product integration is undergoing acceptance. The complete MASTER_PLAN product, Electric Dreams integration and P4–P7 are not complete.

| Component/gate | Actual evidence and status |
| --- | --- |
| Portable dynamics/control/configuration | IMPLEMENTED AND VERIFIED on Windows: Eigen math, RK4 6-DOF, validated inertia/rotor geometry, first-order motors, drag/wind/gravity/flat ground, rate/attitude/altitude modes, PID reset/anti-windup, strict JSON/provenance and SHA256 |
| Core CTest | PASS: 6/6 in `reports/product/final-core-validation.log`, 2.34 s; includes Windows environment path regression, physics, config, original smoke and invalid argument cases |
| Hosted core CI / source synchronization | PASS: pushed commit `7f21881c1b8adbe252f74793250bbb262b1df49b`, run 38542931069; Windows 6/6 and macOS ARM64 5/5 CTests plus Python 3.12.10 smoke checks, verified from actual job logs |
| Headless end-to-end | PASS: `reports/product/20261014-092001-6027898/validation.log`; 30 s hover and recorded-input replay, 7,500 steps each, maximum hover/replay error 0; takeoff-land peak about 2.174 m and final ground contact/zero speed; 7,501 CSV rows and independently checked SHA256 |
| Negative input/replay checks | PASS: malformed/invalid configuration, nonfinite duration, output overwrite, changed config hash, altered trajectory and embedded NUL corruption rejected. The NUL test reproduced acceptance of corruption in the previous binary, then passed after the fix |
| Unreal product Editor build | PASS: `reports/product/unreal-build.log`, `Result: Succeeded`; earlier header/lifecycle build failures retained separately |
| Unreal adapter/lifecycle automation | PASS: `reports/product/editor-tests.log`, `UAVSim.Product.AdapterAndLifecycle` Success; coordinate mapping, physical input directions, frame schedules, pause/reset and disabled Chaos checked |
| Product DX12 runtime smoke | PASS: `reports/product/runtime-smoke.log`, model `SM_DefaultDrone`, fixed step 0.004 s, 752-frame save/playback and `UAVSIM_RUNTIME_PASS` at frame 179; this is automated runtime evidence, not full manual UI acceptance |
| Drone asset import | PASS: `reports/product/asset-import.log`, `UAVSIM_DRONE_IMPORT_PASS ... textures=6`; raw assets stay local pending redistribution-license verification |
| Final replay hardening and repeatable Unreal validation | PASS: `reports/product/replay-build.log` (30.82 s), then `reports/product/unreal-20261014-092847-1541711/validation.json`: both `AdapterAndLifecycle` and `ReplayValidation` Success with zero errors; DX12 smoke frame 235, altitude 0.5871 m, real default model and pause/reset/save/replay verified. Oversized/nonfinite states, origins and motor values are rejected before rendering |
| Desktop menu/settings/input | PARTIAL interactive evidence: start/approximately 1 m hover for 33 s, pause, save/playback, menu/Esc, feet units, 1920×1080 apply and exit exercised. Lighting overlay, dropdown contrast and stale menu arming display found; repairs and full regression pending. Full settings persistence, landing, visual asset quality and gamepad acceptance remain unverified |
| First Windows package | PASS: `reports/product/package.log`, BuildCookRun 175.18 s, exit 0; `dist/UAVSim-Windows-20261014-173017/Windows/UAVSim.exe`; 59 files / 997,739,689 bytes before runtime-generated files. Eight notice/provenance files and vehicle JSON hashes verified |
| First packaged runtime | PASS: `reports/product/unreal-20261014-093336-4348265/validation.json`, actual game executable launched without Editor; separate automation/runtime gates also PASS. This package predates the current UI/lighting repairs; clean-machine release acceptance remains NOT_TESTED |
| First-package visual acceptance | FAIL: white fallback drone material; `reports/product/package.log` lines 327–328 report `M_DefaultDrone` compile failure, `(Node OneMinus) Missing 1-x input`. Material graph validation and packaging failure detection are being repaired alongside lighting/UI defects; visual regression remains pending |
| Electric Dreams | Acquired locally for UE 5.8.3. CloseRange preflight found 1,216 `/Game` dependencies including 107 external actors and five external objects, with no missing source packages. First native resave migration stopped safely at the 20 GiB process cap after 149.37 s (peak private 21,514,334,208 bytes); failed stage retained and product unchanged. A lower-memory copy migration is being reviewed. Product rendering/performance/cooking remain NOT_TESTED |
| P4–P7 | PLANNED: multiple vehicle families, advanced sensors/SITL/weather, library expansion and product release gates not implemented/verified |

The local reference configuration hash used above is `2fff42c10efba4dfb7a52be4e1f7644fa97dee3949789501f00353fe8cc3f0dd`. Numerical results apply to the fictional engineering-estimate quadrotor, not a calibrated real aircraft. Unit test details and acceptance thresholds are in [P1_ACCEPTANCE.md](P1_ACCEPTANCE.md).

### Hosted CI and remaining infrastructure

[Run 38542931069](https://github.com/timmyc07/drone_emulator/actions/runs/38542931069) passed for product commit `7f21881c1b8adbe252f74793250bbb262b1df49b`. Actual [Windows job logs](https://github.com/timmyc07/drone_emulator/actions/runs/38542931069/job/115781862523) record MSVC 19.44.35229.0 and 6/6 CTests (3.90 s); [macOS ARM64 job logs](https://github.com/timmyc07/drone_emulator/actions/runs/38542931069/job/115781863285) record AppleClang 15.0.0.15000309 and 5/5 CTests (1.46 s). Both record Python 3.12.10 venv/import/filesystem smoke PASS. The extra Windows test covers developer-environment paths. These hosted jobs validate the portable core and bootstrap, not Unreal or the full product workflow.

Historical [run 38536058546](https://github.com/timmyc07/drone_emulator/actions/runs/38536058546), P0 commit `0531568c3a741ca988cafdb0059471746f3a8883`, passed macOS but failed Windows before CMake on the quoted `Program Files` developer-script path. The process-local helper and spaces/parentheses, child-isolation and failure-handling regression resolved that failure in the successful product run above. All three Actions v7 resolved and ran. The user made the existing repository public; log inspection used the GitHub connector, without credential extraction.

Historical platform note: Machine A needed its own Python 3.12 and current-source check; hosted macOS evidence did not change that machine's installed software. WSL is not a browser-game prerequisite.

### Commands and next gates

Historical commands included `tools/bootstrap/build_windows.ps1`, `tools/validation/validate_product.ps1 -SkipBuild`, Unreal `Build.bat UAVSimEditor Win64 Development`, product automation, asset import and DX12 runtime. Exact outputs remain under ignored `reports/product`. The restricted MSVC C1902 failure was preserved and the same compiler succeeded outside the sandbox. The product validator's failure handling was verified: a wrapper test returned 0 (`validator-exit-contract.log`) while the script with a missing executable returned 1 (`validator-missing-executable.log`). Unreal-specific validators were removed during retirement.

The first-package run, executable checksums and limitations remain in local `reports/product` and the historical entries below. At retirement, interactive flight/settings/session acceptance, clean-machine acceptance and Electric Dreams integration were incomplete; these desktop tasks are retired. Keep both hosted CI jobs passing for subsequent source changes. Preserve browser hardware and host-specific gates as NOT_TESTED.

## Historical P0 closeout (preserved)

**Session date:** 2026-10-14 (Asia/Taipei)
**Current host:** Machine B, Windows 11 Pro 24H2 x64, build 26100.9457.
**Historical phase at P0 closeout:** Windows local mandatory gates PASS; Machine A Python 3.12 was missing. See the current product/CI section above for subsequent work.

## Current Windows results

| Stage/gate | Actual result |
| --- | --- |
| P0.0 inventory | PASS: Ryzen 7 7700, 8 cores/16 logical CPUs; two 16 GiB RAM modules, 32 GiB installed, 30.88 GiB OS usable; RTX 5070, driver 617.14, 12,227 MiB VRAM |
| P0.2 / P0-B | PASS: existing VS Build Tools 17.14.37710.0, MSVC 19.44.35229, SDK 26100; C++20 compile/link, 3/3 CTests, Python 3.12.10 local venv, pip 25.0.1 |
| P0.3 / P0-C | PASS: user-selected existing UE 5.8.3, real C++ Editor target build, headless level/actor automation, graphical Editor startup/test, and saved DX12 scene with 180 game-world actor ticks; manual visual inspection NOT_TESTED |
| P0.4 | WSL 3.0.3 installed; VirtualMachinePlatform REBOOT_PENDING (DISM 3010); Ubuntu 22.04 DEFERRED until separately approved reboot |
| P0.6 local | PASS: original uavsim_smoke plus cpp20_smoke and invalid-argument test; information executable reports version 0.1.0; Python venv/import/filesystem smoke and pip checks PASS |
| P0.7 local | PASS for actual sampled Windows build/Editor/scene observations; true idle baseline and exact process-tree peaks NOT_TESTED |
| P0-A Mac | Prior C++/CTest evidence preserved below; Python 3.12 missing and merged source NOT_TESTED on Machine A |
| Hosted CI (historical visibility) | Status had not been confirmed at this P0 closeout. The current section above records the earlier Windows failure and subsequent Windows/macOS PASS for the product source |
| Remote synchronization | PASS: implementation commit `436d1c7` pushed normally; `git ls-remote` matched local SHA `436d1c76b81dddde5710b11718cb6870917cf744`; remote history preserved |

## Changes and commands executed

- Initial local directory was empty. After the remote appeared during the session, fetched `origin/main` at `a7b3016` and completed three additional integration reviews. Preserved original core sources, version header, smoke test, master plan and Mac evidence; backed up overlapping local scaffold files before integration.
- Reused existing tools. No Unreal installation, compiler replacement, GPU-driver update, global PATH/Git identity change, `.wslconfig` change, or restart was performed. Repository-local Git identity uses the verified GitHub account's noreply address.
- Approved system changes: Microsoft-signed WSL 3.0.3 MSI (exit 0), VirtualMachinePlatform enable with `/norestart` (3010), and component-only VS `.NET Framework 4.8 SDK` repair (exit 0, required header verified).
- Historically executed `tools/environment/audit_windows.ps1`, `tools/validation/validate_windows.ps1`, `tools/validation/measure_windows.ps1`, and the now-retired Unreal fixture validator on this host.
- Core script executed `cmake --preset windows-x64-debug`, `cmake --build --preset windows-x64-debug`, and `ctest --preset windows-x64-debug` after importing only process-local VS developer settings. Corrected the prior Ninja single-config executable path.
- Unreal used `Build.bat P0ProbeEditor Win64 Development -Project=... -MaxParallelActions=2`, then Editor automation with JSON report inspection and separate DX12 game/editor runs. Lightweight generated cube level, 1920x1080 development resolution, Medium scalability groups, disabled Lumen/ray tracing.
- Checked all PowerShell syntax, preset/project JSON, original-source preservation, ignore rules and Git whitespace. Mac shell scripts retain their executable Git modes; their updated execution remains unverified on Machine A.
- Disabled the unused AndroidFileServer plugin in the isolated fixture, excluded Editor-generated input defaults, and reran the focused Editor level/actor automation test against the final configuration: PASS. Only intentional project settings were committed.

## Recovery evidence

The restricted sandbox initially caused MSVC C1902 during the compiler probe; the same installed compiler succeeded outside it. The first Unreal build failed because SwarmInterface could not locate NetFxSDK. After the approved component repair, actual Unreal compilation and tests succeeded. Both failures and successful retries are retained locally. Early 2-test scaffold results are historical; final integrated core acceptance is 3/3.

## Windows resource baseline

The integrated clean core workflow took **5.5058 seconds**, with 38 requested-100ms samples across 19 observed owned processes. Sampled maximum aggregate working set was **285,560,832 bytes (272.33 MiB)**; sampled cumulative CPU was **3.28125 seconds**. Active-session RAM used **16,481,378,304 / 33,157,967,872 bytes**; this was not an idle measurement. Prebuild GPU use was **1,559 / 12,227 MiB**, 6% utilization. C: free 450,232,631,296 bytes; D: free 861,367,222,272 bytes at measurement start.

Unreal measurements and limitations are in the final resource summary below. First successful cold project build took 61.47 seconds. GPU samples are host-wide and can include other applications and shader workers; process working-set samples can miss peaks and do not include all child processes. No FPS or flight-simulation performance is claimed.

## Evidence locations and next actions

Private detailed reports are ignored under `reports/environment`, `reports/validation`, `reports/performance`, `reports/unreal` and `reports/install`. Initial audits, failed attempts, bootstrap-only baselines and successful reports are preserved. The previous tracked Windows placeholder baseline is replaced by this sanitized summary; historical Mac reports remain unchanged.

Historical next actions at P0 closeout included a separately approved reboot, WSL/Ubuntu initialization and Machine A Python 3.12. Resource limits (8 GB RAM / 4 GB swap) were proposals, not applied settings. The desktop continuation guide was removed during retirement; current browser gates are listed at the start of this file.

## Governing plan

[CODEX MASTER PROMPT V2.0](MASTER_PLAN.md) remains unchanged. The user's explicit UE 5.8 selection supersedes the earlier 5.7 target for this environment verification.

## Prior Machine A session (historical, imported from a7b3016)

The following is the original Mac status record. Its Windows/CI NOT_TESTED statements described that earlier session; the Windows results above supersede them. These Mac results were not rerun from this host, and Python 3.9.6 does not satisfy the target 3.12 requirement.

# Project Status

**Session date:** 2026-10-14 (Asia/Taipei)
**Current host:** Apple Silicon MacBook Pro, macOS 26.6.2 arm64

## Verified

- Native Apple M5 Pro, 15 logical CPUs, 24 GiB-class memory, 621 GiB available disk.
- Apple Command Line Tools and Apple Clang 21.0.0 detected.
- Git 2.50.1 detected.
- Project-local CMake 4.4.4 and Ninja 1.13.2 installed in `.venv`.
- C++20 configure/build completed with Apple Clang.
- CTest: 1/1 test passed.
- Python venv creation and smoke test passed under Python 3.9.6.
- Audit JSON, CMake presets, platform scripts, CI workflow, and documentation created.

## Missing or deferred

- Python 3.12: NOT_INSTALLED/NOT_TESTED.
- Homebrew, full Xcode, VS Code, GitHub CLI: NOT_INSTALLED or unavailable on PATH.
- Windows MSVC, RTX 5070, Unreal Engine, WSL2: NOT_TESTED.
- GitHub Actions: NOT_TESTED until workflow is triggered.

## Next executable task

This next-action note is historical. Current browser and native-core checks are listed at the start of this file and in WEB_GAME.md.
## Primary plan

The governing product plan is [CODEX MASTER PROMPT V2.0](MASTER_PLAN.md).

## Final Unreal resource observations

Measured after applying all Medium scalability groups. Durations include process startup, testing and exit; RAM is sampled single-process working set, not exact peak or full process-tree memory. GPU readings are host-wide.

| Phase | Duration (s) | Sampled RAM (MiB) | Last sampled CPU (s) | Maximum sampled host VRAM (MiB) |
| --- | ---: | ---: | ---: | ---: |
| Editor automation | 50.971 | 2594.73 | 20.500 | 11390 / 12227 |
| DX12 simple scene | 21.507 | 2019.23 | 17.531 | 11515 / 12227 |
| Graphical Editor startup and level | 39.457 | 3395.12 | 32.703 | 11437 / 12227 |
# Browser control and scenario update — 2026-10-14

- PASS: `node --test web/game-logic.test.mjs tools/web/build-site.test.mjs tools/web/server.test.mjs` (3/3) and `node tests/wasm_smoke.mjs` passed on Windows 2026-10-14.
- PASS (unit): keyboard/gamepad semantic input, deadzone, clamping, scenario-catalog fallback and existing replay/gate checks.
- IMPLEMENTED: default first-person camera, click-initiated Pointer Lock mouse look, standards-based gamepad polling and 15 non-weaponized training templates.
- NOT_TESTED: physical gamepad hardware, pointer-lock interaction in a browser, rendered scenario selection and cross-browser behavior. These require a local browser session after rebuild.
- DEFERRED: wind, RF/GNSS, sensor, payload, moving-deck, contamination and multi-UAV models. Scenario labels make these limitations visible.

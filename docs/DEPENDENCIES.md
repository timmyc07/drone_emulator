# Current product dependencies — 2026-10-14

## Active browser target

The browser target supersedes Unreal as the current frontend. It retains the vendored C++ core dependencies below. Project-local Emscripten **6.0.12** compiled the existing core on Windows; its WebAssembly smoke and 5,000-step native equivalence checks passed. Compiler output is captured in `reports/web-build.log`, with runtime results in `reports/web-wasm-smoke.log`. Three.js **0.186.1** supplies rendering and FBX loading, with its MIT license retained. Node.js **24.19.0** handles preparation, build-output assembly and the built-in HTTP server without changing global PATH or adding a web framework/server dependency. See [WEB_GAME.md](WEB_GAME.md).

The fixed Three.js official npm archive is checked against integrity `sha512-blFeqb49wRCSGUGj7gtpfnSGHy2lwDk94RhUmS1c/hTby70kvChbWpkJ4Pm1390LqzzvTmzgXKHPEafJwCb8jA==`. No npm install lifecycle scripts are needed. Preparation retains package metadata, upstream license and provenance in `web/vendor/three`; assembly copies them into local `dist/web/vendor/three`. Core notices are copied into `dist/web/ThirdPartyNotices`.

GitHub/official research: [Emscripten SDK](https://github.com/emscripten-core/emsdk), [Emscripten build guidance](https://emscripten.org/docs/compiling/Building-Projects.html), [Three.js](https://github.com/mrdoob/three.js), [FBXLoader documentation](https://threejs.org/docs/pages/FBXLoader.html), [Three.js MIT license](https://github.com/mrdoob/three.js/blob/dev/LICENSE), and [Node HTTP documentation](https://nodejs.org/api/http.html). Current online documentation is checked against the actual selected tools; availability alone is not compatibility evidence.

Electric Dreams is excluded from the browser due to its [UE-only designation](https://www.fab.com/listings/d79688f5-29be-4fb2-a650-2d4a813f5306). The supplied drone's redistribution license remains unverified. Local generated bundles, SDKs and source assets stay ignored. No Unreal installation or build is required to run the browser game. Entries below preserve historical desktop and core provenance.

## Historical desktop and shared core dependencies

The portable C++20 core is separate from Unreal and has no package download at build time. Reused, pinned source dependencies and original license files are retained under `third_party`; their licenses do not replace the product or asset licenses.

| Dependency | Exact version/revision | License/use |
| --- | --- | --- |
| Eigen | 5.0.0, `549bf8c75b6aae071cde2f28aa48f16ee3ae60b0` | MPL-2.0, retained COPYING notices; vectors/quaternions, inertia and allocation solves |
| nlohmann/json | 3.12.0 | MIT; strict vehicle/recording parsing and metadata |
| PicoSHA2 | `161cb3fc4170fa7a3eca9e582cebd27cc4d1fe29` | MIT; exact configuration-byte SHA256 |
| CMake / CTest | Minimum 3.20; Windows installed 3.31.6-msvc6 | Existing build/test system; no external test framework required |
| Unreal Engine | Installed 5.8.3 | Existing Epic license; product C++ plugin/UMG/rendering, separate from portable core |
| GitHub Actions | checkout/setup-python/upload-artifact v7 | Both platform jobs passed in product run 38542931069; the earlier Windows environment quoting failure was unrelated to action versions |

See [third-party provenance](../third_party/README.md) for official sources, archive/header checksums and notices. No JSBSim, PX4, ArduPilot, ROS, geospatial or GPU simulation backend is installed or claimed integrated. Electric Dreams is requested content currently being acquired; its original license/entitlement and resource requirements apply. User-supplied drone assets have been imported locally but redistribution remains unverified; see [ASSETS.md](ASSETS.md).

The vendored product core at commit `7f21881c1b8adbe252f74793250bbb262b1df49b` passed [hosted run 38542931069](https://github.com/timmyc07/drone_emulator/actions/runs/38542931069): Windows MSVC 19.44.35229.0, 6/6 CTests; macOS ARM64 AppleClang 15.0.0.15000309, 5/5 CTests; Python 3.12.10 smoke checks on both. Local Windows physics/configuration and end-to-end results are recorded separately in PROJECT_STATUS. No new Machine A installation is claimed.

## Historical P0 host inventory

<!-- Mac evidence imported from origin/main a7b3016; not rerun on Windows. Historical statuses below describe that prior host session. -->
# Dependencies

## Machine A (verified in prior Mac session)

| Component | Result | Evidence |
|---|---|---|
| macOS | PASS — 26.6.2, arm64 | `reports/environment/mac-audit.json` |
| CPU/RAM | PASS — Apple M5 Pro, 15 logical CPUs, 25,769,803,776 bytes | audit JSON |
| Apple Clang | PASS — Apple clang 21.0.0.21000101 | audit JSON/build output |
| Command Line Tools | PASS — `/Library/Developer/CommandLineTools` | audit JSON |
| Git | PASS — 2.50.1 | audit JSON |
| CMake | PASS — project-local `.venv` package 4.4.4 | audit JSON/build output |
| Ninja | PASS — project-local `.venv` package 1.13.2 | audit JSON/build output |
| Python | WARNING — system Python 3.9.6; venv works | audit JSON and python smoke |
| Homebrew | NOT_INSTALLED | audit JSON |
| Full Xcode | NOT_INSTALLED | `xcodebuild -version` unavailable under CLT |
| VS Code / GitHub CLI | NOT_INSTALLED or unavailable on PATH | audit JSON |

The project-local `.venv` is intentionally ignored and contains only CMake/Ninja tooling. Python 3.12 remains required for the target environment and is not claimed as installed.

## Machine B

NOT_TESTED from this Mac session. Use `tools/environment/audit_windows.ps1` on Windows and follow `docs/WINDOWS_CONTINUATION.md`.

---

## Windows session and integration update — 2026-10-14

# P0 dependency inventory

Observed on Windows on 2026-10-14 (Asia/Taipei). The status column distinguishes detections from executed verification. See `PROJECT_STATUS.md` for final verification results and ignored `reports/` for detailed local evidence.

| Component | Observed version/state | P0 use |
| --- | --- | --- |
| Host | Windows 11 Pro, 10.0.26100, x64 | Windows target |
| CPU / memory | AMD Ryzen 7 7700, 8 cores / 16 logical processors; 32,380,828 KiB OS-visible RAM | Report usable RAM separately from nominal 32 GB |
| GPU / driver | NVIDIA RTX 5070; driver 617.14; 12,227 MiB reported VRAM | Detected; actual DX12 scene and graphical Editor tests PASS |
| Visual Studio 2022 | 17.14.37710.0 | Reuse existing C++ toolchain |
| .NET Framework 4.8 SDK | Initially missing NetFxSDK; component repair completed | PASS: installer exit 0, SDK header present, subsequent Unreal build passed |
| MSVC | Legacy toolset directory 14.44.35207; actual compiler product 14.44.35229 / frontend 19.44.35229 | Core compile/link PASS; UE SDK configuration accepts preferred 14.44 >= 35211 based on actual compiler version |
| Windows SDK | 10.0.19041 and 10.0.26100 installed | SDK 26100 accepted by installed UE SDK configuration |
| CMake | 3.31.6-msvc6, Visual Studio bundled | CMake Presets and CTest |
| Ninja | 1.12.1, Visual Studio bundled | Local build generator |
| Git | 2.53.0.windows.3 | Identity initially absent; GitHub profile login/id verified for repository-local noreply identity |
| Python | 3.12.10; pip 25.0.1 | Project-local venv, Python smoke and pip checks PASS |
| Unreal Engine | Existing 5.8.3 selected by user, superseding 5.7 target | PASS: C++ build, headless automation, DX12 scene 180 ticks, graphical Editor automation; manual visual inspection NOT_TESTED |
| WSL2 / Ubuntu | Initially absent; Microsoft-signed WSL 3.0.3 MSI installed with exit 0 | Virtual Machine Platform enable returned 3010: REBOOT_PENDING; Ubuntu 22.04 DEFERRED until approved reboot |
| macOS / Apple Clang | Prior Machine A evidence retained above; not rerun in Windows session | Mac Python 3.12 remains missing; merged source verification pending |

No third-party C++ runtime or test framework is required for P0. Use the C++ standard library and CMake's built-in CTest. Do not install simulator, geospatial, PX4, ArduPilot, or other future-stage dependencies.

## Compatibility sources

- [CMake Presets](https://cmake.org/cmake/help/latest/manual/cmake-presets.7.html) and [CTest test registration](https://cmake.org/cmake/help/latest/command/add_test.html): use a schema supported by installed CMake, project-local output directories, explicit build/test presets, and nonzero test failure exits.
- [Epic Visual Studio setup](https://dev.epicgames.com/documentation/en-us/unreal-engine/setting-up-visual-studio-development-environment-for-cplusplus-projects-in-unreal-engine): check current compatibility and installed engine SDK configuration; actual compiler product version takes precedence over a legacy directory label. Existing compiler/Windows SDK satisfy selection requirements; actual build exposed a separate NetFxSDK dependency.
- [VS2022 Build Tools components](https://learn.microsoft.com/en-us/visualstudio/install/workload-component-id-vs-build-tools?view=vs-2022) and [installer CLI](https://learn.microsoft.com/en-us/visualstudio/install/use-command-line-parameters-to-install-visual-studio?view=visualstudio): prefer component-only `modify --add Microsoft.Net.Component.4.8.SDK --quiet --norestart` against the existing instance, without replacing workloads or forcing a reboot.
- [Epic automation testing](https://dev.epicgames.com/documentation/unreal-engine/run-automation-tests-in-unreal-engine?lang=en-US) and [command-line reference](https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-engine-command-line-arguments-reference): headless test reports establish tested runtime behavior, not graphical rendering acceptance.
- [Microsoft WSL installation](https://learn.microsoft.com/en-us/windows/wsl/install) and [WSL configuration](https://learn.microsoft.com/en-us/windows/wsl/wsl-config): distinguish feature/distribution installation, reboot requirements, configuration, and post-install verification.
- [Python venv](https://docs.python.org/3.12/library/venv.html): create an isolated local environment rather than change global packages.

# Browser drone game

The current user-directed deliverable is a locally served browser game with the same supplied ZIP drone as its default aircraft. The flight source of truth remains the existing C++20 core, compiled to WebAssembly. Three.js supplies rendering and its upstream FBX loader; the forest uses independent geometry. Unreal builds and Electric Dreams migration are suspended.

## Prepare and launch on Windows

Use Node.js **24.19.0** and PowerShell 7. If Node is not on PATH, invoke its existing executable by absolute path; the prepared local SDK also contains `build/tools/emsdk/node/24.19.0_64bit/node.exe`. Do not change global PATH. The verified Emscripten **6.0.12** SDK is under `build/tools/emsdk`, with local activation and compiler scratch files. A fresh checkout needs that pinned SDK prepared using the [official SDK instructions](https://emscripten.org/docs/getting_started/downloads.html); the build helper reports a missing SDK rather than installing one. Large installations retain the project's approval requirement.

Preparation uses pinned Three.js **0.186.1** with its MIT notice; it downloads the fixed official npm archive and verifies its integrity without running npm installation scripts. Core dependency notices also ship in the local prepared output.

From the repository root:

```powershell
node tools/web/prepare-vendor.mjs
pwsh -NoProfile -File tools/web/build-core.ps1
node tools/web/build-site.mjs
node tools/web/serve.mjs
```

Open `http://127.0.0.1:4173/` in the browser. The server exposes only the assembled `dist/web` output and binds to loopback. Source/configuration outside that output is not part of the served tree. The server is a local development/delivery helper, with no hosted publication or machine-wide installation.

Do not open `web/index.html` or `dist/web/index.html` using `file://`. Browsers block the module, WebAssembly and asset fetches used by this game. The direct-file page deliberately stops before loading the application and displays the same loopback-server command instead of a generic `Failed to fetch` error.

The frontend source is under `web/`; the build copies its known files, generated WebAssembly, approved vendor files, vehicle configuration and local drone assets into ignored `dist/web`. Preflight checks required files and types, including the pinned Three.js import dependencies, before touching the existing output. Do not run a generic static server from the repository root. Rebuild the output after source or configuration changes.

## Assets and delivery boundaries

The supplied `local-assets/drone` directory must contain `source/QuadDrone [mesh].fbx` and these files under `textures/`: `SpaceProbe_[diffuse].png`, `SpaceProbe_[emission-yellow].png`, `SpaceProbe_[gloss].png`, `SpaceProbe_[metal].png`, `SpaceProbe_[normals].png` and `Spaceprobe_[occlusion].png`. Preserve spelling and case. Keep originals unchanged. The frontend reports required asset failures visibly rather than silently substituting a generic aircraft. Loading and rendered-material verification are separate gates.

The drone archive's redistribution license is unverified. Its original files, derived textures and prepared bundle remain local and excluded from GitHub. The browser forest does not use Electric Dreams: the [official Fab listing](https://www.fab.com/listings/d79688f5-29be-4fb2-a650-2d4a813f5306) labels it UE-only. See [ASSETS.md](ASSETS.md) for provenance and restrictions.

The model is cosmetic: the reference quadrotor uses existing documented engineering estimates. Simplified flat-ground contact and graphical forest scenery do not imply detailed obstacle/terrain collision fidelity. Battery, GPS, sensors and real-aircraft calibration are not claimed. Browser-specific gameplay is distinct from the native telemetry and replay tools.

## Play and sessions

Choose a training scenario, then start. First-person view is the default. Click the rendered scene to capture the pointer; mouse movement changes the first-person look direction and Esc releases it. W/S move forward/backward; A/D move sideways; Q/E turn; Space/Ctrl change target altitude. F toggles motors, C changes camera, P pauses, and R resets. Direction release levels the attitude; momentum remains, so opposite input brakes. Leaving the tab pauses the simulation.

Standard-mapped gamepads use left stick for pitch/roll, right-stick horizontal axis for yaw, right/left triggers for climb/descent, A for motor toggle, Y for camera mode, and Menu/Home for pause. Any active manual axis temporarily takes priority over automatic-mission guidance, so a right-stick yaw command cannot be cancelled by the route controller; release the stick to return that axis to the route controller. The application polls `navigator.getGamepads()` once per rendered frame and selects the controller that is currently active, falling back safely when a disconnect event is missed. Standard mapping is preferred; devices exposed by a browser without that label use the common four-axis/two-trigger compatibility layout, including an axis-3 right-stick-horizontal fallback, instead of being silently disabled. Browser mapping and physical-device results vary, so supported hardware must be verified separately.

SIM-001–SIM-010 provide executable mission routes, deterministic disturbance schedules, automatic mission guidance, subsystem state, evaluation metrics, completion gates and reports. SIM-006–SIM-010 add synthetic safety-procedure scenarios with timestamped audit records; SIM-006, 009 and 010 need the local operator's visible `V`/「核准起飛」 acknowledgement. SIM-001 is backed by the current quadrotor physics and live wind environment. All other platform labels are scaled mission-system proxies; their RF/GNSS/payload/energy/thermal/safety metrics are abstract training metrics, not calibrated platform physics. All weapon-related interfaces are deny-by-default and negatively tested; targeting, recognition, terminal guidance, release, impact and harm logic are absent. See [SCENARIOS.md](SCENARIOS.md).

The settings dialog controls sensitivity, quality and metric/imperial display units, saved in browser local storage when available. Physics keeps the existing 0.004 s fixed timestep. The forest is visual scenery without tree or terrain collision physics.

Pause to download a recorded-state JSON or CSV. Recordings stay in the tab until the user downloads them; the recording cap is 18,000 frames. Opening a JSON recording displays validated recorded states, not deterministic physics re-execution. Export is disabled during imported playback, preventing a previous flight's recording from being exported under the imported session. The native/WASM comparison test below separately re-executes native recorded inputs against the C++ WebAssembly core.

## Verification

Record actual compile/runtime output and browser observations in [PROJECT_STATUS.md](PROJECT_STATUS.md). Required checks cover finite simulation state and control input; visible model and materials; start, flight, pause/reset and gameplay; responsive layout; missing resources; and static-server path, method and MIME boundaries. Native regression and hosted core CI remain separate evidence. Cross-browser, other-machine and real gamepad results must not be inferred from a Windows browser session.

The current Windows WebAssembly compile/runtime and native regression passed. Chrome and the in-app browser rendered the supplied red/white drone with six maps. Basic flight, pause/reset, held-P pause, camera and imperial-unit display passed. The in-app browser's 375×812 menu and live HUD fit without horizontal overflow. Actual JSON/CSV downloads and file-chooser import passed; imported playback disabled both exports, and malformed `{}` JSON was rejected visibly. Isolated missing-FBX and missing-WASM browser fixtures displayed explicit failure messages and kept Start disabled. Final HTTP, bundle-preflight, gameplay and WASM-equivalence reruns passed. Full six-ring completion, physical touch hardware, other browser engines and other machines remain NOT_TESTED. Historical Unreal results do not establish web acceptance.

Run focused core, gameplay, bundle preflight and HTTP checks from the repository root after preparation:

```powershell
node tests/wasm_smoke.mjs
node tests/mission_wasm.mjs
node --test web/game-logic.test.mjs web/mission-engine.test.mjs
node tools/web/build-site.test.mjs
node tools/web/server.test.mjs
```

To repeat native trajectory equivalence, pass an existing native `inputs.jsonl` path as the first argument to `tests/wasm_smoke.mjs`. The captured 5,000-step run had maximum state error `1.4094628242311558e-15`, below `1e-9`. Logs are `reports/web-build.log`, `reports/web-wasm-smoke.log` and `reports/web-native-regression.log`; generated artifacts are ignored. The restrictive agent sandbox can report `realpath` EPERM during HTTP checks; normal process permissions passed those checks without weakening path containment.

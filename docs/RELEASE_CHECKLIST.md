# Release checklist

## Active local browser delivery

The user replaced the desktop delivery target with a browser game. The earlier desktop checklist below is suspended and retained as historical evidence. Public hosting and distribution of the supplied assets are outside the current local delivery.

- [x] Build the existing C++20 core as WebAssembly and capture its executed validation: Emscripten 6.0.12, runtime smoke and 5,000-step native comparison PASS.
- [x] Load the same ZIP drone and all six material maps; inspect red/white rendered surfaces. Missing FBX/WASM displays explicit failure and leaves Start disabled.
- [x] Exercise browser flight controls, pause/reset, camera and imperial units; check 375×812 menu/live HUD without horizontal overflow.
- [x] Download JSON/CSV, import the downloaded state recording and verify playback export is disabled.
- [x] Reject malformed `{}` replay JSON visibly.
- [ ] Complete all six rings through actual interactive flight. Narrow desktop-browser layout does not establish physical touch-device acceptance.
- [x] Verify local server traversal rejection, supported methods, missing files and JavaScript/WebAssembly MIME types; expose only prepared `dist/web` on loopback.
- [x] Preserve pinned dependency licenses in prepared output; eight core files and Three.js LICENSE match SHA256. Ignore checks cover user assets/generated bundles.
- [x] Run affected native regressions: Windows 6/6 CTests PASS in `reports/web-native-regression.log`.
- [x] Browser-source commit `9ac524a9be891a6fc26cfc71d61ee55442745763` passed Windows and macOS native core CI in [run 38550487896](https://github.com/timmyc07/drone_emulator/actions/runs/38550487896); browser acceptance remains separately captured locally.
- [x] Record actual tool versions/evidence in [PROJECT_STATUS.md](PROJECT_STATUS.md), and commit/push browser source and documentation.
- [ ] Capture exact browser build numbers; current observations identify Chrome and the Codex in-app browser only.
- [x] Implement and unit-test SIM-001–SIM-005 mission state, events, metrics, completion/failure and versioned reports; complete all five against the C++/WASM core and SIM-001 end-to-end in the actual browser.
- [ ] Replace SIM-002–SIM-005 mission-system proxies with validated platform-specific flight/environment/RF models before claiming authentic fixed-wing, MALE or HALE simulation.
- [ ] Repair or update the local MSVC installation/tool pairing that currently returns C1902, then rerun the affected native CTests; this session's WebAssembly build is PASS but native rebuild is BLOCKED.

Commands and limitations are in [WEB_GAME.md](WEB_GAME.md). Unchecked items are not PASS.

## Historical desktop release checklist — retired

Version 0.2.0 is an implementation under acceptance, not a completed P7 release. A build or automatic smoke test alone does not establish a fully usable packaged simulator.

- [x] Local Windows C++20, CTest, Python and isolated Unreal fixture verified.
- [x] P1 Windows physics/configuration tests and 30-second hover/input-replay/takeoff-land scenarios passed.
- [x] Strict invalid configuration/replay cases and telemetry SHA256/row counts verified.
- [x] Product Unreal Editor build, adapter/lifecycle automation and DX12 runtime smoke passed.
- [x] Local user-provided drone mesh and six textures imported.
- [ ] Complete Machine A Python 3.12 and merged-source validation.
- [x] Hosted core CI for product commit `7f21881c1b8adbe252f74793250bbb262b1df49b`: Windows 6/6 and macOS ARM64 5/5 CTests, both Python 3.12.10 smoke checks PASS in [run 38542931069](https://github.com/timmyc07/drone_emulator/actions/runs/38542931069).
- [ ] Complete interactive aircraft/scene selection, flight/landing, pause/reset, settings persistence, save/playback and normal exit.
- [ ] Verify real gamepad hardware/input protocol; keep untested device support labeled.
- [ ] Complete Electric Dreams download/import/cooking and actual frame/memory measurements.
- [x] Produce and launch the first standalone Windows package without Editor; historical execution details remain in PROJECT_STATUS.
- [ ] Rebuild/retest after the failed drone material compilation and observed lighting, dropdown and menu-status repairs; complete visual and full interactive regression.
- [ ] Verify packaged configuration, assets, runtime prerequisites and third-party license notices.
- [ ] Verify writable sessions, missing/corrupt resources, failed writes and recovery on a clean target environment.
- [ ] Verify asset redistribution terms before external binary/content publication.
- [ ] Provide artifact version/checksums, supported requirements and upgrade instructions.
- [ ] Implement and verify the P4–P6 capabilities before declaring the full MASTER_PLAN product complete.

Current evidence is in [PROJECT_STATUS.md](PROJECT_STATUS.md). The cited hosted run verifies only the portable core and does not establish browser acceptance. Subsequent source changes require fresh CI evidence.

# Release checklist

Version 0.2.0 is an implementation under acceptance, not a completed P7 release. A build or automatic smoke test alone does not establish a fully usable packaged simulator.

- [x] Local Windows C++20, CTest, Python and isolated Unreal fixture verified.
- [x] P1 Windows physics/configuration tests and 30-second hover/input-replay/takeoff-land scenarios passed.
- [x] Strict invalid configuration/replay cases and telemetry SHA256/row counts verified.
- [x] Product Unreal Editor build, adapter/lifecycle automation and DX12 runtime smoke passed.
- [x] Local user-provided drone mesh and six textures imported.
- [ ] Complete Machine A Python 3.12 and merged-source validation.
- [ ] Record successful hosted CI for the final source.
- [ ] Complete interactive aircraft/scene selection, flight/landing, pause/reset, settings persistence, save/playback and normal exit.
- [ ] Verify real gamepad hardware/input protocol; keep untested device support labeled.
- [ ] Complete Electric Dreams download/import/cooking and actual frame/memory measurements.
- [ ] Produce and launch the standalone Windows package without Editor.
- [ ] Verify packaged configuration, assets, runtime prerequisites and third-party license notices.
- [ ] Verify writable sessions, missing/corrupt resources, failed writes and recovery on a clean target environment.
- [ ] Verify asset redistribution terms before external binary/content publication.
- [ ] Provide artifact version/checksums, supported requirements and upgrade instructions.
- [ ] Implement and verify the P4–P6 capabilities before declaring the full MASTER_PLAN product complete.

Current evidence is in [PROJECT_STATUS.md](PROJECT_STATUS.md). The historical P0 macOS CI success does not verify the newer physics/core changes.

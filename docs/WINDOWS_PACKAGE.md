# Historical local Windows prototype — suspended

The user replaced Unreal delivery with a local browser game on 2026-10-14. This page preserves earlier package evidence and reproduction instructions only. Further Unreal builds, repair validation, packaging and Electric Dreams migration are suspended. Use [WEB_GAME.md](WEB_GAME.md) for the active deliverable; these desktop results do not establish browser acceptance. Preserved Unreal source repairs have not been rebuilt or validated after the pivot.

The first verified 0.2.0 package is `dist/UAVSim-Windows-20261014-173017/Windows`. Open `UAVSim.exe` in that directory; keep the complete directory together. Unreal Editor is not needed to launch it. This package was tested on the Windows 11 / RTX 5070 development host; prerequisites and recovery on a clean machine remain unverified.

Choose **Start simulation**, then click the world to use the keyboard. W/S tilt forward/back, A/D tilt left/right, Q/E turn, Space/Left Ctrl climb/descend. F arms/disarms, M changes flight mode, P pauses/resumes, R resets and Esc opens the menu. The initial altitude-hold target is 1 m. Use **Save session**, then **Replay latest saved session** to view recorded states. **Apply and save settings** applies the selected display units/resolution and other settings; **Exit application** closes the simulator. Sessions are written under `UAVSim/Saved/Sessions` in this development package.

The first package has a **visual failure**: the drone material failed to compile because its OneMinus node lacked an input, so Unreal used a white fallback material. It also contains lighting-overlay, dropdown-contrast and stale menu-arming display defects. Repairs are in progress and are not present in the binaries identified below. Full interactive acceptance, visual regression, actual gamepad hardware and Electric Dreams integration remain pending. Physics currently uses simplified flat-ground contact.

This package and its assets remain local for the authorized evaluation. The supplied drone's redistribution license is unverified; do not publish the package or raw assets. Source dependency notices are included under `ThirdPartyNotices`. See [ASSETS.md](ASSETS.md) for the separate asset restrictions.

## Verified first-package evidence

- `reports/product/package.log`: BuildCookRun 175.18 s, `BUILD SUCCESSFUL`, exit 0; lines 327–328 nevertheless report the drone material compile failure. A successful packaging process does not establish visual correctness.
- `reports/product/unreal-20261014-093336-4348265/validation.json`: automation, Editor game runtime and packaged runtime PASS. The packaged test launched the actual game executable under `UAVSim/Binaries/Win64`; build was skipped in this validation invocation.
- Archive contents: 59 files, 997,739,689 bytes, excluding runtime-generated `Saved` files. Eight third-party notice/provenance files matched their source SHA256 values. Vehicle JSON SHA256: `2fff42c10efba4dfb7a52be4e1f7644fa97dee3949789501f00353fe8cc3f0dd`.
- Actual game executable `UAVSim/Binaries/Win64/UAVSim.exe` SHA256: `1258FBE865D8EFDE1FAA290E0EF21DD14643C82232092B98B1B38D490170B344`.
- Launcher `UAVSim.exe` SHA256: `B6A7E469F9377DCB2663ED2AEE9B8C7109C7324BD8CB65ECA010937840ED783F`.

Visible testing exercised start, approximately 1 m hover for 33 seconds, pause, save/playback, menu/Esc, feet units, 1920×1080 apply and normal exit. These observations coexist with the defects above; they do not complete P3 or P7. A rebuilt package requires its own checksum and acceptance evidence.

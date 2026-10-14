# Known issues

## Active browser delivery

- WebAssembly build/runtime and the current Windows native regression passed. Chrome/in-app browser model rendering, basic controls, 375×812 layout, JSON/CSV downloads, imported playback and missing-FBX/WASM failure handling passed. Full six-ring interactive completion, physical touch devices, other browser engines and other machines remain NOT_TESTED.
- A restricted agent sandbox can reject the static server's filesystem `realpath` checks with EPERM. The same server checks passed with normal process permissions. Retain the environment failure; do not weaken the served-directory boundary to bypass it.
- The user-supplied FBX and six textures are the required default model, but redistribution permission remains unverified. They stay local; public hosting and committed assets are excluded.
- Electric Dreams is UE-only and cannot be converted into this Three.js game. Its integration is suspended; the forest uses independently created geometry.
- The original reference quadrotor has estimated parameters and simplified flat-ground contact. The visual model and forest do not establish real-aircraft fidelity, detailed tree/terrain physics or sensor simulation.
- Actual gamepad hardware and browser operation on other machines require separate evidence; neither is inferred from keyboard or local Windows tests.

## Historical infrastructure and desktop issues

- MSVC compiler probing can fail with C1902 under the restricted agent sandbox. Preserve diagnostics and rerun the project-local build outside the sandbox; do not replace a working compiler based on this failure alone.
- WSL VirtualMachinePlatform is pending a separately approved restart. Ubuntu is not initialized and resource limits are not yet applied.
- Machine A's historical Python 3.9.6 environment does not meet the required 3.12 target; updated source/scripts need independent verification there.
- Product Editor automation, DX12 game runtime and the first standalone packaged runtime passed, but first-package visual acceptance FAILED: a missing OneMinus input prevented the drone material compiling and produced a white fallback. Visible testing also found a lighting-rebuild overlay, unreadable dropdown rows and stale menu arming status. Repairs, visual/full interactive regression and clean-machine release acceptance remain pending. The first package does not include these repairs.
- Electric Dreams acquisition/integration and performance acceptance remain incomplete. The current physics contacts a flat plane and does not model terrain or obstacle collisions.
- Gamepad analog bindings exist, but no actual gamepad device has been verified.
- User-provided mesh/textures are local-only while redistribution licensing is unverified.
- P4–P7 features remain planned; this is not a complete multi-aircraft/SITL simulator release.

Resolved: the historical Windows CI developer-script quoting failure now passes its path regression and the full hosted job. Product commit `7f21881c1b8adbe252f74793250bbb262b1df49b` passed Windows 6/6 and macOS ARM64 5/5 CTests in [run 38542931069](https://github.com/timmyc07/drone_emulator/actions/runs/38542931069).

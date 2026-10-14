# Known issues

- MSVC compiler probing can fail with C1902 under the restricted agent sandbox. Preserve diagnostics and rerun the project-local build outside the sandbox; do not replace a working compiler based on this failure alone.
- WSL VirtualMachinePlatform is pending a separately approved restart. Ubuntu is not initialized and resource limits are not yet applied.
- Machine A's historical Python 3.9.6 environment does not meet the required 3.12 target; updated source/scripts need independent verification there.
- Product Editor automation and a real DX12 game runtime passed. Full manual UI/flight/settings acceptance and packaged runtime remain separate pending gates.
- Hosted P0 Windows CI failed on a developer-script path containing spaces. A process-local quoting helper and path regression now pass locally; final hosted CI must confirm the fix. The P0 macOS job passed.
- Electric Dreams acquisition/integration and performance acceptance remain incomplete. The current physics contacts a flat plane and does not model terrain or obstacle collisions.
- Gamepad analog bindings exist, but no actual gamepad device has been verified.
- User-provided mesh/textures are local-only while redistribution licensing is unverified.
- P4–P7 features remain planned; this is not a complete multi-aircraft/SITL simulator release.

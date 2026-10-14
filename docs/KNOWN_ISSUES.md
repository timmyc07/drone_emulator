# Known issues

- MSVC compiler probing can fail with C1902 under the restricted agent sandbox. Preserve diagnostics and rerun the project-local build outside the sandbox; do not replace a working compiler based on this failure alone.
- WSL VirtualMachinePlatform is pending a separately approved restart. Ubuntu is not initialized and resource limits are not yet applied.
- Machine A's historical Python 3.9.6 environment does not meet the required 3.12 target; updated source/scripts need independent verification there.
- Automated graphical Editor startup and DX12 execution passed; manual inspection of the displayed scene remains untested.

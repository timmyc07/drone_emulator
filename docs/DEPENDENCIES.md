# Dependencies

## Machine A (verified in this session)

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

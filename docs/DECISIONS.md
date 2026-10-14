# Decisions

- The core remains a headless C++20 CMake target so Unreal is optional for P0 core CI.
- CMake and Ninja were installed only inside the project-local `.venv`; Homebrew and full Xcode were not installed without explicit approval.
- The Mac system Python is 3.9.6, so Python 3.12 is recorded as a missing target dependency rather than misreported as available.
- Windows and Unreal statuses remain NOT_TESTED/DEFERRED because this session cannot access Machine B.
- GitHub Actions uses the explicit `macos-14` and `windows-2022` hosted labels; workflow success is not claimed until it runs.

# Universal UAV Simulator

Universal UAV Simulator (UAVSim) is a cross-platform UAV simulation product. P0 establishes a headless C++20 core, reproducible CMake/CTest builds, environment audit tooling, and platform-specific continuation instructions.

## Local core build

```sh
cmake --preset macos-arm64-debug
cmake --build --preset macos-arm64-debug
ctest --preset macos-arm64-debug --output-on-failure
```

See `docs/PROJECT_STATUS.md` for verified host evidence and remaining Windows/Unreal work.

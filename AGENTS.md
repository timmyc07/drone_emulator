# UAVSim Repository Rules

- Build the portable core as cross-platform C++20 with CMake and CTest.
- Keep the headless simulation core independent of Unreal Engine.
- Prioritize shippable software and operational evidence; do not add academic thesis deliverables.
- Every build and test claim requires captured command evidence.
- Develop with platform awareness: macOS validation runs on macOS and Windows validation runs on Windows.
- Never claim an untested platform, dependency, CI workflow, Unreal project, or benchmark as successful.
- Manage dependencies conservatively; do not replace existing toolchains or modify machine-wide settings without explicit approval.
- Do not use destructive cleanup to resolve setup problems.
- Update `docs/PROJECT_STATUS.md` after each P0 task or session.

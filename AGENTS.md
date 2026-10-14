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

## Additional user requirements and bootstrap protocol


## Scope and architecture

- Build a maintainable UAV simulation software product; do not create academic thesis deliverables.
- Use cross-platform C++20. Keep the headless core independent of Unreal Engine.
- P0 prepares development infrastructure only; do not implement flight dynamics.
- Prefer existing code, standard-library/platform features, and installed dependencies. Keep changes minimal and complete.

## Before every implementation

1. Read relevant code, documentation, deployment configuration, and this file.
2. Perform a deep GitHub search for relevant open-source implementations. Reuse suitable existing technology rather than redesigning it; record sources and preserve applicable licenses.
3. Check current official or primary technical documentation against the actual installed versions.
4. Complete three reviews before changing files: official compatibility; code/configuration and failure risks; conventions/documentation/maintenance. Default to three parallel agents when available; otherwise perform three independent review rounds.
5. Apply only changes with confidence above 90%. Record lower-confidence risks, blockers, and alternatives before implementation.

## Safety and evidence

- Detect the host OS before platform-specific commands. Never claim another machine was tested remotely without evidence.
- Inspect existing installations/settings first. Do not overwrite configuration; back up existing user configuration before an approved change.
- Obtain approval for administrator operations, large installations, Visual Studio workload changes, WSL distributions/features, machine-wide environment changes, GPU/security changes, environment removal, and remote publication/deletion.
- Obtain separate approval before restarting the computer or shutting down an active WSL environment.
- Use project-local venvs and build directories; do not modify global PATH or Git identity without approval.
- Never log credentials, serial numbers, or unrelated personal paths. Ignore private machine reports, caches, venvs, and Unreal generated files.
- Record critical workflow and error handling through the existing logger; when none exists in bootstrap scripts, retain explicit diagnostics and failure logs.
- Frontend interfaces must be responsive, without fixed-size layouts.

## Completion

- Run necessary verification and capture actual command results. Distinguish PASS, FAIL, BLOCKED, DEFERRED, and NOT_TESTED; absence of testing is never PASS.
- Update `docs/PROJECT_STATUS.md`, `docs/P0_PLAN.md`, `docs/DEPENDENCIES.md`, and `docs/DECISIONS.md` as applicable after each task.
- Commit and push completed changes to the configured GitHub remote. Do not overwrite an existing remote or invent Git identity. Record a blocker when authentication, identity, or remote access prevents synchronization.
- Do not declare P0 complete until mandatory acceptance gates pass on the respective hosts. Unreal acceptance requires a real compiler/editor/level test; CI acceptance requires an actual successful run.

## Ponytail default

At the start of each conversation read `C:/Users/Yumai/.codex/skills/ponytail/SKILL.md` and apply full mode unless the user changes or disables it. It does not waive GitHub research, three reviews, documentation, verification, or commit/push requirements. If unavailable, report that fact rather than claiming it was loaded.

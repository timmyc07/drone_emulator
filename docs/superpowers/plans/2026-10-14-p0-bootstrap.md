# Universal UAV Simulator P0 Bootstrap Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Bootstrap and verify the locally executable P0.0, P0.1, P0.5, and P0.6 work for the Apple Silicon host, while documenting Windows work that has not run.

**Architecture:** Keep a headless C++20 core independent of Unreal. Use CMake presets and CTest for the portable build, shell/PowerShell wrappers for platform validation, and JSON/Markdown environment reports generated from read-only probes.

**Tech Stack:** C++20, CMake, CTest, Ninja when available, Apple Clang/MSVC, Python 3, POSIX shell, PowerShell, GitHub Actions.

**Spec:** `/Users/reikous/.codex/attachments/89fa4b4a-b96f-4600-8655-c4af2c935c70/貼上的文字.txt`

## Global Constraints

- Do not claim Windows or Unreal verification from this Mac session.
- Do not install Homebrew, Unreal, Visual Studio, WSL, or system-wide software without approval.
- Keep the core headless and independent of Unreal.
- Record actual versions and use PASS, FAIL, BLOCKED, DEFERRED, and NOT_TESTED accurately.
- Keep builds, virtual environments, credentials, and machine-specific reports out of Git.

### Task 1: Repository and audit scaffolding

**Files:** `AGENTS.md`, `README.md`, `.gitignore`, `.gitattributes`, audit scripts, report directories, P0 docs.

- [ ] Add durable repository rules and project status documents.
- [ ] Add read-only macOS and Windows audit scripts producing structured JSON.
- [ ] Run the macOS audit and save the actual report.

### Task 2: Minimal C++20 and CTest infrastructure

**Files:** `CMakeLists.txt`, `CMakePresets.json`, `include/uavsim/version.hpp`, `src/uavsim_info.cpp`, `tests/smoke_test.cpp`.

- [ ] Define a C++20 executable and CTest smoke test.
- [ ] Configure Debug and Release presets using a project-local `build/` directory.
- [ ] Build and run CTest with the available local CMake toolchain; record blockers if CMake is unavailable.

### Task 3: Cross-platform validation and CI

**Files:** `tools/bootstrap/build_macos.sh`, `tools/bootstrap/build_windows.ps1`, `tools/validation/validate_macos.sh`, `tools/validation/validate_windows.ps1`, `.github/workflows/core-ci.yml`.

- [ ] Add fail-fast validation wrappers that report command results.
- [ ] Add hosted macOS and Windows CI definitions without claiming they have run.
- [ ] Document runner and dependency limitations.

### Task 4: Reports, dependencies, and repository initialization

**Files:** `docs/DEPENDENCIES.md`, `docs/DECISIONS.md`, `docs/P0_PLAN.md`, `docs/ROADMAP.md`, `docs/PROJECT_STATUS.md`, performance reports.

- [ ] Record actual Mac evidence and explicit Windows/Unreal status.
- [ ] Create a sanitized Windows continuation checklist.
- [ ] Initialize Git, commit local work, configure the requested remote, and push only after local verification.

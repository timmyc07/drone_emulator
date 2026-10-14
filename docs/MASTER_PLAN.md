# CODEX MASTER PROMPT V2.0
# Universal UAV Simulator — Product Development

## Current target override — 2026-10-14

The user's latest direction is a browser drone game, without Unreal, using the same user-supplied ZIP drone as the default aircraft appearance. This direction supersedes the Unreal visualization/integration requirements, Windows desktop/package delivery target and Electric Dreams work below. The original specification is retained as historical product context; its remaining Unreal tasks are suspended, not new browser acceptance gates.

The active architecture retains the existing cross-platform C++20 flight core and compiles it to WebAssembly with a project-local Emscripten toolchain. Three.js supplies browser rendering and its existing FBX loader. Physics remains the single source of aircraft movement; graphical gameplay uses an independently created forest, not converted UE-only Electric Dreams content. The visual drone does not change the reference quadrotor's estimated physical parameters.

Current delivery is a locally served browser build with responsive Chinese controls/HUD, the supplied model/materials, actual flight controls, pause/reset and visible loading/failure states. Use [WEB_GAME.md](WEB_GAME.md) for commands and boundaries and [PROJECT_STATUS.md](PROJECT_STATUS.md) for executed evidence. Browser, WebAssembly and material acceptance require their own tests. The user authorized removal of the Unreal application and desktop tooling; historical reports and local user assets remain. Retain existing engineering, license, review and verification requirements. Hosted publication and redistribution of the unverified-license drone assets are not part of this delivery.

## Historical desktop specification

> Execution status (2026-10-14): this file remains the governing specification, not a completion claim. P1 has executable Windows acceptance evidence, and product commit `7f21881c1b8adbe252f74793250bbb262b1df49b` passed both Windows and macOS ARM64 core CI in [run 38542931069](https://github.com/timmyc07/drone_emulator/actions/runs/38542931069). Product Unreal build/automation/runtime tests and the first standalone Windows package/runtime have passed locally. Interactive P2/P3 repairs/regression, clean-machine release acceptance, requested Electric Dreams integration and P4–P7 remain incomplete. See [PROJECT_STATUS.md](PROJECT_STATUS.md) for exact evidence and [ROADMAP.md](ROADMAP.md) for remaining gates. Unreal 5.8.3 is the selected installed engine.

## 0. Your Mission and Authority

You are the principal software architect, simulation engineer, senior C++ developer, Unreal Engine developer, QA engineer, and technical project manager for Universal UAV Simulator (UAVSim).

You are responsible for implementing and delivering a real, usable, extensible engineering-grade drone simulator.

This is a SOFTWARE PRODUCT DEVELOPMENT PROJECT.

It is NOT an academic thesis project.

Do not prioritize:
- Research paper writing
- Academic novelty
- Publication-oriented experiments
- Thesis-specific deliverables
- Academic formatting

Instead prioritize:
1. Working software
2. Physically meaningful flight simulation
3. Reliable user controls
4. Usable desktop interface
5. Extensible aircraft support
6. Software stability
7. Automated verification
8. Performance
9. Maintainability
10. Windows distribution

Engineering validation is mandatory for software quality, not for academic publication.

Do not stop after writing a plan. Implement, compile, test, fix, and document working features.

You may make routine architecture and implementation decisions autonomously.

Do not bypass security controls, execute untrusted scripts without review, delete user data, purchase services, publish repositories, or modify system-wide settings without appropriate authorization.

Do not claim that a feature works without executing the relevant verification.

## 1. Product Definition

Product name:
Universal UAV Simulator

Short name:
UAVSim

Primary product target:
Windows 11 x64 desktop application

Secondary development target:
Apple Silicon macOS ARM64

Primary user experience:
A graphical drone simulator in which users can select an aircraft, select an environment, control the aircraft, observe its instruments, and analyze flight behavior.

The final product must not require users to interact with a command-line interface for ordinary flight simulation.

The simulator must support future expansion into:
- Quadrotors
- Hexacopters
- Octocopters
- FPV multirotors
- Fixed-wing UAVs
- VTOL UAVs
- Unmanned helicopters
- Experimental aerial configurations
- Publicly documented civilian UAV models
- Publicly documented, non-weaponized approximations of military UAV flight characteristics

Never claim a public-data aircraft approximation is equivalent to a validated real aircraft.

Do not implement weapon deployment, engagement, targeting, or strike-planning systems.

## 2. Available Development Hardware

Development Machine A:

Apple Silicon MacBook Pro
24GB unified memory
macOS ARM64

Primary responsibilities:
- C++ simulation core
- Physics development
- Unit testing
- Headless simulator
- Python utilities
- Configuration tools
- Documentation
- Cross-platform validation

Development Machine B:

Windows desktop
NVIDIA GeForce RTX 5070
32GB system RAM
Windows x64

Primary responsibilities:
- Unreal Engine 5
- Windows application integration
- 3D rendering
- Input devices
- User interface
- Performance profiling
- SITL integration when ready
- Windows application packaging

The exact CPU models and available disk capacities must be detected rather than assumed.

Resource constraints:
- Keep initial Windows development compatible with 32GB RAM.
- Prefer Unreal development at 1080p and medium rendering quality.
- Use lightweight test scenes initially.
- Avoid unnecessary high-resolution assets.
- Keep Unreal and Headless simulation independently executable.
- Default future WSL2 memory limit to 8GB.
- Prefer CPU-based simulation before investigating GPU acceleration.
- Limit parallel build and simulation workers according to actual memory measurements.

Do not assume that the development agent has access to both machines simultaneously.

If running on one host, verify that host and record testing required on the other.

## 3. Technology Architecture

Required:
- C++20
- CMake
- CTest
- GoogleTest or equivalent
- JSON vehicle and scenario configuration
- CSV telemetry
- Git
- Cross-platform continuous integration

Visualization:
- Unreal Engine 5
- C++ Unreal plugin/module
- Blueprint for appropriate UI and content workflows
- UMG for interface implementation

Preferred future integrations:
- JSBSim
- PX4 SITL
- ArduPilot SITL

Evaluate Project AirSim before reproducing functionality already available through a compatible, appropriately licensed implementation.

Dependency selection must be based on official documentation, license compatibility, host availability, and demonstrated builds.

Do not embed all subsystems into Unreal Engine.

Maintain a clean separation between:
- Flight simulation core
- Vehicle definitions
- Flight controllers
- Environment
- Rendering
- User interface
- Input
- Telemetry
- Asset management

Exactly one authoritative physics backend may update a given vehicle at a given simulation step.

## 4. Core Architecture

Implement the following logical components:

SimulationCore
- Simulation clock
- Fixed-step scheduler
- Vehicle lifecycle
- World state
- Deterministic stepping

Math
- Vectors
- Matrices
- Quaternions
- Coordinate transformations
- Numerical utilities

FlightDynamics
- 6-DOF rigid-body physics
- Translational dynamics
- Rotational dynamics
- External forces and torques
- Numerical integration

Propulsion
- Multirotor thrust
- Reaction torque
- Motor response
- Rotor saturation

Environment
- Gravity
- Air density
- Wind
- Simplified drag
- Ground interaction

FlightControl
- Motor commands
- Rate control
- Attitude control
- Altitude control
- Control allocation

VehicleRegistry
- Vehicle definitions
- Configuration validation
- Vehicle category
- Simulation backend selection
- Asset references
- Parameter provenance

Telemetry
- Simulation state logging
- CSV export
- Session metadata
- Flight replay data

UnrealAdapter
- State snapshots
- Input translation
- Unit conversion
- Coordinate conversion
- Rendering interpolation
- Thread-safe data exchange

UI
- Main menu
- Vehicle selection
- Environment selection
- Simulation controls
- Telemetry HUD
- Settings
- Error notifications

Avoid unnecessary frameworks and premature abstractions.

## 5. Coordinate Systems

Simulation world:
Right-handed ENU.

X = East
Y = North
Z = Up

Aircraft body:
Right-handed FLU.

X = Forward
Y = Left
Z = Up

All physics calculations must use SI units.

Use meters, kilograms, seconds, radians, Newtons, and Newton-meters.

Represent attitude using normalized quaternions.

Explicitly document quaternion multiplication order and body-to-world rotation conventions.

Coordinate transformations between the simulation and Unreal Engine must be centralized inside UnrealAdapter.

Add automated transformation tests.

No implicit conversions are permitted.

## 6. Flight Dynamics

Implement a six-degree-of-freedom rigid-body simulation.

State:
- Position
- Velocity
- Orientation quaternion
- Body angular velocity

Vehicle properties:
- Mass
- Center of mass
- Full inertia tensor
- External force
- External torque

Required equations:

Position:
dp/dt = v

Translation:
dv/dt = R(q)F_body/m + F_world/m + g

Rotation:
dω/dt = I^-1(τ - ω × Iω)

Quaternion integration:
Implement consistently with the selected convention.

Required behavior:
- Strict parameter validation
- Finite numerical state checks
- Positive-definite inertia validation
- Configurable fixed timestep
- Default dt = 0.004 seconds
- Baseline RK4 integrator
- Quaternion normalization
- Numerical convergence tests

The physics state must never be replaced by precomputed animations or direct position manipulation to simulate flight.

## 7. Initial Aircraft

Implement:

Generic Quadrotor X-01

Configuration:
X-layout quadrotor

Initial reference mass:
1.5 kg

Include:
- Mass
- Inertia tensor
- Center of mass
- Four rotor positions
- Rotor spin directions
- Rotor thrust coefficients
- Rotor reaction torque coefficients
- Maximum rotor speeds
- Motor response time constants
- Aerodynamic drag parameters
- Control limits

Use documented engineering assumptions for unknown values.

Motor model:

T_i = kT * omega_i^2

Q_i = spin_i * kQ * omega_i^2

Rotor moment:

tau_i = r_i cross F_i

Implement motor lag, nonnegative rotor speeds, saturation, and internally consistent motor mixing.

Validate that the reference quadrotor has enough thrust for stable hovering.

All aircraft movement must result from the dynamics model.

## 8. Flight Controls

Implement:
- Motor arming and disarming
- Manual rate mode
- Stabilized attitude mode
- Basic altitude-hold mode
- Collective thrust input
- Roll, pitch, and yaw controls
- PID controller reset
- PID anti-windup
- Motor command saturation
- Flight controller status

Support:
- Keyboard
- Game controller
- Configurable control bindings

Future support:
- USB RC transmitters
- Joysticks
- Flight sticks

Do not claim hardware support without testing the actual input protocol and device.

Provide a default keyboard mapping and an in-application controls reference.

## 9. Environment

Initial version:
- Simple open test level
- Ground plane
- Gravity
- Configurable wind
- Basic atmosphere
- Simplified drag
- Simplified ground collision

Future:
- Terrain
- Cities
- Mountains
- Coastlines
- Airports
- Weather
- Turbulence
- Time of day
- Sensor visualization

Start with one minimal environment.

The development milestone must be driven by flight behavior, not decorative world complexity.

## 10. Product User Interface

Build an actual graphical workflow.

Main Menu:
- Start Simulation
- Select Aircraft
- Select Environment
- Settings
- Exit

Aircraft Selection:
- Aircraft name
- Category
- Preview
- Basic specifications
- Model fidelity level
- Available control modes

Environment Selection:
- Environment name
- Description
- Weather preset
- Starting location

In-Flight HUD:
- Altitude
- Airspeed / ground speed with clear labeling
- Heading
- Attitude
- Vertical speed
- Flight mode
- Simulation status
- Wind information
- Simulation elapsed time

Flight Session:
- Start
- Pause
- Resume
- Reset
- Exit to menu
- Save session
- Open replay

Settings:
- Graphics quality
- Resolution
- Input mappings
- Control sensitivity
- Units for display
- Audio if implemented

User interaction must not require modifying configuration files manually.

Use configuration files internally, but provide a graphical interface for basic supported settings.

Do not display fictional battery charge, GPS status, or sensor measurements as if they were physically simulated.

Mark unmodeled fields clearly or omit them.

## 11. Unreal Integration

Implement the Unreal front end after Headless Core validation.

Required:
- Unreal C++ plugin
- Simulation bridge
- One quadrotor actor
- One basic simulation level
- Camera
- Input handling
- Flight HUD
- Pause/reset
- Data exchange
- Interpolated rendering

Use the C++ flight dynamics core as the source of truth.

Do not let Unreal Chaos and the custom flight dynamics engine simultaneously control the same aircraft.

Support a fixed simulation timestep independent of frame rate.

Implement interpolation for visual smoothness.

All state sharing must be thread-safe.

If Unreal Engine is unavailable, report its integration status honestly and continue implementing independently testable components.

## 12. Telemetry and Flight Replay

Telemetry:
- Simulation timestamp
- Aircraft ID
- Position
- Velocity
- Attitude
- Angular rates
- Motor commands
- Rotor speeds
- Rotor thrust
- Flight mode
- Wind
- Scenario identifier
- Simulator version
- Configuration hash

Support CSV export and structured metadata.

Build replay with a clear distinction between:
1. Recorded state playback
2. Re-execution using recorded inputs

Do not call state playback physically deterministic replay.

For input replay, verify compatibility and compare state trajectories within documented tolerances.

Create a user-accessible flight session directory.

Avoid writing experimental output directly into version-controlled source directories.

## 13. Vehicle Configuration System

Create a schema-based vehicle registry.

Every vehicle must contain:
- Unique ID
- Name
- Category
- Configuration revision
- Physics backend
- Mass and inertia
- Propulsion description
- Control description
- 3D asset reference
- Public data sources, when applicable
- Assumptions
- Validation status

Separate:
- Officially documented parameters
- Measured parameters
- Engineering estimates
- Unknown parameters

Support loading multiple aircraft configurations without modifying or recompiling the C++ simulation core, when supported by the chosen backend.

Do not implement every aircraft category prematurely.

## 14. Testing and Acceptance

Required tests:

Mathematics:
- Vector operations
- Quaternion operations
- Coordinate transforms
- Rotation composition

Physics:
- Free fall
- Constant acceleration
- Forces and torques
- Inertia validation
- Numerical convergence

Propulsion:
- Rotor force
- Rotor torque
- Motor lag
- Saturation

Control:
- PID reset
- Anti-windup
- Hover equilibrium
- Attitude recovery
- Altitude hold

Runtime:
- Fixed timestep
- Reset
- Pause
- Resume
- Deterministic behavior
- No NaN or infinity

Unreal:
- Correct actor spawn
- Coordinate conversion
- Control responsiveness
- Pause/reset behavior
- Rendering independence
- Correct simulation clock

UI:
- Aircraft selection
- Environment selection
- Start and stop session
- Settings persistence
- Missing resource handling

Initial numerical acceptance targets:
- Free fall, 1 second: under 0.01m position error without drag
- Quaternion norm error: under 1e-6
- Steady-state hover thrust: within 0.5% of aircraft weight
- Ideal hover maximum altitude error: under 0.2m over 30 seconds when starting near equilibrium
- All required tests pass

Treat these as development acceptance criteria, not proof of real-aircraft certification.

For performance, establish and report actual measured baselines on the host hardware.

Do not invent FPS measurements or simulation performance results.

## 15. Development Stages

P0 — Infrastructure
- Repository audit
- Environment detection
- CMake
- Tests
- Git workflows
- CI
- Build scripts

P1 — Flight Dynamics
- Math
- 6-DOF
- Motors
- Environment
- Reference quadrotor
- Headless app
- Telemetry
- Tests

P2 — First Flyable Unreal Prototype
- UE project
- C++ bridge
- Generic quadrotor
- Simple scene
- Keyboard input
- Camera
- HUD
- Start/pause/reset

P3 — Usable Simulator
- Main menu
- Aircraft selection
- Environment selection
- Settings
- Gamepad support
- Flight modes
- Session recording
- Playback
- Windows packaging

P4 — Multiple Vehicle Classes
- Additional multirotor layouts
- Fixed-wing
- VTOL
- Unmanned rotorcraft
- Backend adapters

P5 — Advanced Simulation
- Sensor models
- Weather
- More realistic aerodynamic effects
- PX4 SITL
- ArduPilot SITL
- Additional environments

P6 — Vehicle Library
- Vehicle registry expansion
- Public reference configurations
- Civilian and industrial vehicles
- Non-weaponized military aircraft approximations
- Parameter confidence documentation
- Asset management

P7 — Product Release
- Installer or portable distribution
- Runtime dependency management
- Versioning
- Crash/error handling
- Upgrade documentation
- Release verification

Do not begin a later phase when the previous phase has critical failed tests.

Noncritical features may be deferred if recorded explicitly.

## 16. Codex Execution Protocol

At the beginning of every work session:

1. Inspect repository state.
2. Read README.md.
3. Read docs/ROADMAP.md.
4. Read docs/PROJECT_STATUS.md.
5. Read docs/DECISIONS.md.
6. Inspect open tasks and known failures.
7. Identify the earliest incomplete milestone.
8. Select a small, testable implementation task.

For each task:
1. Define acceptance criteria.
2. Implement source code.
3. Build affected targets.
4. Run tests.
5. Fix failures.
6. Inspect changes.
7. Update documentation.
8. Commit changes if authorized.
9. Record next task.

Do not mark an unfinished task complete.

Do not advance a milestone solely because files were created.

If tests fail, investigate the actual cause.

Never weaken test thresholds merely to obtain green test results.

If a task requires unavailable hardware or software, record the blocker and continue with independent tasks.

Use conservative, reversible technical decisions.

Preserve working code.

## 17. Required Project Tracking

Maintain:

docs/ROADMAP.md
- Full development roadmap
- Milestones
- Dependencies
- Acceptance criteria

docs/PROJECT_STATUS.md
- Current phase
- Implemented components
- Verified components
- Incomplete work
- Current blockers
- Next tasks

docs/DECISIONS.md
- Architecture decisions
- Alternatives considered
- Justification

docs/DEPENDENCIES.md
- Version locks
- Compatibility
- Licenses
- Official sources

docs/KNOWN_ISSUES.md
- Reproducible bugs
- Severity
- Workarounds

docs/RELEASE_CHECKLIST.md
- Packaging
- Smoke tests
- Resource requirements
- Release quality gates

Separate verified functionality from planned functionality.

## 18. Prohibited Shortcuts

Never:
- Fake aircraft flight using precomputed animations
- Directly teleport physics states to simulate stabilization
- Tie physics to rendering FPS
- Mix units or coordinate systems implicitly
- Claim real-world fidelity without evidence
- Invent real-world aircraft parameters
- Fabricate sensor measurements
- Ignore numerical errors
- Claim tests passed without running them
- Claim an Unreal build succeeded without building it
- Deliver only documentation when executable software is required
- Implement unused features merely to make the repository appear large
- Integrate unreviewed dependencies
- Expose credentials
- Change software licenses without permission
- Delete existing work without a recovery path

## 19. Phase Completion Gates

P0 complete when:
- Project structure exists
- Both supported platforms have documented build instructions
- Actual host build passes
- Tests execute
- CI configuration exists
- Cross-platform results are recorded

P1 complete when:
- Headless core compiles
- Quadrotor configuration loads
- Simulation advances correctly
- Required physics tests pass
- Hover scenario works
- Telemetry exports successfully

P2 complete when:
- Unreal opens the project
- Drone is visible
- Input affects actual flight dynamics
- HUD displays simulation state
- Pause/reset works
- Flight can be demonstrated

P3 complete when:
- A user can start the desktop application
- Select a supported aircraft and scene
- Fly and land
- Save and replay a session
- Use the settings interface
- Exit normally
- Run the packaged application without the Unreal Editor

Later phase gates must be specified before implementation begins.

## 20. Final Reporting

After every execution session, report:

- Current phase
- Tasks completed
- Files changed
- Build commands
- Tests executed
- Actual results
- Executable outputs
- Known issues
- Blockers
- Next tasks

Use explicit status categories:

IMPLEMENTED AND VERIFIED
IMPLEMENTED BUT UNVERIFIED
PLANNED
BLOCKED
FAILED

Never fabricate completion.

## 21. Start Work

Start with:
1. Repository inspection
2. Host environment detection
3. Compatible dependency selection
4. P0 implementation
5. Minimal cross-platform C++ executable
6. Automated build and test
7. Project status documentation

If the repository is empty, initialize it.

If useful code already exists, preserve and reuse it where appropriate.

Continue through the next feasible testable tasks within the available execution session.

Do not stop after writing an implementation proposal.

The ultimate deliverable is a usable Windows UAV simulator, backed by a cross-platform flight simulation core.

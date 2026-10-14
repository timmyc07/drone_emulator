# P1 headless acceptance

This is the acceptance procedure, not a declaration that every gate has passed. Captured results and current blockers belong in PROJECT_STATUS.

The portable core uses SI, ENU world, FLU body, Hamilton quaternions in `w,x,y,z` order rotating body vectors into world. The default fixed step is 0.004 seconds. One backend advances each vehicle. JSON vehicle schema version 1 requires exactly the documented keys in `configs/vehicles/generic_quadrotor_x01.json`; missing, duplicate, unknown, incorrectly typed and nonfinite fields fail. Mass/inertia/rotor/controller physical validation is also mandatory. IDs use lowercase ASCII letters, numbers, dash or underscore. Four rotors and the `uavsim-rk4` backend are the currently supported configuration family.

The reference parameters are engineering assumptions for a fictional generic vehicle. They are not measured, manufacturer-validated, or a real-aircraft certification. Provenance separates official values, measurements, engineering estimates and unknowns; assumptions and provenance are each required. `asset_reference` identifies rendering content separately from the headless physics.

Required numerical gates:

| Gate | Criterion |
| --- | --- |
| Free fall | 1-second position error below 0.01 m without drag |
| Orientation | Maximum quaternion norm error below 1e-6 |
| Hover thrust | Equilibrium total thrust within 0.5% of weight |
| Near-equilibrium hover | Maximum altitude error below 0.2 m over 30 s |
| RK4 | Numerical convergence for a nontrivial rotation/translation trajectory |
| Validation | Reject invalid, asymmetric or non-positive-definite inertia, nonfinite state and invalid rotor parameters |
| Controls | Motor lag/saturation, mixing signs, PID reset/anti-windup, attitude recovery and altitude hold |
| Runtime | Pause/resume/reset, fixed step and repeatable trajectories |

After building, from the repository root:

```text
uavsim_headless validate --vehicle configs/vehicles/generic_quadrotor_x01.json
uavsim_headless hover --vehicle configs/vehicles/generic_quadrotor_x01.json --duration 30 --output <new-session-directory>
uavsim_headless replay --vehicle configs/vehicles/generic_quadrotor_x01.json --session <recorded-session> --output <new-replay-directory>
uavsim_headless scenario --scenario takeoff-land --vehicle configs/vehicles/generic_quadrotor_x01.json --duration 30 --output <new-session-directory>
```

Use the executable in the selected platform build directory. On Windows, `pwsh -File tools/validation/validate_product.ps1` builds/tests and runs these scenarios, verifies CSV row count, and checks SHA256 independently with PowerShell. It writes timestamped evidence under ignored `reports/product`. Use `-SkipBuild` only after an actual successful build of the current source. A duration is rounded upward to a whole simulation step, between 0.004 s and one hour. Takeoff/landing requires at least 20 s. Existing output directories are rejected to preserve prior sessions.

Takeoff/landing acceptance requires an observed peak altitude of at least 1.8 m followed by ground contact with final speed below 0.2 m/s. Negative checks reject nonfinite duration, output overwrite, mismatched configuration hashes and altered replay trajectories. Scenario-specific metadata measurements are null when not applicable.

Each recording contains `telemetry.csv`, `inputs.jsonl`, `vehicle.json` and `metadata.json`. CSV rows include initial state and every executed step, aircraft ID, time, world position/velocity, quaternion, body angular rates, motor commands, rotor speeds/thrusts, mode, arming/ground state and wind. Metadata identifies version, scenario, coordinate convention, fixed step, initial state/speeds and SHA256 of the exact input vehicle bytes. Whitespace changes intentionally change this hash. Metadata is written only after telemetry files close successfully; incomplete sessions without metadata are not accepted as completed recordings.

Headless replay means recorded-input re-execution, not state playback. It requires matching simulator version, configuration hash, timestep and coordinate frames, then compares every recorded state with a 1e-9 maximum component-group norm tolerance. The replay file records the actual resulting trajectory. The tolerance is a regression criterion for compatible executions, not a promise of bitwise determinism across compilers, CPUs or future versions. UI state playback must be labeled separately.

JSON parsing reuses [nlohmann/json v3.12.0](https://github.com/nlohmann/json/tree/v3.12.0) and its [official parse API](https://json.nlohmann.me/api/basic_json/parse/). SHA256 reuses [PicoSHA2](https://github.com/okdshin/PicoSHA2), including its standard known-answer test; dependency versions/licenses are recorded in DEPENDENCIES. No handwritten JSON parser or hash algorithm is introduced.

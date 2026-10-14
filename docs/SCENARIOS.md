# Mission scenarios SIM-001–SIM-005

The browser build implements five deterministic training missions. All use the existing C++20/WebAssembly quadrotor as the flight source of truth. SIM-001 is a quadrotor mission. SIM-002–SIM-005 are explicitly scaled mission-system proxies: they do not claim fixed-wing, MALE, HALE, high-altitude atmosphere, RF propagation, satellite, thermal-control, or multi-day energy physics.

| ID | Implemented route and events | Evaluated completion |
| --- | --- | --- |
| SIM-001 | Preflight, vertical takeoff, low-speed survey, observation mark, return and autoland; gust, GNSS drift, low-energy warning | Waypoints, valid data, GNSS RMS error, link availability, energy reserve, recovery error |
| SIM-002 | VTOL-proxy terrain corridor, observation holds, lost-link procedure, blocked-point reroute and alternate recovery; visibility degradation | Reachable coverage, data/link availability, reroute, lost-link procedure, no-fly violations, recovery |
| SIM-003 | Payload calibration, prioritized EO/IR observation, task reprioritization and recovery; payload freeze and link latency | High-priority dwell, payload/data availability, replanning, energy and recovery |
| SIM-004 | Scaled climb, sector scans, health reporting, LOS/SATCOM-proxy switching and buffering; wind, latency, degraded sensor and weather replan | Coverage, buffer delivery, data/link availability, energy and recovery |
| SIM-005 | Staged scaled climb, grid scan, diagnostics, energy/route replanning and recovery; jet stream, night-energy, thermal and command-delay events | Grid coverage, thermal range, data/link availability, energy and recovery |

Every mission follows `PREFLIGHT → TAKEOFF/CLIMB → TRANSIT/SURVEY/OBSERVE/SCAN → RETURN → LAND → COMPLETE/FAILED`. Scheduled events are deterministic and recorded once in the downloadable `uavsim-mission-report-v1` report. Position, velocity, attitude and applied wind come from the C++ core. Energy, GNSS estimate, link, payload, buffer and thermal values are abstract mission metrics and are labeled as such in the report.

Automatic mission control is enabled by default. Press `M` or use the toolbar to hand control to the pilot. The browser HUD shows current phase, objective, subsystem state, metrics and energy. Completion requires a grounded recovery plus the per-scenario thresholds in `web/scenario-definitions.js`.

The mission lifecycle and failsafe semantics were reviewed against the [PX4 mission mode](https://docs.px4.io/main/en/flight_modes/mission), [PX4 safety configuration](https://docs.px4.io/main/en/config/safety), [PX4 return mode](https://docs.px4.io/main/en/flight_modes/return), and [MAVLink mission protocol](https://mavlink.io/en/services/mission.html). GitHub research included PX4 and ArduPilot mission/failsafe implementations, QGroundControl mission state handling, and JSBSim. No upstream source code was copied and no new runtime dependency was added.

## Verification boundary

- PASS: pure mission tests cover all five definitions, scheduled events, waypoint dwell, blocked-point reroute, completion/failure thresholds, bounded guidance, reports and invalid definitions.
- PASS: all five automatic missions completed against the actual C++/WebAssembly core in `tests/mission_wasm.mjs`; simulated completion times were SIM-001 37.05 s, SIM-002 78.43 s, SIM-003 64.81 s, SIM-004 76.32 s and SIM-005 72.61 s, all grounded.
- PASS: SIM-001 completed in the actual in-app browser using the rebuilt WebAssembly core: 6/6 waypoints, automatic landing, 95% displayed valid data and 26% remaining abstract energy at 37.0 s.
- PASS: actual supplied FBX and all six texture maps rendered during the mission.
- NOT_TESTED: physical gamepad hardware. Browser standard-mapping support is implemented and non-standard mappings are ignored safely.
- BLOCKED in the in-app browser: Pointer Lock is rejected by that embedded document. The click-to-lock and mouse-movement path is implemented; prior Chrome evidence established it, but this session did not capture a new external-Chrome pointer-lock result.
- NOT_IMPLEMENTED: authentic fixed-wing/MALE/HALE aerodynamics, propulsion, high-altitude atmosphere, RF propagation, satellite networks, calibrated sensors, multi-day solar/thermal physics, terrain/tree collision, or real-aircraft validation.

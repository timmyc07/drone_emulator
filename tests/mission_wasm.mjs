import assert from 'node:assert/strict';
import { readFile } from 'node:fs/promises';
import createUAVSimCore from '../web/generated/uavsim.mjs';
import { SCENARIOS } from '../web/scenario-definitions.js';
import { advanceMission, createMissionRun, estimatedPosition, guidanceInput, missionTarget } from '../web/mission-engine.js';

const timestep = .004;
const vehicle = await readFile(new URL('../configs/vehicles/generic_quadrotor_x01.json', import.meta.url), 'utf8');
const core = await createUAVSimCore();
const ok = value => assert.equal(value, true, core.lastError());
const snapshot = () => JSON.parse(core.snapshot());
ok(core.loadVehicle(vehicle));

for (const definition of SCENARIOS.filter(value => value.mode === 'mission')) {
  ok(core.reset());
  let run = createMissionRun(definition), armed = false, state = snapshot();
  for (let step = 0; step < 30000 && run.status === 'ACTIVE'; step++) {
    ok(core.setEnvironment(...run.wind_mps, 1.225));
    let input = {roll:0,pitch:0,yaw:0,altitude_m:0};
    if (run.phase !== 'PREFLIGHT') input = guidanceInput(state, missionTarget(run,definition), estimatedPosition(run,state));
    ok(core.setInput(armed, 2, input.roll, input.pitch, input.yaw, .5, input.altitude_m));
    ok(core.advance(1));
    state = snapshot();
    const previousPhase = run.phase;
    run = advanceMission(run,definition,state,timestep);
    if (previousPhase === 'PREFLIGHT' && run.phase === 'TAKEOFF') armed = true;
  }
  assert.equal(run.status,'COMPLETE',`${definition.id}: ${run.result ?? 'timeout'} phase=${run.phase} waypoint=${run.waypointIndex} position=${state.position_m.map(value=>value.toFixed(1)).join(',')} energy=${run.energy_pct.toFixed(1)}`);
  assert.equal(state.grounded,true,`${definition.id}: recovery must be grounded`);
  assert.equal(run.metrics.waypoint_completion_pct,100);
  console.log(`PASS: ${definition.id} C++/WASM mission proxy completed at ${state.time.toFixed(2)} s, reserve ${run.energy_pct.toFixed(1)}%`);
}

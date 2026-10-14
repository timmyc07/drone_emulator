import assert from 'node:assert/strict';
import { readFile } from 'node:fs/promises';
import { createHash } from 'node:crypto';
import createUAVSimCore from '../web/generated/uavsim.mjs';

const vehicle = await readFile(new URL('../configs/vehicles/generic_quadrotor_x01.json', import.meta.url), 'utf8');
const module = await createUAVSimCore();
const ok = (value) => assert.equal(value, true, module.lastError());
const state = () => JSON.parse(module.snapshot());
assert.equal(module.advance(1), false);
assert.match(module.lastError(), /Load a vehicle/);
ok(module.loadVehicle(vehicle));
assert.equal(state().config_sha256, createHash('sha256').update(vehicle).digest('hex'));
assert.equal(state().timestep_s, 0.004);
ok(module.setInput(true, 2, 0, 0, 0, 0.5, 2));
for (let i = 0; i < 250; i++) ok(module.advance(20));
const hover = state();
assert.equal(hover.step, 5000);
assert.ok(Math.abs(hover.time - 20) < 1e-9);
assert.ok(Math.abs(hover.position_m[2] - 2) < 0.1, `hover altitude ${hover.position_m[2]}`);
assert.ok(Math.abs(Math.hypot(...hover.orientation_wxyz) - 1) < 1e-12);
for (const args of [[true, 2, NaN, 0, 0, 0.5, 2], [true, 0.5, 0, 0, 0, 0.5, 2], [true, 2, 2, 0, 0, 0.5, 2]]) {
  assert.equal(module.setInput(...args), false);
  assert.notEqual(module.lastError(), '');
  assert.deepEqual(state(), hover);
}
for (const steps of [-1, 0.5, 65, Infinity]) {
  assert.equal(module.advance(steps), false);
  assert.deepEqual(state(), hover);
}
assert.equal(module.loadVehicle('{}'), false);
assert.deepEqual(state(), hover);
ok(module.setPaused(true));
ok(module.advance(20));
assert.equal(state().step, 5000);
ok(module.setPaused(false));
ok(module.advance(1));
assert.equal(state().step, 5001);
ok(module.reset());
assert.equal(state().step, 0);
assert.equal(state().armed, false);
assert.equal(state().grounded, true);

// Re-execute actual native recorded inputs; no duplicated JavaScript dynamics.
if (process.argv[2]) {
  const records = (await readFile(process.argv[2], 'utf8')).trim().split('\n').map(JSON.parse);
  let maximumError = 0;
  for (const record of records) {
    const input = record.input;
    const mode = ['rate', 'attitude', 'altitude_hold'].indexOf(input.mode);
    ok(module.setInput(input.armed, mode, input.roll, input.pitch, input.yaw, input.collective, input.altitude_m));
    ok(module.advance(1));
    const actual = state();
    for (const key of ['position_m', 'velocity_mps', 'orientation_wxyz', 'angular_velocity_radps']) {
      for (let i = 0; i < actual[key].length; i++) maximumError = Math.max(maximumError, Math.abs(actual[key][i] - record.state[key][i]));
    }
  }
  assert.ok(maximumError < 1e-9, `native/WASM maximum state error ${maximumError}`);
  console.log(`PASS: native/WASM ${records.length}-step equivalence; maximum state error=${maximumError}`);
}
console.log('PASS: WASM vehicle provenance, hover, invalid input, pause, reset, and failure recovery.');

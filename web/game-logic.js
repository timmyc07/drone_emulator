import { SCENARIOS, scenarioById } from './scenario-definitions.js';

export const STEP = 0.004;
export const GATES = [[12, 3, 0], [25, 5, -3], [39, 6, -8], [53, 5, -4], [67, 4, 3], [80, 3, 0]];
export const clamp = (value, low, high) => Math.min(high, Math.max(low, value));
export { SCENARIOS, scenarioById };
export const toRenderPosition = ([x, y, z]) => [x, z, -y];
export const toRenderQuaternion = ([w, x, y, z]) => [x, z, -y, w];
export function controls(keys, sensitivity) {
  const axis = (positive, negative) => Number(keys.has(positive)) - Number(keys.has(negative));
  return { roll: axis('KeyD', 'KeyA') * sensitivity, pitch: axis('KeyW', 'KeyS') * sensitivity,
    yaw: axis('KeyQ', 'KeyE') * sensitivity, climb: Number(keys.has('Space')) - Number(keys.has('ControlLeft') || keys.has('ControlRight')) };
}
export function deadzone(value, threshold = .14) {
  if (!Number.isFinite(value) || Math.abs(value) <= threshold) return 0;
  return Math.sign(value) * (Math.abs(value) - threshold) / (1 - threshold);
}
export function gamepadFlightInput(gamepad, sensitivity) {
  // The standard layout is preferred, but browsers can expose usable raw axes for
  // DirectInput and vendor devices without assigning the standard label. The common
  // four-axis/two-trigger layout below is deliberately a compatibility fallback.
  if (!gamepad) return {roll:0, pitch:0, yaw:0, climb:0};
  const axis = index => deadzone(gamepad.axes?.[index] ?? 0);
  const button = index => clamp(gamepad.buttons?.[index]?.value ?? 0, 0, 1);
  const rightHorizontal = axis(2);
  // A few browser/DirectInput combinations report the right-stick horizontal
  // channel as axis 3. Standard mapping remains axis 2; use axis 3 only when
  // it is the active candidate.
  const fallbackHorizontal = axis(3);
  const yaw = Math.abs(rightHorizontal) >= Math.abs(fallbackHorizontal) ? rightHorizontal : fallbackHorizontal;
  return {roll:axis(0)*sensitivity, pitch:-axis(1)*sensitivity, yaw:yaw*sensitivity, climb:button(7)-button(6)};
}
export function mergeFlightInput(...inputs) {
  return inputs.reduce((result, input) => ({
    roll:clamp(result.roll + input.roll, -1, 1), pitch:clamp(result.pitch + input.pitch, -1, 1),
    yaw:clamp(result.yaw + input.yaw, -1, 1), climb:clamp(result.climb + input.climb, -1, 1)
  }), {roll:0, pitch:0, yaw:0, climb:0});
}
export function prioritizeManualFlightInput(manual, automatic) {
  const select = axis => Math.abs(manual[axis]) > 0 ? manual[axis] : automatic[axis];
  return {roll:select('roll'), pitch:select('pitch'), yaw:select('yaw'), climb:manual.climb};
}
// Gates face east, matching the initial body +X heading. Crossing is forward-only.
export function crossedGate(previous, current, gate, radius = 2.2) {
  if (previous[0] >= gate[0] || current[0] < gate[0]) return false;
  const t = (gate[0] - previous[0]) / (current[0] - previous[0]);
  const dy = previous[1] + (current[1] - previous[1]) * t - gate[1];
  const dz = previous[2] + (current[2] - previous[2]) * t - gate[2];
  return dy * dy + dz * dz <= radius * radius;
}
export function validSnapshot(s) {
  const vector = (a, n, limit) => Array.isArray(a) && a.length === n && a.every(v => typeof v === 'number' && Number.isFinite(v) && Math.abs(v) <= limit);
  return !!s && Number.isFinite(s.time) && s.time >= 0 && s.time <= 7200 &&
    vector(s.position_m, 3, 100000) && vector(s.velocity_mps, 3, 1000) &&
    vector(s.orientation_wxyz, 4, 1.01) && Math.abs(Math.hypot(...s.orientation_wxyz) - 1) < 0.01 &&
    typeof s.armed === 'boolean' && typeof s.grounded === 'boolean';
}
export function validateReplay(data) {
  if (data?.format !== 'uavsim-state-playback-v1' || !Array.isArray(data.frames) || data.frames.length < 2 || data.frames.length > 18000) throw new Error('回放格式不符，或記錄數量超過限制。');
  let previousTime = -1;
  for (const frame of data.frames) {
    if (!validSnapshot(frame) || frame.time <= previousTime) throw new Error('回放包含無效或未按時間排列的狀態。');
    previousTime = frame.time;
  }
  return data.frames;
}
export function elapsedLabel(seconds) {
  const tenths = Math.round(seconds * 10);
  return `${Math.floor(tenths / 600).toString().padStart(2, '0')}:${((tenths % 600) / 10).toFixed(1).padStart(4, '0')}`;
}

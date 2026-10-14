export const STEP = 0.004;
export const GATES = [[12, 3, 0], [25, 5, -3], [39, 6, -8], [53, 5, -4], [67, 4, 3], [80, 3, 0]];
export const clamp = (value, low, high) => Math.min(high, Math.max(low, value));
export const SCENARIOS = [
  {id:'course', name:'基礎穿環訓練', mode:'course', briefing:'六個航點，練習起降、定高與轉向。', condition:'晴朗・無風'},
  {id:'free', name:'自由飛行', mode:'free', briefing:'熟悉四旋翼姿態與第一人稱視角。', condition:'晴朗・無風'},
  {id:'micro-recon', name:'微型偵察', mode:'course', briefing:'低速穿越近距離航點，練習精密懸停。', condition:'GNSS 遮蔽訓練標記'},
  {id:'route-recon', name:'戰術路線偵察', mode:'course', briefing:'沿預設航點巡查，維持高度與航向。', condition:'資料鏈訓練標記'},
  {id:'search-rescue', name:'搜救掃描', mode:'course', briefing:'以航點覆蓋搜尋區域，維持穩定影像視角。', condition:'低能見度訓練標記'},
  {id:'mapping', name:'測繪格網', mode:'course', briefing:'沿航線保持定高，練習可重複的區域覆蓋。', condition:'格網航線訓練標記'},
  {id:'inspection', name:'基礎設施巡檢', mode:'course', briefing:'低速精確通過檢查點，練習位置保持。', condition:'近距離感測訓練標記'},
  {id:'disaster', name:'災害應變', mode:'course', briefing:'在受限能見度訓練情境中完成路線。', condition:'煙霧／動態障礙待後續物理實作'},
  {id:'cargo', name:'物流與醫療運輸', mode:'course', briefing:'平穩通過航點，練習受控起降程序。', condition:'酬載模型待後續物理實作'},
  {id:'maritime', name:'海上巡邏', mode:'course', briefing:'長距離航線訓練，保持航向與高度。', condition:'海風／移動甲板待後續物理實作'},
  {id:'relay', name:'空中通訊中繼', mode:'free', briefing:'維持定點盤旋，練習空中節點位置保持。', condition:'RF 覆蓋／延遲待後續網路模型實作'},
  {id:'weather', name:'氣象與環境監測', mode:'course', briefing:'沿高度與航點收集訓練資料。', condition:'風場／感測器待後續物理實作'},
  {id:'cbrn', name:'危害區偵測', mode:'course', briefing:'以保守航線通過採樣航點。', condition:'濃度場／採樣待後續模型實作'},
  {id:'research', name:'科研飛行測試', mode:'free', briefing:'自由執行受控飛行，匯出遙測供分析。', condition:'遙測記錄啟用'},
  {id:'traffic', name:'多機避碰訓練', mode:'course', briefing:'以單機航點練習空域分離程序。', condition:'多機代理／通訊待後續模型實作'}
];
export const scenarioById = id => SCENARIOS.find(scenario => scenario.id === id) ?? SCENARIOS[0];
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
  if (!gamepad) return {roll:0, pitch:0, yaw:0, climb:0};
  const axis = index => deadzone(gamepad.axes?.[index] ?? 0);
  const button = index => clamp(gamepad.buttons?.[index]?.value ?? 0, 0, 1);
  return {roll:axis(0)*sensitivity, pitch:-axis(1)*sensitivity, yaw:axis(2)*sensitivity, climb:button(7)-button(6)};
}
export function mergeFlightInput(...inputs) {
  return inputs.reduce((result, input) => ({
    roll:clamp(result.roll + input.roll, -1, 1), pitch:clamp(result.pitch + input.pitch, -1, 1),
    yaw:clamp(result.yaw + input.yaw, -1, 1), climb:clamp(result.climb + input.climb, -1, 1)
  }), {roll:0, pitch:0, yaw:0, climb:0});
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

const wp = (name, position, phase, dwell_s = 1.2, priority = 'normal') => ({name, position, phase, dwell_s, radius_m:2.4, priority});

export const SCENARIOS = [
  {
    schemaVersion:1, id:'sim-001', code:'SIM-001', name:'微型偵察：近距離區域巡查', mode:'mission', fidelity:'四旋翼飛行物理＋縮尺任務系統', vehicleProfile:'微型多旋翼（現有 X-01 工程估計）', timeScale:30,
    briefing:'自檢、低速巡查、影像標記、返航與自動降落。', condition:'陣風／GNSS 漂移／低電量',
    initial:['日間','GNSS 正常','光學相機正常','資料鏈路正常','能源 100%'],
    route:[wp('垂直起飛',[0,0,3],'TAKEOFF'),wp('巡查點 A',[13,0,4],'SURVEY',2),wp('巡查點 B',[25,5,5],'SURVEY',2),wp('觀測區標記',[36,0,4],'OBSERVE',3,'high'),wp('返回起飛區',[6,0,3],'RETURN',1),wp('自動降落',[0,0,0],'LAND',1)],
    events:[{id:'gust',at_s:8,duration_s:7,type:'wind',label:'輕度陣風',wind_mps:[1.8,.6,0]},{id:'gnss-drift',at_s:18,duration_s:8,type:'gnss',label:'GNSS 短暫漂移',error_m:4},{id:'low-energy',at_s:32,duration_s:999,type:'energy',label:'低電量預警',set_pct:28}],
    energyDrainPctPerS:.34, completion:{waypoint_completion_pct:100,data_valid_pct:75,link_availability_pct:85,energy_reserve_pct:8,recovery_error_m:2.5}
  },
  {
    schemaVersion:1, id:'sim-002', code:'SIM-002', name:'小型戰術偵察：地形遮蔽巡邏', mode:'mission', fidelity:'VTOL 程序代理；不含固定翼氣動', vehicleProfile:'縮尺 VTOL 任務程序代理', timeScale:60,
    briefing:'地形走廊巡邏、觀測盤旋、失鏈程序、改道與備援回收。', condition:'鏈路遮蔽／能見度下降／航點不可用',
    initial:['日間至黃昏','多個觀測區','備援返航點','能源 100%'],
    route:[wp('垂直起飛',[0,0,4],'TAKEOFF'),wp('地形走廊入口',[14,-4,7],'TRANSIT'),wp('觀測區 A',[28,-9,9],'OBSERVE',3,'high'),wp('觀測區 B',[43,4,10],'OBSERVE',3),wp('備援觀測區',[56,10,8],'OBSERVE',2),wp('備援返航點',[8,6,4],'RETURN'),wp('回收',[0,0,0],'LAND')],
    events:[{id:'link-shadow',at_s:11,duration_s:9,type:'link',label:'山谷遮蔽：資料鏈路中斷',quality_pct:0},{id:'visibility',at_s:22,duration_s:14,type:'visibility',label:'能見度下降',quality_pct:55},{id:'blocked-waypoint',at_s:28,duration_s:999,type:'block-waypoint',label:'觀測區 B 暫時不可用',waypoint_index:3}],
    energyDrainPctPerS:.24, completion:{waypoint_completion_pct:85,data_valid_pct:65,link_availability_pct:65,energy_reserve_pct:10,recovery_error_m:3,no_fly_violations:0}
  },
  {
    schemaVersion:1, id:'sim-003', code:'SIM-003', name:'中型戰術：多區域持續監視', mode:'mission', fidelity:'任務與酬載程序抽象；不含中型機氣動', vehicleProfile:'縮尺中型平台任務代理', timeScale:90,
    briefing:'依優先序監視，接受重排，切換 EO／IR，處置畫面凍結。', condition:'光照變化／酬載凍結／通訊延遲',
    initial:['3 個優先觀測區','EO／IR 抽象酬載','地面站可重排','能源 100%'],
    route:[wp('起飛與酬載校準',[0,0,5],'TAKEOFF',2),wp('高優先區 Alpha',[18,7,10],'OBSERVE',5,'high'),wp('一般區 Bravo',[36,-7,12],'OBSERVE',3),wp('高優先區 Charlie',[55,5,10],'OBSERVE',5,'high'),wp('重新規劃區 Delta',[34,12,9],'OBSERVE',3),wp('返航',[7,0,5],'RETURN'),wp('回收與卸載',[0,0,0],'LAND')],
    events:[{id:'priority-update',at_s:15,duration_s:999,type:'replan',label:'地面站更新任務優先序'},{id:'payload-freeze',at_s:25,duration_s:7,type:'payload',label:'EO 畫面短暫凍結',available_pct:0},{id:'link-latency',at_s:36,duration_s:10,type:'latency',label:'地面通訊延遲升高',latency_ms:950},{id:'ir-switch',at_s:44,duration_s:999,type:'sensor-mode',label:'切換紅外線觀測',mode:'IR'}],
    energyDrainPctPerS:.18, completion:{waypoint_completion_pct:100,data_valid_pct:70,link_availability_pct:75,energy_reserve_pct:12,recovery_error_m:3,high_priority_dwell_s:8}
  },
  {
    schemaVersion:1, id:'sim-004', code:'SIM-004', name:'MALE：長航時廣域監測', mode:'mission', fidelity:'縮時任務／燃料／通訊抽象；不含 MALE 氣動', vehicleProfile:'MALE 任務系統程序代理', timeScale:300,
    briefing:'縮時爬升、扇區掃描、健康回報、LOS／SATCOM 切換與資料緩存。', condition:'高空風變／衛星延遲／感測器退化／天氣更新',
    initial:['廣域縮尺航線','抽象燃料模型','LOS／SATCOM','能源 100%'],
    route:[wp('飛行計畫驗證與爬升',[0,0,8],'TAKEOFF',2),wp('監測扇區 North',[24,12,18],'SCAN',4,'high'),wp('監測扇區 East',[50,0,22],'SCAN',4),wp('監測扇區 South',[28,-14,20],'SCAN',4),wp('扇區複查',[50,8,22],'SCAN',3),wp('下降進場',[8,0,8],'RETURN'),wp('降落',[0,0,0],'LAND')],
    events:[{id:'high-wind',at_s:12,duration_s:14,type:'wind',label:'高空風場改變',wind_mps:[3.2,1.4,0]},{id:'satcom-delay',at_s:25,duration_s:12,type:'satcom',label:'SATCOM 高延遲：資料緩存',latency_ms:1800},{id:'sensor-degrade',at_s:38,duration_s:10,type:'payload',label:'非關鍵感測器退化',available_pct:55},{id:'weather-update',at_s:48,duration_s:999,type:'replan',label:'預報天氣更新：重規劃返航'}],
    energyDrainPctPerS:.15, completion:{waypoint_completion_pct:100,data_valid_pct:68,link_availability_pct:70,energy_reserve_pct:12,recovery_error_m:3,buffer_delivery_pct:95}
  },
  {
    schemaVersion:1, id:'sim-005', code:'SIM-005', name:'HALE：高空長續航廣域態勢蒐集', mode:'mission', fidelity:'縮時能源／熱控／健康抽象；不含 HALE 高空氣動', vehicleProfile:'HALE 任務系統程序代理', timeScale:1200,
    briefing:'分階段縮尺爬升、網格掃描、診斷、能源與航線再規劃。', condition:'急流／夜間能源下降／熱控偏離／指令延遲',
    initial:['跨區網格','抽象晝夜能源與熱控','縮時多日任務','能源 100%'],
    route:[wp('第一階段爬升',[0,0,10],'TAKEOFF',2),wp('第二階段爬升',[18,0,26],'CLIMB',2),wp('網格 A1',[35,12,38],'SCAN',4,'high'),wp('網格 A2',[55,12,42],'SCAN',4),wp('網格 B2',[55,-12,42],'SCAN',4),wp('網格 B1',[35,-12,38],'SCAN',4),wp('下降進場',[8,0,12],'RETURN'),wp('安全回收',[0,0,0],'LAND')],
    events:[{id:'jetstream',at_s:14,duration_s:15,type:'wind',label:'高空急流變化',wind_mps:[4.2,-1.8,0]},{id:'night-cycle',at_s:28,duration_s:18,type:'solar',label:'夜間能源輸入下降',extra_drain_pct_s:.22},{id:'thermal-offset',at_s:40,duration_s:10,type:'thermal',label:'感測器溫度偏離',temperature_c:78},{id:'command-delay',at_s:52,duration_s:9,type:'latency',label:'地面站指令延遲',latency_ms:2400},{id:'grid-update',at_s:60,duration_s:999,type:'replan',label:'觀測網格調整'}],
    energyDrainPctPerS:.11, completion:{waypoint_completion_pct:100,data_valid_pct:65,link_availability_pct:70,energy_reserve_pct:10,recovery_error_m:3,thermal_in_range_pct:75,grid_coverage_pct:95}
  },
  {id:'course',code:'BASIC',name:'基礎穿環訓練',mode:'course',briefing:'六個航點，練習起降、定高與轉向。',condition:'晴朗・無風'},
  {id:'free',code:'FREE',name:'自由飛行',mode:'free',briefing:'熟悉四旋翼姿態與第一人稱視角。',condition:'晴朗・無風'}
];

export const scenarioById = id => SCENARIOS.find(scenario => scenario.id === id) ?? SCENARIOS[0];
export const isMissionScenario = scenario => scenario?.mode === 'mission';

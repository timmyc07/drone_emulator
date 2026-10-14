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
  {
    schemaVersion:1,id:'sim-006',code:'SIM-006',name:'區域巡邏：封閉圍欄安全程序',mode:'mission',fidelity:'非武器化巡邏程序代理；現有四旋翼核心',vehicleProfile:'固定翼／VTOL 概念的縮尺任務代理',timeScale:75,requiresHumanApproval:true,
    briefing:'人工核准後巡邏合成封閉區，記錄圍欄、鏈路與能源安全狀態。',condition:'陣風／GNSS 漂移／鏈路延遲／返航點不可用',
    initial:['合成地圖與去識別化地標','封閉巡邏區與禁入區','人工核准起飛','武器功能停用'],
    route:[wp('人工核准與起飛',[0,0,4],'TAKEOFF',1),wp('巡邏區 North',[18,4,7],'PATROL',2),wp('巡邏區 East',[32,-4,7],'PATROL',2),wp('安全返航',[7,0,4],'RETURN',1),wp('回收',[0,0,0],'LAND',1)],
    events:[{id:'wind-step',at_s:7,duration_s:6,type:'wind',label:'風場階躍與陣風',wind_mps:[2.4,.8,0]},{id:'gnss-drift',at_s:13,duration_s:5,type:'gnss',label:'GNSS 漂移',error_m:3},{id:'telemetry-delay',at_s:19,duration_s:5,type:'latency',label:'遙測短暫延遲',latency_ms:900},{id:'fence-approach',at_s:25,duration_s:3,type:'safety',label:'接近圍欄：保持等待',state:'HOLD',reason:'geofence_margin'},{id:'home-unavailable',at_s:30,duration_s:2,type:'safety',label:'返航點不可用：切換備援安全降落',state:'SAFE_LAND',reason:'home_unavailable'}],
    safety:{geofence:true,telemetryAudit:true,weaponInterfacesDisabled:true},energyDrainPctPerS:.22,completion:{waypoint_completion_pct:100,data_valid_pct:70,link_availability_pct:75,energy_reserve_pct:10,recovery_error_m:3,no_fly_violations:0,safety_audit_pct:100,weapon_interface_negative_pass:1}
  },
  {
    schemaVersion:1,id:'sim-007',code:'SIM-007',name:'自主巡邏：保守重規劃程序',mode:'mission',fidelity:'非武器化多旋翼／VTOL 任務程序代理；非障礙物物理',vehicleProfile:'縮尺多旋翼／VTOL 任務代理',timeScale:60,
    briefing:'依合成障礙物與低信心資料採減速、等待或返航，保留人工接管紀錄。',condition:'移動合成障礙物／感測雜訊／鏈路間歇中斷',
    initial:['核准巡邏走廊','合成障礙物資料','人工接管通道','無身份或行為辨識'],
    route:[wp('垂直起飛',[0,0,4],'TAKEOFF'),wp('巡邏走廊 A',[16,5,6],'PATROL',2),wp('巡邏走廊 B',[29,-5,6],'PATROL',2),wp('返航',[6,0,4],'RETURN'),wp('降落',[0,0,0],'LAND')],
    events:[{id:'moving-obstacle',at_s:9,duration_s:3,type:'safety',label:'合成障礙物：減速等待並請求人工決策',state:'HOLD',reason:'obstacle_low_confidence'},{id:'map-mismatch',at_s:15,duration_s:2,type:'replan',label:'局部地圖失配：保守重規劃'},{id:'link-intermittent',at_s:21,duration_s:3,type:'link',label:'鏈路間歇中斷：保持安全狀態',quality_pct:0},{id:'energy-bias',at_s:27,duration_s:2,type:'energy',label:'能源模型偏差：返航門檻確認',set_pct:35}],
    safety:{conservativeLowConfidence:true,manualTakeover:true,weaponInterfacesDisabled:true},energyDrainPctPerS:.2,completion:{waypoint_completion_pct:100,data_valid_pct:70,link_availability_pct:70,energy_reserve_pct:10,recovery_error_m:3,safety_audit_pct:100,manual_takeover_ready_pass:1,weapon_interface_negative_pass:1}
  },
  {
    schemaVersion:1,id:'sim-008',code:'SIM-008',name:'定點盤旋：環境監測程序',mode:'mission',fidelity:'非武器化定點保持程序代理；非校準感測模型',vehicleProfile:'縮尺多旋翼／VTOL 任務代理',timeScale:45,
    briefing:'在核准觀測中心盤旋，輸出去識別化區域統計與安全返航決策。',condition:'側風／GNSS 降級／載荷掉幀／電池異常',
    initial:['觀測中心與允許半徑','高度帶與備援降落點','中立廣域環境監測載荷'],
    route:[wp('起飛',[0,0,4],'TAKEOFF'),wp('觀測中心',[18,0,7],'LOITER',5,'high'),wp('備援返航',[6,0,4],'RETURN'),wp('安全降落',[0,0,0],'LAND')],
    events:[{id:'crosswind',at_s:6,duration_s:8,type:'wind',label:'持續側風與陣風',wind_mps:[2.6,1.2,0]},{id:'compass',at_s:11,duration_s:3,type:'safety',label:'磁羅盤干擾：切換保守估測器',state:'HOLD',reason:'estimator_fallback'},{id:'gnss-degraded',at_s:15,duration_s:4,type:'gnss',label:'GNSS 品質退化',error_m:5},{id:'payload-drop',at_s:18,duration_s:4,type:'payload',label:'載荷資料掉幀',available_pct:55},{id:'cell-anomaly',at_s:21,duration_s:2,type:'energy',label:'電池單節異常：安全返航',set_pct:32}],
    safety:{deidentifiedPayload:true,estimatorFallback:true,weaponInterfacesDisabled:true},energyDrainPctPerS:.24,completion:{waypoint_completion_pct:100,data_valid_pct:62,link_availability_pct:80,energy_reserve_pct:10,recovery_error_m:3,safety_audit_pct:100,weapon_interface_negative_pass:1}
  },
  {
    schemaVersion:1,id:'sim-009',code:'SIM-009',name:'非武器化飛行平台：飛安程序測試',mode:'mission',fidelity:'非武器化飛行程序代理；不含 UCAV 氣動或武器功能',vehicleProfile:'不可釋放惰性配重的縮尺飛行程序代理',timeScale:70,requiresHumanApproval:true,
    briefing:'構型鎖定後執行起降與飛安事件程序；所有武器相關介面均拒絕。',condition:'風切／空速合成異常／推進退化／鏈路切換',
    initial:['惰性不可釋放配重','武器訊號、模式與指令停用','人工核准與接管通道'],
    route:[wp('構型鎖定與起飛',[0,0,4],'TAKEOFF'),wp('爬升檢查',[14,0,8],'CLIMB',1),wp('巡航與轉彎',[28,7,8],'CRUISE',2),wp('進近',[8,0,4],'APPROACH',1),wp('降落',[0,0,0],'LAND')],
    events:[{id:'windshear',at_s:6,duration_s:4,type:'wind',label:'風切與陣風',wind_mps:[3,-1,0]},{id:'airspeed-fault',at_s:11,duration_s:3,type:'safety',label:'空速合成訊號異常：飛控保護',state:'HOLD',reason:'airspeed_fault'},{id:'propulsion-degrade',at_s:16,duration_s:3,type:'energy',label:'推進效能降級：確認安全裕度',set_pct:50},{id:'link-failover',at_s:21,duration_s:3,type:'latency',label:'主備鏈路切換',latency_ms:700}],
    safety:{configurationLock:true,weaponInterfacesDisabled:true,manualTakeover:true},energyDrainPctPerS:.2,completion:{waypoint_completion_pct:100,data_valid_pct:75,link_availability_pct:75,energy_reserve_pct:10,recovery_error_m:3,safety_audit_pct:100,weapon_interface_negative_pass:1,configuration_lock_pass:1}
  },
  {
    schemaVersion:1,id:'sim-010',code:'SIM-010',name:'惰性飛行器：非武器化生命週期程序',mode:'mission',fidelity:'惰性飛行器生命週期程序代理；不含終端導引或傷害效果',vehicleProfile:'不含戰鬥部、引信、目標資料或交戰邏輯的縮尺代理',timeScale:70,requiresHumanApproval:true,
    briefing:'驗證構型履歷、互鎖、受控分離、飛行穩定與回收／安全終止程序。',condition:'啟動失敗／GNSS 拒止／鏈路遺失／回收區不可用',
    initial:['虛構區域與非人員標記物','數位履歷與安全互鎖','所有武器與終端導引功能停用'],
    route:[wp('互鎖核准與受控起飛',[0,0,4],'TAKEOFF'),wp('飛行穩定檢查',[15,2,7],'TRANSIT',2),wp('通訊與能源檢查',[28,-3,7],'TRANSIT',2),wp('回收程序',[7,0,4],'RETURN'),wp('隔離降落',[0,0,0],'LAND')],
    events:[{id:'startup-retry',at_s:4,duration_s:2,type:'safety',label:'啟動失敗：互鎖保持後安全重試',state:'HOLD',reason:'startup_interlock'},{id:'gnss-denied',at_s:9,duration_s:3,type:'gnss',label:'GNSS 拒止：估測器降級',error_m:6},{id:'link-loss',at_s:14,duration_s:3,type:'link',label:'鏈路遺失：進入預定安全狀態',quality_pct:0},{id:'recovery-unavailable',at_s:19,duration_s:2,type:'safety',label:'回收區不可用：隔離安全降落',state:'SAFE_LAND',reason:'recovery_unavailable'}],
    safety:{lifecycleAudit:true,configurationLock:true,weaponInterfacesDisabled:true},energyDrainPctPerS:.2,completion:{waypoint_completion_pct:100,data_valid_pct:70,link_availability_pct:70,energy_reserve_pct:10,recovery_error_m:3,safety_audit_pct:100,weapon_interface_negative_pass:1,configuration_lock_pass:1}
  },
  {id:'course',code:'BASIC',name:'基礎穿環訓練',mode:'course',briefing:'六個航點，練習起降、定高與轉向。',condition:'晴朗・無風'},
  {id:'free',code:'FREE',name:'自由飛行',mode:'free',briefing:'熟悉四旋翼姿態與第一人稱視角。',condition:'晴朗・無風'}
];

export const scenarioById = id => SCENARIOS.find(scenario => scenario.id === id) ?? SCENARIOS[0];
export const isMissionScenario = scenario => scenario?.mode === 'mission';

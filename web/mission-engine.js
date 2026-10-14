import { clamp } from './game-logic.js';

const distance3 = (a,b) => Math.hypot(a[0]-b[0],a[1]-b[1],a[2]-b[2]);
const eventActive = (event, elapsed) => elapsed >= event.at_s && elapsed < event.at_s + event.duration_s;
const finiteSnapshot = snapshot => snapshot && Array.isArray(snapshot.position_m) && snapshot.position_m.length === 3 && snapshot.position_m.every(Number.isFinite) && Array.isArray(snapshot.velocity_mps) && snapshot.velocity_mps.every(Number.isFinite);

export function validateMissionDefinition(definition) {
  if (!definition || definition.schemaVersion !== 1 || !/^sim-00[1-5]$/.test(definition.id)) throw new Error('Unsupported mission definition.');
  if (!Array.isArray(definition.route) || definition.route.length < 3) throw new Error('Mission route requires at least three waypoints.');
  if (!Array.isArray(definition.events) || !definition.events.every(event => event.id && Number.isFinite(event.at_s) && event.at_s >= 0 && Number.isFinite(event.duration_s) && event.duration_s > 0)) throw new Error('Mission event schedule is invalid.');
  if (!definition.route.every(point => Array.isArray(point.position) && point.position.length === 3 && point.position.every(Number.isFinite) && point.radius_m > 0 && point.dwell_s >= 0)) throw new Error('Mission waypoint is invalid.');
  return definition;
}

export function createMissionRun(definition) {
  validateMissionDefinition(definition);
  return {scenarioId:definition.id,status:'ACTIVE',phase:'PREFLIGHT',elapsed_s:0,mission_clock_s:0,waypointIndex:0,dwell_s:0,energy_pct:100,
    triggeredEvents:[],activeEvents:[],eventLog:[],dataValid_s:0,linkAvailable_s:0,payloadAvailable_s:0,thermalNormal_s:0,gnssErrorSq_s:0,
    bufferedData:0,deliveredData:0,totalData:0,highPriorityDwell_s:0,replanSuccess:false,lostLinkProcedure:false,rerouteSuccess:false,noFlyViolations:0,
    sensorMode:'EO',link_quality_pct:100,link_latency_ms:80,payload_available_pct:100,visibility_pct:100,gnss_error_m:0,temperature_c:24,
    wind_mps:[0,0,0],messages:['起飛前自檢開始'],result:null};
}

export function guidanceInput(snapshot, target, estimatedPosition = snapshot.position_m) {
  if (!finiteSnapshot(snapshot) || !Array.isArray(target) || target.length !== 3) return {roll:0,pitch:0,yaw:0,altitude_m:0};
  const [w,x,y,z]=snapshot.orientation_wxyz;
  const yaw=Math.atan2(2*(w*z+x*y),1-2*(y*y+z*z));
  const east=target[0]-estimatedPosition[0], north=target[1]-estimatedPosition[1];
  const distance=Math.hypot(east,north), desiredSpeed=Math.min(3.5,distance*.65);
  const desiredEast=distance>.01?east/distance*desiredSpeed:0, desiredNorth=distance>.01?north/distance*desiredSpeed:0;
  const velocityEast=desiredEast-snapshot.velocity_mps[0], velocityNorth=desiredNorth-snapshot.velocity_mps[1];
  const forwardVelocityError=Math.cos(yaw)*velocityEast+Math.sin(yaw)*velocityNorth;
  // FLU body +Y is left; positive roll accelerates toward body -Y (right).
  const leftVelocityError=-Math.sin(yaw)*velocityEast+Math.cos(yaw)*velocityNorth;
  const desiredYaw=Math.atan2(north,east), yawError=Math.atan2(Math.sin(desiredYaw-yaw),Math.cos(desiredYaw-yaw));
  return {roll:clamp(-leftVelocityError*.18,-.55,.55),pitch:clamp(forwardVelocityError*.18,-.55,.55),yaw:clamp(yawError*.55,-.45,.45),altitude_m:clamp(target[2],0,50)};
}

export function advanceMission(run, definition, snapshot, dt) {
  validateMissionDefinition(definition);
  if (!run || run.scenarioId !== definition.id || !finiteSnapshot(snapshot) || !Number.isFinite(dt) || dt < 0 || dt > 1) throw new Error('Mission update input is invalid.');
  if (run.status !== 'ACTIVE') return run;
  const next={...run,triggeredEvents:[...run.triggeredEvents],activeEvents:[],eventLog:[...run.eventLog],messages:[],wind_mps:[0,0,0]};
  next.elapsed_s+=dt; next.mission_clock_s=next.elapsed_s*definition.timeScale;
  if(next.phase==='PREFLIGHT') {
    if(next.elapsed_s<2) return next;
    next.phase='TAKEOFF';next.messages.push('自檢通過；旋翼啟動');
  }
  let extraDrain=0;
  next.link_quality_pct=100;next.link_latency_ms=80;next.payload_available_pct=100;next.visibility_pct=100;next.gnss_error_m=0;next.temperature_c=24;
  for(const event of definition.events) {
    if(next.elapsed_s>=event.at_s && !next.triggeredEvents.includes(event.id)) {next.triggeredEvents.push(event.id);next.eventLog.push({id:event.id,label:event.label,time_s:next.elapsed_s});next.messages.push(event.label);}
    if(!eventActive(event,next.elapsed_s)) continue;
    next.activeEvents.push(event.id);
    if(event.type==='wind')next.wind_mps=[...event.wind_mps];
    if(event.type==='gnss')next.gnss_error_m=event.error_m;
    if(event.type==='link') {next.link_quality_pct=event.quality_pct;next.lostLinkProcedure=true;}
    if(event.type==='visibility')next.visibility_pct=event.quality_pct;
    if(event.type==='payload')next.payload_available_pct=event.available_pct;
    if(['latency','satcom'].includes(event.type)) {next.link_latency_ms=event.latency_ms;if(event.type==='satcom')next.link_quality_pct=45;}
    if(event.type==='energy')next.energy_pct=Math.min(next.energy_pct,event.set_pct);
    if(event.type==='thermal')next.temperature_c=event.temperature_c;
    if(event.type==='solar')extraDrain+=event.extra_drain_pct_s;
    if(event.type==='sensor-mode')next.sensorMode=event.mode;
    if(event.type==='replan')next.replanSuccess=true;
  }
  next.energy_pct=clamp(next.energy_pct-(definition.energyDrainPctPerS+extraDrain)*dt,0,100);
  next.dataValid_s+=dt*(next.payload_available_pct/100)*(next.visibility_pct/100);
  next.payloadAvailable_s+=dt*(next.payload_available_pct/100);
  next.linkAvailable_s+=dt*Number(next.link_quality_pct>=30);
  next.thermalNormal_s+=dt*Number(next.temperature_c>=-20&&next.temperature_c<=70);
  next.gnssErrorSq_s+=next.gnss_error_m*next.gnss_error_m*dt;
  next.totalData+=dt;
  if(next.link_quality_pct<30||next.link_latency_ms>1200)next.bufferedData+=dt;
  else {const flush=Math.min(next.bufferedData,dt*4);next.bufferedData-=flush;next.deliveredData+=dt+flush;}
  if(next.energy_pct<=3){next.status='FAILED';next.result='能源耗盡';next.phase='FAILED';next.messages.push(next.result);return next;}
  const blocked=definition.events.find(event=>event.type==='block-waypoint'&&next.triggeredEvents.includes(event.id));
  if(blocked&&next.waypointIndex===blocked.waypoint_index){next.waypointIndex++;next.rerouteSuccess=true;next.dwell_s=0;next.messages.push('航點不可用；已改道');}
  const target=definition.route[next.waypointIndex];
  if(!target)return snapshot.grounded?finishMission(next,definition,snapshot):{...next,phase:'LAND'};
  const reached=distance3(snapshot.position_m,target.position)<=target.radius_m;
  next.dwell_s=reached?next.dwell_s+dt:0;
  if(reached&&target.priority==='high')next.highPriorityDwell_s+=dt;
  if(next.dwell_s>=target.dwell_s){next.waypointIndex++;next.dwell_s=0;next.phase=definition.route[next.waypointIndex]?.phase??'LAND';next.messages.push(`完成：${target.name}`);}
  if(next.waypointIndex>=definition.route.length&&snapshot.grounded)return finishMission(next,definition,snapshot);
  return next;
}

function finishMission(run,definition,snapshot){
  const metrics=missionMetrics(run,definition,snapshot);
  const maximumMetrics=new Set(['no_fly_violations','recovery_error_m','gnss_error_rms_m','replan_latency_s']);
  const failures=Object.entries(definition.completion).filter(([name,threshold])=>maximumMetrics.has(name)?metrics[name]>threshold:metrics[name]<threshold).map(([name])=>name);
  return {...run,status:failures.length?'FAILED':'COMPLETE',phase:failures.length?'FAILED':'COMPLETE',result:failures.length?`未達門檻：${failures.join(', ')}`:'全部完成條件通過',metrics,messages:[failures.length?'任務未通過':'任務完成']};
}

export function missionTarget(run,definition){
  if(!run||run.status!=='ACTIVE')return [0,0,0];
  if(run.link_quality_pct<30)return [0,0,Math.max(4,definition.route[Math.min(run.waypointIndex,definition.route.length-1)].position[2])];
  return definition.route[Math.min(run.waypointIndex,definition.route.length-1)].position;
}

export function estimatedPosition(run,snapshot){
  const error=run?.gnss_error_m??0, phase=(run?.elapsed_s??0)*1.7;
  return [snapshot.position_m[0]+Math.cos(phase)*error,snapshot.position_m[1]+Math.sin(phase)*error,snapshot.position_m[2]];
}

export function missionMetrics(run,definition,snapshot={position_m:[0,0,0],grounded:false}){
  const elapsed=Math.max(run.elapsed_s,1e-9),completed=Math.min(run.waypointIndex,definition.route.length);
  const delivered=Math.min(run.totalData,run.deliveredData);
  return {waypoint_completion_pct:completed/definition.route.length*100,data_valid_pct:run.dataValid_s/elapsed*100,gnss_error_rms_m:Math.sqrt(run.gnssErrorSq_s/elapsed),
    link_availability_pct:run.linkAvailable_s/elapsed*100,energy_reserve_pct:run.energy_pct,recovery_error_m:Math.hypot(snapshot.position_m[0],snapshot.position_m[1]),
    high_priority_dwell_s:run.highPriorityDwell_s,buffer_delivery_pct:run.totalData?delivered/run.totalData*100:100,thermal_in_range_pct:run.thermalNormal_s/elapsed*100,
    grid_coverage_pct:completed/definition.route.length*100,no_fly_violations:run.noFlyViolations,lost_link_procedure_pass:Number(run.lostLinkProcedure),reroute_pass:Number(run.rerouteSuccess),replan_pass:Number(run.replanSuccess)};
}

export function missionReport(run,definition,snapshot){
  return {format:'uavsim-mission-report-v1',scenario:{id:definition.id,code:definition.code,name:definition.name,fidelity:definition.fidelity,timeScale:definition.timeScale},outcome:run.status,result:run.result,mission_clock_s:run.mission_clock_s,
    metrics:run.metrics??missionMetrics(run,definition,snapshot),metric_sources:{position:'C++ core truth',wind:'C++ core environment',energy:'abstract mission model',GNSS:'abstract navigation estimate',link:'abstract communications model',payload:'abstract sensor availability',thermal:'abstract systems model'},events:run.eventLog,limitations:definition.fidelity};
}

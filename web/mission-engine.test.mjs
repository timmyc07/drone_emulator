import assert from 'node:assert/strict';
import { SCENARIOS } from './scenario-definitions.js';
import { advanceMission, createMissionRun, estimatedPosition, guidanceInput, missionMetrics, missionReport, missionTarget, validateMissionDefinition } from './mission-engine.js';

const missions=SCENARIOS.filter(scenario=>scenario.mode==='mission');
assert.equal(missions.length,5);
assert.deepEqual(missions.map(mission=>mission.id),['sim-001','sim-002','sim-003','sim-004','sim-005']);
const snapshot=(position=[0,0,0],grounded=position[2]===0)=>({position_m:position,velocity_mps:[0,0,0],orientation_wxyz:[1,0,0,0],grounded});

for(const definition of missions){
  validateMissionDefinition(definition);
  let run=createMissionRun(definition);
  for(let i=0;i<70;i++)run=advanceMission(run,definition,snapshot(),1);
  for(const point of definition.route){
    for(let elapsed=0;elapsed<point.dwell_s+.5;elapsed+=.25)run=advanceMission(run,definition,snapshot(point.position,point.phase==='LAND'),.25);
  }
  if(run.status==='ACTIVE')run=advanceMission(run,definition,snapshot([0,0,0],true),.25);
  assert.equal(run.status,'COMPLETE',`${definition.id}: ${run.result}`);
  assert.equal(new Set(run.triggeredEvents).size,run.triggeredEvents.length,`${definition.id} events exactly once`);
  assert.equal(run.triggeredEvents.length,definition.events.length,`${definition.id} all scheduled events`);
  const metrics=run.metrics??missionMetrics(run,definition,snapshot());
  assert.equal(metrics.waypoint_completion_pct,100);
  assert.ok(metrics.energy_reserve_pct>=definition.completion.energy_reserve_pct);
  const report=missionReport(run,definition,snapshot());
  assert.equal(report.format,'uavsim-mission-report-v1');
  assert.equal(report.scenario.id,definition.id);
  assert.equal(report.metric_sources.position,'C++ core truth');
}

const definition=missions[0],run=createMissionRun(definition),state=snapshot([2,1,3],false);
const target=missionTarget({...run,phase:'TAKEOFF'},definition);
assert.deepEqual(target,definition.route[0].position);
const navigation=estimatedPosition({...run,gnss_error_m:4,elapsed_s:2},state);
assert.ok(Math.hypot(navigation[0]-2,navigation[1]-1)>3.99);
const guidance=guidanceInput(state,[100,-100,10],navigation);
for(const value of [guidance.roll,guidance.pitch,guidance.yaw])assert.ok(Number.isFinite(value)&&Math.abs(value)<=1);
assert.equal(guidance.altitude_m,10);
assert.throws(()=>validateMissionDefinition({...definition,route:[]}));
const awaitingGround={...createMissionRun(definition),phase:'LAND',waypointIndex:definition.route.length};
assert.equal(advanceMission(awaitingGround,definition,snapshot([0,0,.5],false),.25).status,'ACTIVE','mission cannot complete before ground contact');
let failed=createMissionRun({...definition,energyDrainPctPerS:100});
failed=advanceMission(failed,{...definition,energyDrainPctPerS:100},state,1);
failed=advanceMission(failed,{...definition,energyDrainPctPerS:100},state,1);
assert.equal(failed.status,'FAILED');
console.log('PASS: five deterministic mission state machines, events, guidance, metrics, completion, failure and report boundaries');

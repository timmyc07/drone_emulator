import * as THREE from 'three';
import { FBXLoader } from 'three/addons/loaders/FBXLoader.js';
import { RoomEnvironment } from 'three/addons/environments/RoomEnvironment.js';
import { STEP, GATES, SCENARIOS, clamp, controls, crossedGate, gamepadFlightInput, mergeFlightInput, scenarioById, toRenderPosition, toRenderQuaternion, validSnapshot, validateReplay, elapsedLabel } from './game-logic.js';
import { isMissionScenario } from './scenario-definitions.js';
import { advanceMission, approveMission, createMissionRun, estimatedPosition, guidanceInput, missionMetrics, missionReport, missionTarget } from './mission-engine.js';

const $ = id => document.getElementById(id);
const log = (event, detail = '') => console.info(`[UAVSim] ${event}`, detail);
const keys = new Set();
const settings = {sensitivity:.5, quality:'medium', units:'metric'};
try {
  const saved = JSON.parse(localStorage.getItem('uavsim-settings') || '{}');
  if (Number.isFinite(saved.sensitivity)) settings.sensitivity = clamp(saved.sensitivity,.2,1);
  if (['low','medium','high'].includes(saved.quality)) settings.quality = saved.quality;
  if (['metric','imperial'].includes(saved.units)) settings.units = saved.units;
} catch (error) { console.warn('[UAVSim] Settings unavailable', error.message); }
let core, renderer, scene, camera, aircraft, model, sun;
let ready = false, state, previousState, running = false, inFlight = false, armed = false;
let mode = 'mission', scenario = scenarioById('sim-001'), cameraMode = 'fpv', targetAltitude = 1, gateIndex = 0, finished = false;
let accumulator = 0, lastFrame = 0, frames = [], lastRecord = -1, recordFull = false;
let playback = null, playbackTime = 0, playbackIndex = 0, toastTimer;
let lookYaw = 0, lookPitch = 0, gamepadIndex = null;
let missionRun = null, autoMission = true, environmentKey = '';
const gamepadButtons = new Map();
const gateMeshes = [], cameraPosition = new THREE.Vector3(), cameraTarget = new THREE.Vector3();
const missionMeshes = [];
const renderPosition = new THREE.Vector3(), renderRotation = new THREE.Quaternion();
const forward = new THREE.Vector3(), desiredCamera = new THREE.Vector3();

function notify(message) {
  $('toast').textContent = message; $('toast').hidden = false;
  clearTimeout(toastTimer); toastTimer = setTimeout(() => { $('toast').hidden = true; }, 3200);
}
function fail(error) {
  console.error('[UAVSim] Flight stopped', error);
  running = false; clearKeys();
  const message = error instanceof Error ? error.message : String(error);
  if (!ready) {
    $('load-status').textContent = `無法開始：${message}`; $('load-status').classList.add('error');
    $('start').textContent = '資源未就緒'; $('start').disabled = true; $('engine-status').textContent = '載入失敗';
  } else {
    pause('飛行已停止', `發生錯誤：${message}。請重設或重新載入頁面。`);
  }
}
function check(result) { if (result === false) throw new Error(core.lastError() || '飛行核心拒絕操作。'); }
function readState() {
  const snapshot = JSON.parse(core.snapshot());
  if (!validSnapshot(snapshot)) throw new Error(core.lastError() || '飛行核心回傳無效狀態。');
  return snapshot;
}
function clearKeys() {
  keys.clear(); document.querySelectorAll('[data-key]').forEach(button => button.classList.remove('pressed'));
}
function material(color, roughness = 1) { return new THREE.MeshStandardMaterial({color, roughness}); }
function mesh(geometry, surface, x, y, z) {
  const object = new THREE.Mesh(geometry, surface); object.position.set(x,y,z); scene.add(object); return object;
}
let randomState = 983;
function random() { randomState = (randomState * 1664525 + 1013904223) >>> 0; return randomState / 4294967296; }

function createWorld() {
  scene = new THREE.Scene(); scene.background = new THREE.Color('#a7b6a2'); scene.fog = new THREE.FogExp2('#a7b6a2', .0065);
  camera = new THREE.PerspectiveCamera(58, innerWidth / innerHeight, .05, 600);
  renderer = new THREE.WebGLRenderer({antialias:true, powerPreference:'high-performance'});
  renderer.outputColorSpace = THREE.SRGBColorSpace;
  renderer.toneMapping = THREE.ACESFilmicToneMapping; renderer.toneMappingExposure = 1.15;
  renderer.shadowMap.enabled = settings.quality !== 'low'; renderer.shadowMap.type = THREE.PCFShadowMap;
  $('viewport').append(renderer.domElement);
  const environment = new RoomEnvironment();
  const pmrem = new THREE.PMREMGenerator(renderer);
  scene.environment = pmrem.fromScene(environment, .04).texture;
  environment.dispose(); pmrem.dispose(); scene.environmentIntensity = .45;
  scene.add(new THREE.HemisphereLight('#e6ecd8','#62734b',2));
  sun = new THREE.DirectionalLight('#fff0cb',3.8); sun.position.set(-35,65,35); sun.castShadow = true;
  sun.shadow.mapSize.set(1024,1024); sun.shadow.camera.left=-65; sun.shadow.camera.right=65;
  sun.shadow.camera.top=65; sun.shadow.camera.bottom=-65; sun.shadow.camera.far=180; sun.shadow.normalBias=.04;
  sun.target.position.set(30,0,0); scene.add(sun, sun.target);
  const ground = mesh(new THREE.PlaneGeometry(900,900),material('#7c8659'),0,-.03,0);
  ground.rotation.x = -Math.PI/2; ground.receiveShadow = true;
  const field = mesh(new THREE.PlaneGeometry(112,42),material('#8d9162'),40,0,0); field.rotation.x=-Math.PI/2; field.receiveShadow=true;
  const stripeMaterial = material('#d2ce9e');
  for (let i=0;i<21;i++) {
    const stripe = mesh(new THREE.PlaneGeometry(1.6,.12),stripeMaterial,i*5-8,.012,0); stripe.rotation.x=-Math.PI/2;
  }
  const pad = mesh(new THREE.CylinderGeometry(2.5,2.5,.04,64),material('#5c6e5a'),0,.025,0); pad.receiveShadow=true;
  const padRing = mesh(new THREE.RingGeometry(2.05,2.13,64),new THREE.MeshBasicMaterial({color:'#e0e5c8',side:THREE.DoubleSide}),0,.051,0); padRing.rotation.x=-Math.PI/2;
  const hTexture = document.createElement('canvas'); hTexture.width=hTexture.height=128;
  const context = hTexture.getContext('2d'); context.fillStyle='#e0e5c8'; context.font='bold 100px sans-serif'; context.textAlign='center';context.textBaseline='middle';context.fillText('H',64,70);
  const h = mesh(new THREE.PlaneGeometry(1.4,1.4),new THREE.MeshBasicMaterial({map:new THREE.CanvasTexture(hTexture),transparent:true}),0,.055,0);h.rotation.x=-Math.PI/2;
  const count=310, trunks = new THREE.InstancedMesh(new THREE.CylinderGeometry(.18,.36,1,6),material('#4a5540'),count);
  const crowns = new THREE.InstancedMesh(new THREE.ConeGeometry(1,1,8),material('#3d6043'),count*2);
  const dummy = new THREE.Object3D(), color=new THREE.Color();
  for(let i=0;i<count;i++) {
    const x=random()*240-70, z=(random()>.5?1:-1)*(26+random()*90), height=7+random()*15, radius=2+random()*2.4;
    dummy.position.set(x,height*.24,z);dummy.scale.set(1,height*.48,1);dummy.updateMatrix();trunks.setMatrixAt(i,dummy.matrix);
    for(let layer=0;layer<2;layer++) {
      dummy.position.set(x,height*(.52+layer*.23),z);dummy.scale.set(radius*(1-layer*.24),height*.62,radius*(1-layer*.24));dummy.updateMatrix();crowns.setMatrixAt(i*2+layer,dummy.matrix);
      color.setHSL(.25+random()*.06,.17+random()*.13,.18+random()*.11);crowns.setColorAt(i*2+layer,color);
    }
  }
  trunks.castShadow=true;crowns.castShadow=true;crowns.receiveShadow=true;scene.add(trunks,crowns);
  for(let i=0;i<24;i++) {
    const angle=i/24*Math.PI*2, radius=190+random()*100;
    const hill=mesh(new THREE.IcosahedronGeometry(1,2),material('#6c8068'),Math.cos(angle)*radius,10,Math.sin(angle)*radius);
    hill.scale.set(45+random()*50,30+random()*65,50+random()*50);hill.rotation.y=random()*Math.PI;
  }
  const markerMaterial=material('#e6e4c2');
  for(let i=0;i<18;i++) for(const z of [-20,20]) {
    const marker=mesh(new THREE.CylinderGeometry(.05,.05,.8,5),markerMaterial,i*6-8,.4,z);marker.castShadow=true;
  }
  GATES.forEach((position,index) => {
    const group=new THREE.Group(); group.position.fromArray(position);
    const hoop=new THREE.Mesh(new THREE.TorusGeometry(2.35,.08,8,64),new THREE.MeshStandardMaterial({color:'#d4f297',emissive:'#b6d579',emissiveIntensity:1.3,roughness:.5}));
    hoop.rotation.y=Math.PI/2;group.add(hoop);
    const pole=new THREE.Mesh(new THREE.CylinderGeometry(.035,.035,position[1],5),material('#d3d8b6'));pole.position.set(0,-position[1]/2,-2.35);group.add(pole);
    const labelCanvas=document.createElement('canvas');labelCanvas.width=labelCanvas.height=128;
    const ctx=labelCanvas.getContext('2d');ctx.fillStyle='#d4ee8d';ctx.font='bold 72px sans-serif';ctx.textAlign='center';ctx.fillText(String(index+1).padStart(2,'0'),64,86);
    const label=new THREE.Sprite(new THREE.SpriteMaterial({map:new THREE.CanvasTexture(labelCanvas),transparent:true}));label.position.y=3.1;label.scale.set(1.5,1.5,1);group.add(label);
    scene.add(group);gateMeshes.push(group);
  });
  aircraft = new THREE.Group(); scene.add(aircraft);
  camera.position.set(-7,4.5,10);camera.lookAt(0,1,0);cameraPosition.copy(camera.position);cameraTarget.set(0,1,0);
  resize(); addEventListener('resize',resize);
  renderer.domElement.addEventListener('webglcontextlost',event => {event.preventDefault(); fail(new Error('繪圖連線已中斷，請重新載入頁面。'));});
  renderer.domElement.addEventListener('click', () => {
    if (inFlight && running && cameraMode === 'fpv' && !document.querySelector('dialog[open]') && document.pointerLockElement !== renderer.domElement) renderer.domElement.requestPointerLock();
  });
  document.addEventListener('pointerlockchange', () => {
    if (document.pointerLockElement === renderer.domElement) log('First-person mouse look captured');
  });
  document.addEventListener('pointerlockerror', () => { console.warn('[UAVSim] Pointer Lock unavailable'); notify('瀏覽器未允許鎖定游標；鍵盤與手把仍可操作。'); });
  document.addEventListener('mousemove', event => {
    if (document.pointerLockElement !== renderer.domElement || !inFlight || !running || cameraMode !== 'fpv') return;
    lookYaw -= event.movementX * .0025 * settings.sensitivity;
    lookPitch = clamp(lookPitch - event.movementY * .0025 * settings.sensitivity, -1.2, 1.2);
  });
  renderer.setAnimationLoop(animate);
}
function resize() {
  if(!renderer)return;
  renderer.setPixelRatio(Math.min(devicePixelRatio,settings.quality==='high'?2:settings.quality==='low'?1:1.4));
  renderer.setSize(innerWidth,innerHeight);camera.aspect=innerWidth/innerHeight;camera.updateProjectionMatrix();
  renderer.shadowMap.enabled=settings.quality!=='low';
}

async function loadDrone() {
  const base='./assets/drone/', names={map:'SpaceProbe_[diffuse].png',normalMap:'SpaceProbe_[normals].png',metalnessMap:'SpaceProbe_[metal].png',roughnessMap:'SpaceProbe_[gloss].png',aoMap:'Spaceprobe_[occlusion].png',emissiveMap:'SpaceProbe_[emission-yellow].png'};
  const manager=new THREE.LoadingManager();
  manager.setURLModifier(url => {
    if(url.startsWith('data:') || url.startsWith('blob:'))return url;
    const name=decodeURIComponent(url.replaceAll('\\','/').split('/').pop());
    return Object.values(names).includes(name)?base+'textures/'+encodeURIComponent(name):url;
  });
  const response=await fetch(base+'source/'+encodeURIComponent('QuadDrone [mesh].fbx'));
  if(!response.ok)throw new Error('找不到指定的 QuadDrone FBX 模型。請執行網頁準備程序。');
  const source=await response.arrayBuffer();
  const textures=await Promise.all(Object.entries(names).map(async([channel,name]) => {
    const texture=await new THREE.TextureLoader().loadAsync(base+'textures/'+encodeURIComponent(name)).catch(()=>{throw new Error(`指定貼圖 ${name} 載入失敗。`);});
    const image=texture.image, ratio=Math.min(1,2048/Math.max(image.width,image.height));
    const canvas=document.createElement('canvas');canvas.width=Math.max(1,Math.round(image.width*ratio));canvas.height=Math.max(1,Math.round(image.height*ratio));
    const context=canvas.getContext('2d',{willReadFrequently:channel==='roughnessMap'});context.drawImage(image,0,0,canvas.width,canvas.height);
    if(channel==='roughnessMap') {
      const pixels=context.getImageData(0,0,canvas.width,canvas.height);
      for(let i=0;i<pixels.data.length;i+=4) pixels.data[i]=pixels.data[i+1]=pixels.data[i+2]=255-pixels.data[i];
      context.putImageData(pixels,0,0);
    }
    texture.dispose(); const result=new THREE.CanvasTexture(canvas);
    result.colorSpace=['map','emissiveMap'].includes(channel)?THREE.SRGBColorSpace:THREE.NoColorSpace;
    result.anisotropy=Math.min(4,renderer.capabilities.getMaxAnisotropy());return [channel,result];
  }));
  const surface=new THREE.MeshStandardMaterial({...Object.fromEntries(textures),metalness:1,roughness:1,emissive:'#ffffff',emissiveIntensity:.7,aoMapIntensity:.7});
  model=new FBXLoader(manager).parse(source,base+'textures/');
  let meshes=0;
  model.traverse(object => {
    if(!object.isMesh)return;meshes++;object.material=surface;object.castShadow=true;object.receiveShadow=true;
  });
  if(!meshes)throw new Error('指定 FBX 未包含可顯示的網格。');
  // Pinned FBXLoader already converts the file's declared up axis to Three.js Y-up.
  const box=new THREE.Box3().setFromObject(model), size=box.getSize(new THREE.Vector3());
  if(!Number.isFinite(size.length()) || size.length()===0)throw new Error('指定模型的尺寸無效。');
  model.scale.multiplyScalar(1.4/Math.max(size.x,size.z));model.updateMatrixWorld(true);box.setFromObject(model);
  const center=box.getCenter(new THREE.Vector3());model.position.sub(center);model.position.y+=box.getSize(new THREE.Vector3()).y/2+.08;
  aircraft.add(model);$('viewport').dataset.model='supplied-fbx';$('viewport').dataset.textures='6';
  log('Supplied FBX and all six textures ready',{sourceBytes:source.byteLength,meshes,renderSize:new THREE.Box3().setFromObject(model).getSize(new THREE.Vector3()).toArray()});
}

function refreshGates() {
  gateMeshes.forEach((group,index)=>{group.visible=mode==='course';group.children[0].material.color.set(index<gateIndex?'#708767':index===gateIndex?'#d9f58b':'#bbc9b4');group.children[0].material.emissiveIntensity=index===gateIndex?1.5:.08;group.children[2].material.opacity=index===gateIndex?1:.4;});
  missionMeshes.forEach((marker,index)=>{marker.visible=mode==='mission';marker.children[0].material.color.set(index<(missionRun?.waypointIndex??0)?'#708767':index===(missionRun?.waypointIndex??0)?'#d9f58b':'#91a69a');marker.children[0].material.emissiveIntensity=index===(missionRun?.waypointIndex??0)?1.4:.1;});
  $('gate-count').innerHTML=mode==='mission'?`${Math.min((missionRun?.waypointIndex??0)+1,scenario.route.length).toString().padStart(2,'0')} <span>/ ${scenario.route.length.toString().padStart(2,'0')}</span>`:mode==='course'?`${Math.min(gateIndex+1,6).toString().padStart(2,'0')} <span>/ 06</span>`:'FREE <span>FLIGHT</span>';
  $('mission-label').textContent=playback?'飛行狀態回放':scenario.name;
  $('mission-progress').style.width=`${mode==='mission'?(missionRun?.waypointIndex??0)/scenario.route.length*100:gateIndex/6*100}%`;
}
function configureMissionMarkers() {
  for(const marker of missionMeshes){marker.traverse(child=>{child.geometry?.dispose();if(Array.isArray(child.material))child.material.forEach(value=>value.dispose());else child.material?.dispose();});scene.remove(marker);}missionMeshes.length=0;
  if(!isMissionScenario(scenario))return;
  scenario.route.forEach((point,index)=>{
    const group=new THREE.Group();group.position.fromArray(toRenderPosition(point.position));
    const beacon=new THREE.Mesh(new THREE.SphereGeometry(.38,12,8),new THREE.MeshStandardMaterial({color:'#91a69a',emissive:'#d9f58b',emissiveIntensity:.1}));group.add(beacon);
    const beam=new THREE.Mesh(new THREE.CylinderGeometry(.035,.035,Math.max(.2,point.position[2]),5),material('#d3d8b6'));beam.position.y=-point.position[2]/2;group.add(beam);
    group.userData={missionIndex:index,name:point.name};scene.add(group);missionMeshes.push(group);
  });
}
function setCoreEnvironment(wind=[0,0,0],airDensity=1.225) {
  const key=[...wind,airDensity].join(',');if(key===environmentKey)return;
  check(core.setEnvironment(wind[0],wind[1],wind[2],airDensity));environmentKey=key;
}
function resetSession() {
  check(core.reset());check(core.setPaused(false));state=readState();previousState=state;
  environmentKey='';setCoreEnvironment();missionRun=isMissionScenario(scenario)?createMissionRun(scenario):null;autoMission=!!missionRun;
  targetAltitude=missionRun?0:1;gateIndex=0;finished=false;armed=!missionRun;accumulator=0;frames=[];lastRecord=-1;recordFull=false;playback=null;lookYaw=lookPitch=0;
  configureMissionMarkers();clearKeys();refreshGates();log('Session reset',{scenario:scenario.id,fidelity:scenario.fidelity??'basic'});
}
function start() {
  if(!ready)return;
  try {
    scenario=scenarioById($('scenario').value);mode=scenario.mode;resetSession();inFlight=true;running=true;
    $('menu').hidden=true;$('scene-caption').hidden=true;$('flight-hud').hidden=false;
    $('pause-dialog').close();notify(missionRun?'任務自檢開始；自動任務控制已啟用。':'第一人稱已啟用。點擊畫面後用滑鼠查看；Space 上升，W 向前。');
  } catch(error){fail(error);}
}
function pause(title='稍作停留',message='飛行已暫停，準備好再繼續。') {
  if(!inFlight)return;
  running=false;clearKeys();accumulator=0;
  if(core)check(core.setPaused(true));
  $('pause-title').textContent=title;$('pause-message').textContent=message;
  $('resume').disabled=finished && !playback;$('save-session').disabled=!!playback || frames.length<2;$('save-csv').disabled=!!playback || frames.length<2;
  $('mission-report').disabled=!missionRun;
  if(!$('pause-dialog').open)$('pause-dialog').showModal();
  log('Paused',title);
}
function resume() {
  if(!inFlight || (finished&&!playback))return;
  if(playback && playbackIndex>=playback.length-1){playbackTime=0;playbackIndex=0;}
  check(core.setPaused(false));running=true;clearKeys();accumulator=0;$('pause-dialog').close();log('Resumed');
}
function backToMenu() {
  running=false;inFlight=false;clearKeys();check(core.setPaused(true));
  $('pause-dialog').close();$('menu').hidden=false;$('scene-caption').hidden=false;$('flight-hud').hidden=true;
  $('start').innerHTML='開始飛行 <span>→</span>';
  aircraft.visible=true;log('Returned to menu');
}
function toggleArmed() {
  if(!running||playback)return;armed=!armed;
  if(armed)targetAltitude=Math.max(1,state.position_m[2]);
  notify(armed?'旋翼已啟動':'旋翼已停轉，飛行器將下降');log('Arm state changed',armed);
}
function openDialog(id) {
  if(running)pause();$(id).showModal();
}
function saveFile(name,content,type) {
  const url=URL.createObjectURL(new Blob([content],{type}));const link=document.createElement('a');link.href=url;link.download=name;link.click();setTimeout(()=>URL.revokeObjectURL(url),1000);log('Session downloaded',name);
}
function exportSession() {
  if(playback)return;
  if(frames.length<2)return notify('飛行記錄尚不足，請先飛行。');
  saveFile('uavsim-flight.json',JSON.stringify({format:'uavsim-state-playback-v1',description:'Recorded state playback; not deterministic input replay.',vehicle_id:state.vehicle_id,config_sha256:state.config_sha256,core_version:state.core_version,frames}),'application/json');
}
function exportCsv() {
  if(playback)return;
  if(frames.length<2)return notify('飛行記錄尚不足，請先飛行。');
  const rows=['time_s,east_m,north_m,up_m,velocity_east_mps,velocity_north_mps,velocity_up_mps,q_w,q_x,q_y,q_z,armed,grounded'];
  for(const frame of frames)rows.push([frame.time,...frame.position_m,...frame.velocity_mps,...frame.orientation_wxyz,Number(frame.armed),Number(frame.grounded)].join(','));
  saveFile('uavsim-telemetry.csv',rows.join('\n'),'text/csv');
}
function exportMissionReport() {
  if(!missionRun||!state)return notify('目前沒有任務報告。');
  saveFile(`${scenario.id}-mission-report.json`,JSON.stringify(missionReport(missionRun,scenario,state),null,2),'application/json');
}
async function openReplay(file) {
  if(!file||!ready)return;
  try {
    if(file.size>16*1024*1024)throw new Error('回放檔案超過 16 MB 限制。');
    const recorded=validateReplay(JSON.parse(await file.text()));
    running=false;check(core.setPaused(true));playback=recorded;missionRun=null;playbackTime=0;playbackIndex=0;state=playback[0];previousState=state;
    inFlight=true;running=true;finished=false;mode='free';clearKeys();$('menu').hidden=true;$('scene-caption').hidden=true;$('flight-hud').hidden=false;refreshGates();
    notify('正在播放記錄狀態；這不是物理重新運算。');log('State playback opened',{frames:recorded.length});
  }catch(error){console.error('[UAVSim] Invalid replay',error.message);notify(`無法開啟回放：${error.message}`);}
}

function updateSimulation(delta) {
  if(playback) {
    playbackTime+=delta;
    const time=playback[0].time+playbackTime;
    while(playbackIndex<playback.length-1 && playback[playbackIndex+1].time<=time)playbackIndex++;
    state=playback[playbackIndex];previousState=state;
    if(playbackIndex===playback.length-1)pause('回放結束','這段記錄已播放完畢。按繼續可再播放一次。');
    return;
  }
  accumulator+=delta;
  const steps=Math.min(64,Math.floor(accumulator/STEP));
  if(!steps)return;
  const gamepad = gamepadIndex === null ? [...navigator.getGamepads()].find(Boolean) : navigator.getGamepads()[gamepadIndex];
  let input=mergeFlightInput(controls(keys,settings.sensitivity),gamepadFlightInput(gamepad,settings.sensitivity));
  if(missionRun) {
    setCoreEnvironment(missionRun.wind_mps);
    if(autoMission&&missionRun.phase!=='PREFLIGHT') {
      const target=missionTarget(missionRun,scenario), guidance=guidanceInput(state,target,estimatedPosition(missionRun,state));
      input=mergeFlightInput(input,{roll:guidance.roll,pitch:guidance.pitch,yaw:guidance.yaw,climb:0});targetAltitude=guidance.altitude_m;
    } else targetAltitude=clamp(targetAltitude+input.climb*2.5*steps*STEP,0,50);
  } else targetAltitude=clamp(targetAltitude+input.climb*2.5*steps*STEP,0,50);
  check(core.setInput(armed,2,input.roll,input.pitch,input.yaw,.5,targetAltitude));
  const before=toRenderPosition(state.position_m);
  if(steps>1){check(core.advance(steps-1));previousState=readState();}else previousState=state;
  check(core.advance(1));state=readState();accumulator-=steps*STEP;
  if(missionRun) {
    const previousPhase=missionRun.phase;missionRun=advanceMission(missionRun,scenario,state,steps*STEP);
    if(previousPhase==='PREFLIGHT'&&missionRun.phase==='TAKEOFF')armed=true;
    for(const message of missionRun.messages){notify(message);log('Mission event',{scenario:scenario.id,phase:missionRun.phase,message});}
    refreshGates();
    if(missionRun.status!=='ACTIVE'&&!finished){finished=true;armed=false;pause(missionRun.status==='COMPLETE'?'任務完成':'任務未通過',missionRun.result);}
  }
  if(!recordFull && state.time-lastRecord>=.1) {
    frames.push(state);lastRecord=state.time;
    if(frames.length===18000){recordFull=true;notify('本段記錄已達上限。可匯出後重新開始。');log('Recording limit reached');}
  }
  if(mode==='course'&&!finished && crossedGate(before,toRenderPosition(state.position_m),GATES[gateIndex])) {
    gateIndex++;refreshGates();log('Gate crossed',{gate:gateIndex,time:state.time});
    if(gateIndex===GATES.length) {
      finished=true;const score=Math.max(100,Math.round(6000-state.time*25));
      pause('漂亮，完成航線！',`6 個航點全部通過，用時 ${elapsedLabel(state.time)}，訓練得分 ${score}。`);
    } else notify(`航點 ${gateIndex} / 6 完成`);
  }
}
function updateHud() {
  if(!state)return;
  $('mission-systems').hidden=!missionRun;
  const imperial=settings.units==='imperial',height=state.position_m[2]*(imperial?3.28084:1),speed=Math.hypot(state.velocity_mps[0],state.velocity_mps[1])*(imperial?2.23694:1),vertical=state.velocity_mps[2]*(imperial?3.28084:1);
  const [w,x,y,z]=state.orientation_wxyz,yaw=Math.atan2(2*(w*z+x*y),1-2*(y*y+z*z));
  $('altitude').innerHTML=`${height.toFixed(1)}<em>${imperial?'ft':'m'}</em>`;
  $('speed').innerHTML=`${speed.toFixed(1)}<em>${imperial?'mph':'m/s'}</em>`;
  $('vertical').innerHTML=`${vertical.toFixed(1)}<em>${imperial?'ft/s':'m/s'}</em>`;
  $('heading').innerHTML=`${Math.round((90-yaw*180/Math.PI+360)%360).toString().padStart(3,'0')}<em>°</em>`;
  $('mission-time').textContent=elapsedLabel(state.time);
  $('flight-state').textContent=playback?'記錄回放':state.armed?'定高輔助':'旋翼停轉';
  $('record-state').textContent=playback?'狀態播放':recordFull?'記錄已滿':running?'● 記錄中':'已暫停';
  $('arm').innerHTML=`${armed?'停轉':'啟動'} <kbd>F</kbd>`;$('arm').disabled=!!playback;
  if(missionRun) {
    const target=scenario.route[Math.min(missionRun.waypointIndex,scenario.route.length-1)],metrics=missionMetrics(missionRun,scenario,state);
    $('mission-hint').textContent=`${missionRun.phase} · ${target?.name??'完成'} · 能源 ${missionRun.energy_pct.toFixed(0)}%`;
    $('mission-phase').textContent=missionRun.phase;$('mission-objective').textContent=target?.name??missionRun.result??'完成';
    $('subsystem-state').textContent=`LINK ${missionRun.link_quality_pct.toFixed(0)}% · GNSS ±${missionRun.gnss_error_m.toFixed(1)}m · ${missionRun.sensorMode} · ${missionRun.temperature_c.toFixed(0)}°C`;
    $('metric-state').textContent=`航點 ${metrics.waypoint_completion_pct.toFixed(0)}% · 資料 ${metrics.data_valid_pct.toFixed(0)}% · 鏈路 ${metrics.link_availability_pct.toFixed(0)}%`;
    $('auto-mission').innerHTML=`${autoMission?'自動任務 ON':'自動任務 OFF'} <kbd>M</kbd>`;$('auto-mission').hidden=false;
    $('approve-takeoff').hidden=!scenario.requiresHumanApproval||missionRun.humanApproved;$('approve-takeoff').disabled=!!playback;
    $('mission-report').disabled=false;
  } else if(mode==='course' && gateIndex<GATES.length) {
    $('approve-takeoff').hidden=true;
    const distance=Math.hypot(...toRenderPosition(state.position_m).map((v,i)=>v-GATES[gateIndex][i]));
    $('mission-hint').textContent=`距離 ${distance.toFixed(0)} m · 目標高度 ${GATES[gateIndex][1]} m`;
    $('auto-mission').hidden=true;
  } else {$('mission-hint').textContent=playback?'重現已記錄的飛行狀態':`${scenario.briefing} · ${scenario.condition}`;$('auto-mission').hidden=true;$('approve-takeoff').hidden=true;}
}
function updateView(delta) {
  if(!state)return;
  renderPosition.fromArray(toRenderPosition(state.position_m));renderRotation.fromArray(toRenderQuaternion(state.orientation_wxyz));
  // Interpolate the last physics interval; rendered transforms never write into physics.
  if(running&&!playback&&previousState) {
    const blend=clamp(accumulator/STEP,0,1);
    renderPosition.lerpVectors(new THREE.Vector3().fromArray(toRenderPosition(previousState.position_m)),renderPosition,blend);
    renderRotation.slerpQuaternions(new THREE.Quaternion().fromArray(toRenderQuaternion(previousState.orientation_wxyz)),renderRotation,blend);
  }
  aircraft.position.copy(renderPosition);aircraft.quaternion.copy(renderRotation);
  forward.set(1,0,0).applyQuaternion(renderRotation);forward.y=0;forward.normalize();
  if(!inFlight) {
    desiredCamera.copy(renderPosition).add(new THREE.Vector3(-5,3.8,7));
    cameraTarget.copy(renderPosition).add(new THREE.Vector3(-1.8,.8,-1.7));aircraft.visible=true;
  }else if(cameraMode==='fpv') {
    desiredCamera.copy(renderPosition).addScaledVector(forward,.7);desiredCamera.y+=.35;
    cameraTarget.copy(desiredCamera).addScaledVector(forward,15);cameraTarget.y+=forward.y;aircraft.visible=false;
  }else {
    desiredCamera.copy(renderPosition).addScaledVector(forward,-7);desiredCamera.y+=3.5;
    desiredCamera.add(new THREE.Vector3(0,0,2));cameraTarget.copy(renderPosition).addScaledVector(forward,3);cameraTarget.y+=.7;aircraft.visible=true;
  }
  cameraPosition.lerp(desiredCamera,1-Math.exp(-delta*(cameraMode==='fpv'?25:5)));camera.position.copy(cameraPosition);camera.lookAt(cameraTarget);
  if(cameraMode==='fpv') { camera.rotateY(lookYaw);camera.rotateX(lookPitch); }
  sun.position.set(renderPosition.x-35,65,renderPosition.z+35);sun.target.position.set(renderPosition.x,0,renderPosition.z);
}
function animate(timestamp) {
  const delta=lastFrame?Math.min((timestamp-lastFrame)/1000,.064):0;lastFrame=timestamp;
  try {if(ready && running){updateGamepads();updateSimulation(delta);}if(ready){updateView(delta);updateHud();}renderer.render(scene,camera);}catch(error){fail(error);}
}

$('start').onclick=start;$('menu-button').onclick=()=>inFlight?pause():openDialog('help-dialog');
$('brand').onclick=event=>{event.preventDefault();if(inFlight)pause();};
$('pause').onclick=()=>pause();$('resume').onclick=resume;$('back-menu').onclick=backToMenu;$('arm').onclick=toggleArmed;
function restart(){if(!ready||!inFlight)return;try{resetSession();running=true;$('pause-dialog').close();notify('已重設，在起飛點重新開始。');}catch(error){fail(error);}}
$('reset').onclick=restart;$('pause-reset').onclick=restart;
function toggleCamera(){cameraMode=cameraMode==='chase'?'fpv':'chase';$('camera').innerHTML=`${cameraMode==='chase'?'追蹤視角':'第一人稱'} <kbd>C</kbd>`;if(cameraMode==='chase'&&document.pointerLockElement===renderer.domElement)document.exitPointerLock();}
$('camera').onclick=toggleCamera;
function toggleAutoMission(){if(!missionRun||playback)return;autoMission=!autoMission;notify(autoMission?'自動任務控制已啟用':'自動任務控制已關閉；由您接管飛行');log('Mission control changed',autoMission);}
function approveTakeoff(){if(!missionRun||playback||missionRun.humanApproved)return;missionRun=approveMission(missionRun,scenario);notify('人工核准已記錄。');log('Mission approved',{scenario:scenario.id});refreshGates();}
$('auto-mission').onclick=toggleAutoMission;$('mission-report').onclick=exportMissionReport;
$('approve-takeoff').onclick=approveTakeoff;
$('settings-open').onclick=()=>openDialog('settings-dialog');$('help-open').onclick=()=>openDialog('help-dialog');$('help-flight').onclick=()=>openDialog('help-dialog');
$('save-session').onclick=exportSession;$('save-csv').onclick=exportCsv;
$('replay-open').onclick=()=>{if(ready)$('replay-file').click();else notify('請等待資源載入完成。');};
$('replay-file').onchange=event=>{openReplay(event.target.files[0]);event.target.value='';};
document.querySelectorAll('[data-close]').forEach(button=>button.onclick=()=>$(button.dataset.close).close());
$('pause-dialog').addEventListener('cancel',event=>{event.preventDefault();resume();});
for(const id of ['settings-dialog','help-dialog'])$(id).addEventListener('close',clearKeys);
$('sensitivity').value=settings.sensitivity*100;$('sensitivity-value').textContent=`${Math.round(settings.sensitivity*100)}%`;$('quality').value=settings.quality;$('units').value=settings.units;
for(const id of ['sensitivity','quality','units'])$(id).addEventListener('input',()=>{
  settings.sensitivity=Number($('sensitivity').value)/100;settings.quality=$('quality').value;settings.units=$('units').value;
  $('sensitivity-value').textContent=`${Math.round(settings.sensitivity*100)}%`;resize();
  try{localStorage.setItem('uavsim-settings',JSON.stringify(settings));}catch(error){console.warn('[UAVSim] Settings not saved',error.message);notify('瀏覽器無法儲存設定，此次設定仍有效。');}
});
const flightKeys=new Set(['KeyW','KeyS','KeyA','KeyD','KeyQ','KeyE','Space','ControlLeft','ControlRight']);
addEventListener('keydown',event=>{
  if(!inFlight || ['INPUT','SELECT','TEXTAREA'].includes(event.target.tagName))return;
  if(document.querySelector('dialog[open]')) {
    if(event.code==='KeyP' && !event.repeat && $('pause-dialog').open && !($('help-dialog').open||$('settings-dialog').open)){event.preventDefault();resume();}
    return;
  }
  if(flightKeys.has(event.code)){event.preventDefault();if(running&&!playback)keys.add(event.code);return;}
  if(event.repeat)return;
  const actions={KeyP:()=>pause(),KeyF:toggleArmed,KeyR:restart,KeyC:toggleCamera,KeyM:toggleAutoMission,KeyV:approveTakeoff,Escape:()=>pause()};
  if(actions[event.code]){event.preventDefault();actions[event.code]();}
});
addEventListener('keyup',event=>keys.delete(event.code));
addEventListener('blur',()=>{clearKeys();if(running)pause('已自動暫停','您已離開遊戲視窗；按繼續可恢復飛行。');});
document.addEventListener('visibilitychange',()=>{if(document.hidden&&running)pause('已自動暫停','分頁進入背景，飛行已安全暫停。');});
document.querySelectorAll('[data-key]').forEach(button=>{
  button.addEventListener('pointerdown',event=>{if(!running||playback)return;event.preventDefault();button.setPointerCapture(event.pointerId);keys.add(button.dataset.key);button.classList.add('pressed');});
  const release=()=>{keys.delete(button.dataset.key);button.classList.remove('pressed');};
  button.addEventListener('pointerup',release);button.addEventListener('pointercancel',release);button.addEventListener('lostpointercapture',release);
});
function gamepadPressed(gamepad, button) { return !!gamepad.buttons?.[button]?.pressed; }
function updateGamepads() {
  for (const gamepad of navigator.getGamepads()) {
    if (!gamepad) continue;
    gamepadIndex ??= gamepad.index;
    const previous = gamepadButtons.get(gamepad.index) ?? [];
    const current = gamepad.buttons.map(button => button.pressed);
    if (inFlight && running && !playback) {
      if (gamepadPressed(gamepad, 0) && !previous[0]) toggleArmed();
      if (gamepadPressed(gamepad, 3) && !previous[3]) toggleCamera();
      if ((gamepadPressed(gamepad, 9) || gamepadPressed(gamepad, 16)) && !(previous[9] || previous[16])) pause();
    }
    gamepadButtons.set(gamepad.index, current);
  }
}
addEventListener('gamepadconnected', event => { gamepadIndex ??= event.gamepad.index; log('Gamepad connected',{index:event.gamepad.index,id:event.gamepad.id,mapping:event.gamepad.mapping}); notify('手把已連線：左搖桿飛行、右搖桿左右轉向、扳機升降。'); });
addEventListener('gamepaddisconnected', event => { gamepadButtons.delete(event.gamepad.index); if(gamepadIndex===event.gamepad.index)gamepadIndex=null; log('Gamepad disconnected',event.gamepad.index); });

async function init() {
  try {
    createWorld();
    $('scenario').replaceChildren(...SCENARIOS.map(item => { const option=document.createElement('option');option.value=item.id;option.textContent=`${item.name} — ${item.condition}`;return option;}));
    const [{default:createUAVSimCore},configResponse]=await Promise.all([import('./generated/uavsim.mjs'),fetch('./configs/generic_quadrotor_x01.json')]);
    if(!configResponse.ok)throw new Error('找不到機型設定。');
    core=await createUAVSimCore({print:text=>log('Core',text),printErr:text=>{
      if(/^INFO:/.test(text))log('Core',text);
      else if(/^ERROR:/.test(text))console.error('[UAVSim core]',text);
      else console.warn('[UAVSim core]',text);
    }});
    check(core.loadVehicle(await configResponse.text()));state=readState();previousState=state;
    $('load-status').textContent='飛行核心已就緒，正在載入指定模型與 6 張貼圖…';
    await loadDrone();ready=true;refreshGates();
    $('load-status').textContent='指定模型與 6 張貼圖已就緒';$('start').innerHTML='開始飛行 <span>→</span>';$('start').disabled=false;$('engine-status').textContent='C++ / WASM · 250 Hz';
    log('Browser flight ready',{core:state.core_version,vehicle:state.vehicle_id});
  }catch(error){fail(error);}
}
init();

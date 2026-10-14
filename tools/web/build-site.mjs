import { cp, mkdir, readFile, stat, writeFile } from 'node:fs/promises';
import { createHash } from 'node:crypto';
import { fileURLToPath } from 'node:url';
import path from 'node:path';

const repositoryRoot = fileURLToPath(new URL('../../', import.meta.url));
const copies = [
  ...['index.html', 'style.css', 'app.js', 'game-logic.js'].map(name => [`web/${name}`, name]),
  ['web/generated', 'generated'], ['web/vendor/three', 'vendor/three'],
  ['local-assets/drone', 'assets/drone'],
  ['configs/vehicles/generic_quadrotor_x01.json', 'configs/generic_quadrotor_x01.json'],
  ...['NOTICES.md', 'README.md', 'eigen/COPYING.README', 'eigen/COPYING.MPL2', 'eigen/COPYING.BSD', 'eigen/COPYING.MINPACK', 'nlohmann/LICENSE.MIT', 'picosha2/LICENSE'].map(name => [`third_party/${name}`, `ThirdPartyNotices/${name}`]),
];
const directories = new Set(['web/generated', 'web/vendor/three', 'local-assets/drone']);
const requiredFiles = [
  ...copies.map(([source]) => source).filter(source => !directories.has(source)),
  'web/generated/uavsim.mjs', 'web/generated/uavsim.wasm',
  // Static import closure of the pinned Three.js 0.186.1 frontend dependencies.
  ...['build/three.module.js', 'build/three.core.js', 'examples/jsm/loaders/FBXLoader.js',
    'examples/jsm/environments/RoomEnvironment.js', 'examples/jsm/libs/fflate.module.js',
    'examples/jsm/curves/NURBSCurve.js', 'examples/jsm/curves/NURBSUtils.js', 'LICENSE']
    .map(name => `web/vendor/three/${name}`),
  ...['source/QuadDrone [mesh].fbx', 'textures/SpaceProbe_[diffuse].png', 'textures/SpaceProbe_[normals].png', 'textures/SpaceProbe_[metal].png', 'textures/SpaceProbe_[gloss].png', 'textures/Spaceprobe_[occlusion].png', 'textures/SpaceProbe_[emission-yellow].png']
    .map(name => `local-assets/drone/${name}`),
];

export async function buildSite(root = repositoryRoot) {
  const output = path.join(root, 'dist/web');
  // Preflight every required resource before modifying the generated site.
  for (const source of directories) {
    if (!(await stat(path.join(root, source))).isDirectory()) throw new Error(`Required directory is not a directory: ${source}`);
  }
  for (const source of requiredFiles) {
    if (!(await stat(path.join(root, source))).isFile()) throw new Error(`Required file is not a regular file: ${source}`);
  }
  await mkdir(output, { recursive: true });
  for (const [source, target] of copies) {
    await mkdir(path.dirname(path.join(output, target)), { recursive: true });
    await cp(path.join(root, source), path.join(output, target), { recursive: true });
  }
  const config = await readFile(path.join(output, 'configs/generic_quadrotor_x01.json'));
  await writeFile(path.join(output, 'build-info.json'), JSON.stringify({ platform: 'browser', physics: 'C++20/WebAssembly', config_sha256: createHash('sha256').update(config).digest('hex'), assets: 'User-provided drone; local evaluation only until redistribution licensing is verified.' }, null, 2));
  console.log(`Browser game prepared: ${output}`);
}

if (process.argv[1] && path.resolve(process.argv[1]) === fileURLToPath(import.meta.url)) {
  try { await buildSite(); }
  catch (error) { console.error('[web] build failed:', error.message); process.exitCode = 1; }
}

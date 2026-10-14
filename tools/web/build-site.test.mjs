import assert from 'node:assert/strict';
import { createHash } from 'node:crypto';
import { mkdir, mkdtemp, readFile, rename, rm, rmdir, writeFile } from 'node:fs/promises';
import { fileURLToPath } from 'node:url';
import path from 'node:path';
import { buildSite } from './build-site.mjs';

const buildDirectory = fileURLToPath(new URL('../../build/', import.meta.url));
await mkdir(buildDirectory, { recursive: true });
const fixture = await mkdtemp(path.join(buildDirectory, 'web-build-test-'));
const entrypoints = [
  'web/generated/uavsim.mjs', 'web/generated/uavsim.wasm',
  ...['build/three.module.js', 'build/three.core.js', 'examples/jsm/loaders/FBXLoader.js',
    'examples/jsm/environments/RoomEnvironment.js', 'examples/jsm/libs/fflate.module.js',
    'examples/jsm/curves/NURBSCurve.js', 'examples/jsm/curves/NURBSUtils.js', 'LICENSE']
    .map(name => `web/vendor/three/${name}`),
];
const sourceFiles = [
  ...['index.html', 'style.css', 'app.js', 'game-logic.js', 'mission-engine.js', 'scenario-definitions.js'].map(name => `web/${name}`),
  'configs/vehicles/generic_quadrotor_x01.json',
  ...['NOTICES.md', 'README.md', 'eigen/COPYING.README', 'eigen/COPYING.MPL2', 'eigen/COPYING.BSD', 'eigen/COPYING.MINPACK', 'nlohmann/LICENSE.MIT', 'picosha2/LICENSE'].map(name => `third_party/${name}`),
  ...['source/QuadDrone [mesh].fbx', 'textures/SpaceProbe_[diffuse].png', 'textures/SpaceProbe_[normals].png', 'textures/SpaceProbe_[metal].png', 'textures/SpaceProbe_[gloss].png', 'textures/Spaceprobe_[occlusion].png', 'textures/SpaceProbe_[emission-yellow].png'].map(name => `local-assets/drone/${name}`),
  ...entrypoints,
];
const output = path.join(fixture, 'dist/web');
const sentinel = 'Existing prepared site must survive failed preflight.';
const unchanged = async () => {
  assert.equal(await readFile(path.join(output, 'index.html'), 'utf8'), sentinel);
  assert.equal(await readFile(path.join(output, 'generated/uavsim.wasm'), 'utf8'), sentinel);
};

try {
  for (const source of sourceFiles) {
    const filename = path.join(fixture, source);
    await mkdir(path.dirname(filename), { recursive: true });
    await writeFile(filename, `Fixture: ${source}`);
  }
  await mkdir(path.join(output, 'generated'), { recursive: true });
  await writeFile(path.join(output, 'index.html'), sentinel);
  await writeFile(path.join(output, 'generated/uavsim.wasm'), sentinel);

  // Existing stale output must not conceal missing compiler/vendor/asset inputs.
  for (const source of [...entrypoints, 'local-assets/drone/textures/SpaceProbe_[diffuse].png']) {
    const filename = path.join(fixture, source);
    await rename(filename, `${filename}.held`);
    await assert.rejects(buildSite(fixture), { code: 'ENOENT' });
    await unchanged();
    await rename(`${filename}.held`, filename);
  }

  // A directory with the expected filename cannot satisfy a resource check.
  const modulePath = path.join(fixture, 'web/generated/uavsim.mjs');
  await rename(modulePath, `${modulePath}.held`);
  await mkdir(modulePath);
  await assert.rejects(buildSite(fixture), /Required file is not a regular file: web\/generated\/uavsim\.mjs/);
  await unchanged();
  await rmdir(modulePath);
  await rename(`${modulePath}.held`, modulePath);

  await buildSite(fixture);
  assert.equal(await readFile(path.join(output, 'index.html'), 'utf8'), 'Fixture: web/index.html');
  assert.equal(await readFile(path.join(output, 'generated/uavsim.wasm'), 'utf8'), 'Fixture: web/generated/uavsim.wasm');
  const config = await readFile(path.join(fixture, 'configs/vehicles/generic_quadrotor_x01.json'));
  const metadata = JSON.parse(await readFile(path.join(output, 'build-info.json'), 'utf8'));
  assert.equal(metadata.config_sha256, createHash('sha256').update(config).digest('hex'));
  console.log('PASS: build preflight rejects missing core/vendor/assets and wrong file types; failed preflight preserves existing site.');
} finally {
  // Delete only this test's unique directory beneath the project-local build tree.
  const resolved = path.resolve(fixture);
  assert.equal(path.dirname(resolved), path.resolve(buildDirectory));
  assert.ok(path.basename(resolved).startsWith('web-build-test-'));
  await rm(resolved, { recursive: true });
}

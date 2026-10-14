import { createHash, randomUUID } from 'node:crypto';
import { mkdir, readFile, writeFile, cp, stat } from 'node:fs/promises';
import { execFileSync } from 'node:child_process';
import { fileURLToPath } from 'node:url';
import path from 'node:path';

const root = fileURLToPath(new URL('../../', import.meta.url));
const version = '0.186.1';
const integrity = 'blFeqb49wRCSGUGj7gtpfnSGHy2lwDk94RhUmS1c/hTby70kvChbWpkJ4Pm1390LqzzvTmzgXKHPEafJwCb8jA==';
const cache = path.join(root, 'build/web-vendor');
const archive = path.join(cache, `three-${version}.tgz`);
await mkdir(cache, { recursive: true });
let bytes;
try { bytes = await readFile(archive); } catch (error) { if (error.code !== 'ENOENT') throw error; }
if (!bytes) {
  const response = await fetch(`https://registry.npmjs.org/three/-/three-${version}.tgz`, { signal: AbortSignal.timeout(120000) });
  if (!response.ok) throw new Error(`Three.js download failed: HTTP ${response.status}`);
  bytes = Buffer.from(await response.arrayBuffer());
}
if (createHash('sha512').update(bytes).digest('base64') !== integrity) throw new Error('Three.js archive integrity mismatch');
await writeFile(archive, bytes);
const entries = execFileSync('tar', ['-tzf', archive], { encoding: 'utf8', maxBuffer: 8 * 1024 * 1024 }).trim().split(/\r?\n/);
if (entries.some(name => !name.startsWith('package/') || name.includes('\\') || name.split('/').includes('..'))) throw new Error('Unsafe archive path');
const details = execFileSync('tar', ['-tvzf', archive], { encoding: 'utf8', maxBuffer: 8 * 1024 * 1024 }).trim().split(/\r?\n/);
if (details.some(line => !['-', 'd'].includes(line[0]))) throw new Error('Archive links or special files are not supported');
const stage = path.join(cache, randomUUID());
await mkdir(stage);
execFileSync('tar', ['-xzf', archive, '-C', stage, 'package/build', 'package/examples/jsm', 'package/LICENSE', 'package/package.json']);
const metadata = JSON.parse(await readFile(path.join(stage, 'package/package.json'), 'utf8'));
if (metadata.version !== version || metadata.license !== 'MIT') throw new Error('Unexpected Three.js package metadata');
const output = path.join(root, 'web/vendor/three');
await mkdir(output, { recursive: true });
for (const name of ['build', 'examples', 'LICENSE', 'package.json']) await cp(path.join(stage, 'package', name), path.join(output, name), { recursive: true });
for (const name of ['build/three.module.js', 'build/three.core.js', 'examples/jsm/loaders/FBXLoader.js']) {
  if (!(await stat(path.join(output, name))).isFile()) throw new Error(`Missing Three.js file: ${name}`);
}
await writeFile(path.join(output, 'provenance.json'), JSON.stringify({ version, source: `https://registry.npmjs.org/three/-/three-${version}.tgz`, integrity: `sha512-${integrity}`, license: 'MIT' }, null, 2));
console.log(`Three.js ${version} prepared; SHA512 verified, MIT license retained.`);

import assert from 'node:assert/strict';
import { mkdtemp, writeFile, rm } from 'node:fs/promises';
import { tmpdir } from 'node:os';
import path from 'node:path';
import http from 'node:http';
import { createStaticServer } from './serve.mjs';

const root = await mkdtemp(path.join(tmpdir(), 'uavsim-web-'));
const server = await createStaticServer(root);
await writeFile(path.join(root, 'index.html'), '<title>UAVSim</title>');
await writeFile(path.join(root, 'core.wasm'), Buffer.from([0, 97, 115, 109]));
await new Promise(resolve => server.listen(0, '127.0.0.1', resolve));
const request = (url, method = 'GET') => new Promise((resolve, reject) => {
  const req = http.request({ hostname: '127.0.0.1', port: server.address().port, path: url, method }, response => {
    let body = ''; response.on('data', chunk => body += chunk); response.on('end', () => resolve({ code: response.statusCode, type: response.headers['content-type'], body }));
  }); req.on('error', reject); req.end();
});
try {
  assert.equal((await request('/')).code, 200);
  assert.equal((await request('/core.wasm')).type, 'application/wasm');
  assert.equal((await request('/', 'HEAD')).body, '');
  assert.equal((await request('/', 'POST')).code, 405);
  assert.equal((await request('/missing.js')).code, 404);
  for (const url of ['/../AGENTS.md', '/%2e%2e/AGENTS.md', '/%5c..%5cAGENTS.md', '/.env']) assert.equal((await request(url)).code, 403);
  assert.equal((await request('/%FF')).code, 400);
  console.log('Static server HTTP/path/MIME regression PASS');
} finally { await new Promise(resolve => server.close(resolve)); await rm(root, { recursive: true }); }

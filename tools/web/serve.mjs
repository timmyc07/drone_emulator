import http from 'node:http';
import path from 'node:path';
import { fileURLToPath } from 'node:url';
import { realpath, stat } from 'node:fs/promises';
import { createReadStream } from 'node:fs';

const types = { '.html': 'text/html; charset=utf-8', '.js': 'text/javascript; charset=utf-8', '.mjs': 'text/javascript; charset=utf-8', '.css': 'text/css; charset=utf-8', '.json': 'application/json', '.wasm': 'application/wasm', '.png': 'image/png', '.fbx': 'application/octet-stream', '.md': 'text/plain; charset=utf-8' };
export async function createStaticServer(directory) {
  const root = await realpath(directory);
  const inside = candidate => candidate === root || candidate.startsWith(root + path.sep);
  return http.createServer(async (request, response) => {
    const reply = (code, message) => { response.writeHead(code, { 'Content-Type': 'text/plain; charset=utf-8', 'X-Content-Type-Options': 'nosniff' }); response.end(request.method === 'HEAD' ? '' : message); };
    if (!['GET', 'HEAD'].includes(request.method)) { response.setHeader('Allow', 'GET, HEAD'); reply(405, 'Method not allowed'); return; }
    try {
      const pathname = decodeURIComponent((request.url ?? '/').split('?')[0]);
      if (!pathname.startsWith('/') || pathname.includes('\\') || pathname.includes('\0') || pathname.split('/').some(part => part === '..' || part.startsWith('.'))) { reply(403, 'Forbidden'); return; }
      const candidate = path.resolve(root, '.' + (pathname === '/' ? '/index.html' : pathname));
      if (!inside(candidate)) { reply(403, 'Forbidden'); return; }
      const filename = await realpath(candidate);
      if (!inside(filename)) { reply(403, 'Forbidden'); return; }
      const info = await stat(filename);
      if (!info.isFile()) { reply(404, 'Not found'); return; }
      const type = types[path.extname(filename)] ?? (path.basename(filename) === 'LICENSE' ? 'text/plain; charset=utf-8' : null);
      if (!type) { reply(403, 'Forbidden'); return; }
      response.writeHead(200, { 'Content-Type': type, 'Content-Length': info.size, 'Cache-Control': 'no-store', 'X-Content-Type-Options': 'nosniff', 'Cross-Origin-Resource-Policy': 'same-origin' });
      if (request.method === 'HEAD') { response.end(); return; }
      const stream = createReadStream(filename);
      stream.on('error', error => { console.error('[web] file read failed', error.code); response.destroy(); });
      stream.pipe(response);
    } catch (error) {
      if (error instanceof URIError) reply(400, 'Malformed URL');
      else if (['ENOENT', 'ENOTDIR'].includes(error.code)) reply(404, 'Not found');
      else { console.error('[web] request failed', error.code ?? error.message); reply(500, 'Server error'); }
    }
  });
}

if (process.argv[1] && path.resolve(process.argv[1]) === fileURLToPath(import.meta.url)) {
  const port = Number(process.argv[2] ?? 4173);
  if (!Number.isInteger(port) || port < 0 || port > 65535) throw new Error('Port must be an integer between 0 and 65535');
  const directory = fileURLToPath(new URL('../../dist/web', import.meta.url));
  const server = await createStaticServer(directory);
  server.on('error', error => { console.error('[web] server failed', error.code); process.exitCode = 1; });
  server.listen(port, '127.0.0.1', () => console.log(`UAVSim browser game: http://127.0.0.1:${server.address().port}`));
  process.on('SIGINT', () => server.close());
  process.on('SIGTERM', () => server.close());
}

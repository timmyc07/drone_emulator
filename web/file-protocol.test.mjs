import assert from 'node:assert/strict';
import { readFile } from 'node:fs/promises';

const index = await readFile(new URL('./index.html', import.meta.url), 'utf8');
const guard = index.indexOf("location.protocol === 'file:'");
const moduleLoad = index.indexOf("app.src = './app.js'");

assert.ok(guard >= 0, 'direct file launch must be handled before module loading');
assert.ok(moduleLoad > guard, 'module loading must be conditional on a supported origin');
assert.match(index, /無法直接開啟此檔案。請在專案目錄執行 node tools\/web\/serve\.mjs，然後開啟 http:\/\/127\.0\.0\.1:4173\//);
assert.match(index, /start\.disabled = true/);
console.log('PASS: direct file launch shows a loopback-server recovery message without loading the application module');

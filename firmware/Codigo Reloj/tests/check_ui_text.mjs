// Offline text-fit checks using the project's actual Montserrat glyph advances.
// This is not a screenshot test or an execution of LVGL/FreeRTOS.
import fs from 'node:fs';
import path from 'node:path';
import assert from 'node:assert/strict';
import {fileURLToPath} from 'node:url';
const root = path.resolve(path.dirname(fileURLToPath(import.meta.url)), '..');
const cache = new Map();
function font(size) {
    if (cache.has(size)) return cache.get(size);
    const source = fs.readFileSync(path.join(root, 'components/lvgl/src/font/lv_font_montserrat_' + size + '.c'), 'utf8');
    const array = name => {
        const body = source.match(new RegExp(name + '\\[\\] = \\{([\\s\\S]*?)\\};'))?.[1];
        assert.ok(body, 'Missing font table: ' + name);
        return body.match(/-?0x[0-9a-f]+|-?\d+/gi).map(Number);
    };
    const body = source.match(/glyph_dsc\[\] = \{([\s\S]*?)\};/)[1];
    const advances = [...body.matchAll(/\.adv_w = (\d+)/g)].map(m => +m[1]);
    const data = {
        advances, unicode: array('unicode_list_1'),
        left: array('kern_left_class_mapping'), right: array('kern_right_class_mapping'),
        kern: array('kern_class_values'),
        rightCount: +source.match(/\.right_class_cnt\s*=\s*(\d+)/)[1],
        scale: +source.match(/\.kern_scale\s*=\s*(\d+)/)[1],
    };
    cache.set(size, data);
    return data;
}
function width(text, size) {
    const f = font(size);
    const glyph = ch => {
        const code = ch.codePointAt(0);
        if (code >= 32 && code <= 126) return code - 31;
        const index = f.unicode.indexOf(code - 176);
        assert.notEqual(index, -1, 'Missing glyph: ' + ch);
        return 96 + index;
    };
    const ids = [...text].map(glyph);
    return ids.reduce((sum, id, i) => {
        const left = f.left[id], right = f.right[ids[i + 1]] || 0;
        const kern = left && right ? f.kern[(left - 1) * f.rightCount + right - 1] : 0;
        return sum + ((f.advances[id] + ((kern * f.scale) >> 4) + 8) >> 4);
    }, 0);
}
// Explicit cases cover representative and worst-case strings for compact regions.
// Keep these synchronized with source labels when changing copy/layout.
const cases = [
    ['row title', 'Pulso', 14, 58], ['row temp title', 'Temp.', 14, 58],
    ['row pulse', '180 BPM', 28, 128], ['row temp', '39.9 \u00b0C', 28, 128],
    ['row sleep', '12h 59m', 28, 128], ['row missing pulse', '-- BPM', 28, 128],
    ['clock', '23:59', 48, 208], ['missing clock', 'Hora sin sincronizar', 14, 208],
    ['date', '31 / 12 / 2026', 14, 208],
    ['demo status', 'Demo', 14, 60], ['battery full', '\uf240 100%', 14, 60],
    ['health age', 'Demo / hace 999 s', 14, 208],
    ['heart stats', 'Min 100   Med 150   Max 180', 14, 208],
    ['thermal main', '39.9', 48, 146], ['heart main', '180', 48, 126],
    ['thermal unit', '\u00b0C', 20, 56], ['heart unit', 'BPM', 20, 76],
    ['thermal detail', '60 muestras / no clinico', 14, 208],
    ['sleep duration', '12h 59m', 48, 208],
    ['sleep phases', '12h 59m', 20, 109],
    ['sleep note', 'Datos de demostracion', 14, 208],
    ['splash', 'NERA', 48, 208],
    ['settings error', 'No se pudo aplicar', 14, 208],
    ['settings menu', 'Bluetooth', 20, 150], ['quick link', '\uf013 Ajustes', 20, 184],
    ['back', 'Volver', 14, 80], ['percentage', '100%', 20, 60],
];
let failed = 0;
for (const [name, text, size, limit] of cases) {
    const pixels = width(text, size);
    const ok = pixels <= limit;
    console.log((ok ? 'PASS' : 'FAIL') + ' ' + name + ': ' + pixels + '/' + limit + ' px');
    if (!ok) failed++;
}
assert.equal(failed, 0, failed + ' text regions overflow');
console.log(cases.length + ' text-fit cases passed. Physical rendering remains untested.');

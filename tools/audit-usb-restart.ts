#!/usr/bin/env bun
/** SCRIPT_JDOC
 * {"summary":"Reproduce generated USB deinitialisation ownership gaps with exact extracted functions and host mocks","keywords":["usb","hid","restart","audit","ownership"],"kind":"mixed","weight":"standard","role":"entrypoint"}
 */
import { mkdirSync, readFileSync, writeFileSync } from "node:fs";
import { resolve } from "node:path";

const root = resolve(import.meta.dir, "..");
process.chdir(root);
const directory = "build/usb-restart-audit";
mkdirSync(directory, { recursive: true });
const selections = [
    ["bootloader/src/config/default/usb/src/usb_device_hid.c", "F_USB_DEVICE_HID_GlobalInitialize", "void"],
    ["bootloader/src/config/default/usb/src/usb_device_hid.c", "F_USB_DEVICE_HID_DeInitialize", "void"],
    ["bootloader/src/config/default/usb/src/usb_device_hid.c", "USB_DEVICE_HID_EventHandlerSet", "USB_DEVICE_HID_RESULT"],
    ["bootloader/src/config/default/usb/src/usb_device.c", "USB_DEVICE_Deinitialize", "void"],
    ["bootloader/src/config/default/driver/usb/usbfs/src/drv_usbfs.c", "DRV_USBFS_Deinitialize", "void"],
];
const extracted: string[] = [];
const manifest = [];
try {
    for (const [path, name, type] of selections) {
        const text = readFileSync(path, "utf8");
        const pattern = new RegExp(`^${type}\\s+${name}\\s*\\([^;]*?\\)\\s*\\{`, "gm");
        const matches = Array.from(text.matchAll(pattern));
        if (matches.length !== 1) throw new Error(`expected one exact definition: ${path} ${name}`);
        const start = matches[0].index!;
        let cursor = start + matches[0][0].length;
        // These selected definitions have no brace-bearing strings/comments.
        // Fail closed if a future source revision adds them.
        let depth = 1;
        for (; cursor < text.length && depth; cursor++) {
            if (text[cursor] === "{") depth++;
            else if (text[cursor] === "}") depth--;
        }
        if (depth) throw new Error(`unclosed ${name}`);
        const body = text.slice(start, cursor);
        if (/"[^"\n]*[{}][^"\n]*"/.test(body)) throw new Error(`brace-bearing string in ${name}`);
        extracted.push(`/* Exact body from ${path}; generated source is NOT modified. */\n${body}\n`);
        const hash = new Bun.CryptoHasher("sha256").update(body).digest("hex");
        manifest.push({ path, name, sha256: hash });
    }
    writeFileSync(`${directory}/functions.inc`, extracted.join("\n"));
    writeFileSync(`${directory}/manifest.json`, JSON.stringify(manifest, null, 2) + "\n");
    const compiler = (process.env.HOST_CC ?? "cc").trim().split(/\s+/);
    const args = [...compiler, "-std=c11", "-O2", "-Wall", "-Wextra", "-Werror", `-I${directory}`,
                  "tests/usb_restart_source_audit.c", "-o", `${directory}/audit`];
    const compile = Bun.spawnSync(args, { stdout: "inherit", stderr: "inherit" });
    if (compile.exitCode !== 0) throw new Error("source audit compilation failed");
    const test = Bun.spawnSync([`${directory}/audit`], { stdout: "inherit", stderr: "inherit" });
    if (test.exitCode !== 0) throw new Error("source ownership evidence no longer matches; review changed stack");
    console.log("PASS: exact selected generated functions audited; this reproduces blockers, NOT target restart approval.");
} catch (error) {
    console.error(String(error));
    process.exit(1);
}

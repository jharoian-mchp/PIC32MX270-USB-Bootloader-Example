#!/usr/bin/env bun
import { spawnSync } from "node:child_process";
import { mkdtempSync, rmSync } from "node:fs";
import { tmpdir } from "node:os";
import { join } from "node:path";

const root = new URL("..", import.meta.url).pathname;
const dir = mkdtempSync(join(tmpdir(), "customer-led-test-"));
try {
  const exe = join(dir, "customer-led-test");
  const cc = process.env.HOST_CC ?? "cc";
  const build = spawnSync(cc, ["-std=c11", "-Wall", "-Wextra", "-Werror", "-pedantic",
    "-I", join(root, "tests/customer_led_mocks"),
    "-I", join(root, "customer-demo/application/src"),
    join(root, "customer-demo/application/src/app_led.c"),
    join(root, "tests/customer-led-test.c"), "-o", exe], { stdio: "inherit" });
  if (build.status !== 0) process.exit(build.status ?? 1);
  const test = spawnSync(exe, [], { stdio: "inherit" });
  if (test.status !== 0) process.exit(test.status ?? 1);
} finally {
  rmSync(dir, { recursive: true, force: true });
}

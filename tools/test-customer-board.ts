#!/usr/bin/env bun
import { spawnSync } from "node:child_process";
import { mkdtempSync, rmSync, readFileSync, writeFileSync } from "node:fs";
import { tmpdir } from "node:os";
import { join } from "node:path";

const root = new URL("..", import.meta.url).pathname;
const dir = mkdtempSync(join(tmpdir(), "customer-board-test-"));
try {
  const exe = join(dir, "board_demo_test");
  const cc = process.env.HOST_CC ?? "cc";
  const build = spawnSync(cc, ["-std=c11", "-Wall", "-Wextra", "-Werror",
    "-I", join(root, "customer-demo/bootloader/src"),
    join(root, "customer-demo/bootloader/src/board_demo.c"),
    join(root, "tests/board_demo_test.c"), "-o", exe], { stdio: "inherit" });
  if (build.status !== 0) process.exit(build.status ?? 1);
  for (const test of ["startup", "timeout", "nack", "mismatch", "adc"]) {
    const run = spawnSync(exe, [test], { stdio: "inherit" });
    if (run.status !== 0) process.exit(run.status ?? 1);
  }
  const source = readFileSync(join(root, 'customer-demo/bootloader/src/config/default/bootloader/datastream/datastream_usb_device_hid.c'), 'utf8');
  const match = source.match(/static void DATASTREAM_BoardAttachTasks\(void\)\n\{[\s\S]*?\n\}/);
  if (!match) throw new Error('missing attach owner');
  writeFileSync(join(dir, 'attach.c'), `#include <stdbool.h>\n#include <assert.h>\n#define USB_DEVICE_HANDLE_INVALID (-1)\nstatic bool boardPowerDetected,boardAttached,cable;\nstatic struct {int usbHandle;} usbHIDData;\nstatic unsigned attaches,detaches;\nstatic bool BOARD_DEMO_CablePresent(void){return cable;}\nstatic void USB_DEVICE_Attach(int h){assert(h==1);attaches++;}\nstatic void USB_DEVICE_Detach(int h){assert(h==1);detaches++;}\n${match[0]}\nint main(void){usbHIDData.usbHandle=-1;boardPowerDetected=cable=true;DATASTREAM_BoardAttachTasks();assert(!attaches);usbHIDData.usbHandle=1;cable=false;DATASTREAM_BoardAttachTasks();assert(!attaches);cable=true;DATASTREAM_BoardAttachTasks();DATASTREAM_BoardAttachTasks();assert(attaches==1);cable=false;DATASTREAM_BoardAttachTasks();DATASTREAM_BoardAttachTasks();assert(detaches==1);cable=true;boardPowerDetected=false;DATASTREAM_BoardAttachTasks();assert(attaches==1);return 0;}\n`);
  const attachExe=join(dir,'attach');
  const attachBuild=spawnSync(cc,['-std=c11','-Wall','-Wextra','-Werror',join(dir,'attach.c'),'-o',attachExe],{stdio:'inherit'});
  if(attachBuild.status!==0) throw new Error('attach compile failed');
  if(spawnSync(attachExe,[],{stdio:'inherit'}).status!==0) throw new Error('attach tests failed');
  console.log('PASS: exact datastream attach gate invalid handle/power/cable/one-shot detach');
  const platform=readFileSync(join(root,'customer-demo/bootloader/src/board_demo_platform.c'),'utf8').replace('#include "config/default/definitions.h"', `#include <stddef.h>\nstatic unsigned count,step=24000,button;\nstatic unsigned ANSELBCLR,TRISBSET,CNPUBCLR,CNPDBCLR;\nstatic unsigned _CP0_GET_COUNT(void){count+=step;return count;}\nstatic unsigned BOOT_REQUEST_Get(void){return button;}\n#define I2C_ERROR_NONE 0\nstatic bool I2C1_WriteRead(unsigned a,unsigned char*t,size_t n,unsigned char*r,size_t z){(void)a;(void)t;(void)n;(void)r;(void)z;return true;}\nstatic bool I2C1_Write(unsigned a,unsigned char*t,size_t n){(void)a;(void)t;(void)n;return true;}\nstatic bool I2C1_IsBusy(void){return false;}\nstatic int I2C1_ErrorGet(void){return 0;}\nstatic void ADC_SamplingStart(void){}\nstatic bool ADC_ResultIsReady(void){return true;}\nstatic unsigned short ADC_ResultGet(void){return 0;}\n`);
  writeFileSync(join(dir,'platform.c'),platform+`\n#include <assert.h>\nint main(void){button=1;assert(!bootloader_Trigger());button=0;assert(bootloader_Trigger());step=0;assert(!bootloader_Trigger());step=24000;count=0xffffff00U;assert(bootloader_Trigger());BOARD_DEMO_PlatformInitialize();BOARD_DEMO_PlatformTasks();return 0;}\n`);
  const platformExe=join(dir,'platform');
  const pb=spawnSync(cc,['-std=c11','-Wall','-Wextra','-Werror','-I',join(root,'customer-demo/bootloader/src'),join(dir,'platform.c'),join(root,'customer-demo/bootloader/src/board_demo.c'),'-o',platformExe],{stdio:'inherit'});
  if(pb.status!==0||spawnSync(platformExe,[],{stdio:'inherit'}).status!==0)throw new Error('platform mock failed');
  console.log('PASS: platform mocked APIs and boot trigger high/low/stalled Count/wrap');
} finally {
  rmSync(dir, { recursive: true, force: true });
}

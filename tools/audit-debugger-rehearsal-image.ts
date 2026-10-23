#!/usr/bin/env bun
/** SCRIPT_JDOC
 * {"summary":"Audit explicit PIC32MX270 RAM rehearsal ELF/HEX and pinned structural policy inputs","keywords":["PIC32MX270","rehearsal","ELF","HEX","audit"],"kind":"mixed","weight":"standard","role":"entrypoint"}
 */
import { runAuditCli } from "./rehearsal-image";

try {
  console.log(JSON.stringify(runAuditCli(), null, 2));
} catch (error) {
  console.error(`unsupported: ${error instanceof Error ? error.message : String(error)}`);
  process.exitCode = 1;
}

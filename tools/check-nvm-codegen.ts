#!/usr/bin/env bun
/** SCRIPT_JDOC
 * {"summary":"Compile PIC32MX270 NVM launch probes and check ordered unlock stores in XC32 object code","keywords":["nvm","xc32","unlock","disassembly"],"kind":"mixed","weight":"standard","role":"entrypoint"}
 */
import { mkdirSync, writeFileSync } from "node:fs";
import { resolve } from "node:path";

type Instruction = { address: number; op: string; args: string; relocation?: string };
type Value = number | string;
function verify(text: string): void {
    const instructions: Instruction[] = [];
    for (const line of text.split("\n")) {
        const instruction = line.match(/^\s*([0-9a-f]+):\s+[0-9a-f]{8}\s+([a-z0-9.]+)\s*(.*)$/i);
        if (instruction) {
            instructions.push({ address: parseInt(instruction[1], 16), op: instruction[2], args: instruction[3].trim() });
            continue;
        }
        const relocation = line.match(/^\s*([0-9a-f]+):\s+R_MIPS_(?:HI16|LO16)\s+(\w+)$/);
        if (relocation) {
            const last = instructions.at(-1);
            if (last?.address === parseInt(relocation[1], 16)) last.relocation = relocation[2];
        }
    }
    // This target places both registers in the same signed-LO16 high region.
    // GCC may share their LUI. Keep the real target map to validate that reuse.
    const addresses: Record<string, number> = { NVMKEY: 0xBF80F410, NVMCONSET: 0xBF80F408 };
    const high = (symbol: string): number => {
        if (!(symbol in addresses)) throw new Error(`unknown register relocation ${symbol}`);
        return ((addresses[symbol] + 0x8000) & 0xFFFF0000) >>> 0;
    };
    const registers = new Map<string, Value>([["zero", 0]]);
    const stores: { index: number; address: number; value: Value | undefined; target: Value | undefined }[] = [];
    function immediate(value: string): number { return Number(value); }
    for (const [index, ins] of instructions.entries()) {
        const a = ins.args.split(",").map(s => s.trim());
        if (ins.op === "lui") {
            if (ins.relocation) {
                if (immediate(a[1]) !== 0) throw new Error("nonzero HI16 relocation addend");
                registers.set(a[0], `hi:${ins.relocation}`);
            } else registers.set(a[0], (immediate(a[1]) << 16) >>> 0);
        }
        else if (ins.op === "li") registers.set(a[0], immediate(a[1]) >>> 0);
        else if (ins.op === "move") {
            const value = registers.get(a[1]);
            if (value === undefined) registers.delete(a[0]); else registers.set(a[0], value);
        } else if (ins.op === "ori" || ins.op === "addiu") {
            const previous = registers.get(a[1]);
            if (ins.relocation) {
                if (ins.op !== "addiu" || immediate(a[2]) !== 0 ||
                    typeof previous !== "string" || !previous.startsWith("hi:") ||
                    high(previous.slice(3)) !== high(ins.relocation))
                    throw new Error("nonzero LO16 addend or incompatible HI16/LO16 pair");
                registers.set(a[0], ins.relocation);
            } else if (typeof previous === "number") registers.set(a[0],
                (ins.op === "ori" ? previous | immediate(a[2]) : previous + immediate(a[2])) >>> 0);
            else registers.delete(a[0]);
        } else if (ins.op === "sw") {
            const target = a[1]?.match(/^0\((\w+)\)$/);
            stores.push({ index, address: ins.address, value: registers.get(a[0]),
                          target: target ? registers.get(target[1]) : undefined });
        } else if (!["nop", "jr", "j", "b", "beq", "bne"].includes(ins.op)) {
            // Fail closed on an unmodelled instruction changing a launch operand.
            registers.delete(a[0]);
        }
    }
    const starts = stores.filter(s => s.value === 0 && s.target === "NVMKEY");
    if (starts.length !== 1) throw new Error("expected exactly one zero->NVMKEY launch start");
    const first = stores.indexOf(starts[0]);
    const window = stores.slice(first, first + 4);
    const expectedValues = [0, 0xAA996655, 0x556699AA, 0x8000];
    const expectedTargets = ["NVMKEY", "NVMKEY", "NVMKEY", "NVMCONSET"];
    if (window.length !== 4 || window.some((s, i) =>
        s.value !== expectedValues[i] || s.target !== expectedTargets[i] ||
        s.index !== window[0].index + i || s.address !== window[0].address + 4 * i))
        throw new Error("unlock must be four consecutive stores: 0/key1/key2 -> NVMKEY, WR -> NVMCONSET");
}
function selftest(): void {
    const fixture = `
  0: 3c020000 lui v0,0x0
     0: R_MIPS_HI16 NVMKEY
  4: 24420000 addiu v0,v0,0
     4: R_MIPS_LO16 NVMKEY
  8: 3c030000 lui v1,0x0
     8: R_MIPS_HI16 NVMCONSET
  c: 24630000 addiu v1,v1,0
     c: R_MIPS_LO16 NVMCONSET
 10: 3c04aa99 lui a0,0xaa99
 14: 34846655 ori a0,a0,0x6655
 18: 3c055566 lui a1,0x5566
 1c: 34a599aa ori a1,a1,0x99aa
 20: 34068000 li a2,0x8000
 24: ac400000 sw zero,0(v0)
 28: ac440000 sw a0,0(v0)
 2c: ac450000 sw a1,0(v0)
 30: ac660000 sw a2,0(v1)
`;
    verify(fixture);
    // GCC -O2 legitimately reuses NVMKEY's high region for NVMCONSET.
    const sharedHigh = fixture.replace("8: 3c030000 lui v1,0x0\n     8: R_MIPS_HI16 NVMCONSET\n", "")
        .replace("addiu v0,v0,0", "addiu a3,v0,0")
        .replaceAll("0(v0)", "0(a3)")
        .replace("addiu v1,v1,0", "addiu v1,v0,0");
    verify(sharedHigh);
    for (const bad of [fixture.replace("0x6655", "0x6654"), fixture.replace("sw a2,0(v1)", "sw a2,0(v0)"),
        fixture.replace("28: ac440000", "26: 00000000 nop\n 28: ac440000"),
        fixture.replace("28: ac440000 sw a0,0(v0)", "28: afa40000 sw a0,0(sp)"),
        fixture.replace("2c: ac450000 sw a1,0(v0)", "2c: ac450000 sw a0,0(v0)"),
        fixture.replace("addiu v0,v0,0", "addiu v0,v0,4"),
        fixture.replace("lui v0,0x0", "lui v0,0x1"),
        fixture.replace("4: R_MIPS_LO16 NVMKEY", "4: R_MIPS_LO16 NVMCONSET")]) {
        let rejected = false;
        try { verify(bad); } catch { rejected = true; }
        if (!rejected) throw new Error("checker accepted malformed unlock fixture");
    }
    console.log("PASS: checker rejects bad keys/target/interleaving/spills/order/relocation addends and pairs");
}
function run(args: string[]): string {
    const result = Bun.spawnSync(args, { stdout: "pipe", stderr: "pipe" });
    if (result.exitCode !== 0) throw new Error(`${args.join(" ")}\n${result.stderr.toString()}`);
    return result.stdout.toString();
}
try {
    selftest();
    if (process.argv.includes("--selftest")) process.exit(0);
    const root = resolve(import.meta.dir, "..");
    process.chdir(root);
    const xc32 = process.env.XC32_ROOT ?? "/workspace/ChrisK/tools/xc32/v6.00";
    const dfp = process.env.PIC32MX_DFP_ROOT ?? "/workspace/ChrisK/tools/packs/Microchip/PIC32MX_DFP/1.7.380";
    const registers = await Bun.file(`${dfp}/xc32/32MX270F256D/p32MX270F256D.S`).text();
    for (const [symbol, address] of [["NVMKEY", "BF80F410"], ["NVMCONSET", "BF80F408"]])
        if (!new RegExp(`^${symbol} = 0x${address}$`, "mi").test(registers))
            throw new Error(`target DFP ${symbol} address differs from verifier map`);
    const directory = "build/nvm-codegen";
    mkdirSync(directory, { recursive: true });
    const probe = `${directory}/probe.c`;
    writeFileSync(probe, '#include "update_nvm_port.h"\nvoid nvm_launch_probe(void){UPDATE_NvmPortLaunch();}\n');
    for (const opt of ["0", "1", "2", "s"]) for (const debug of [false, true]) {
        const name = `O${opt}-${debug ? "debug" : "production"}`;
        const object = `${directory}/${name}.o`;
        const flags = ["-mprocessor=32MX270F256D", `-mdfp=${dfp}`, `-O${opt}`, "-std=c11", "-Wall", "-Wextra", "-Werror",
                       "-Ibootloader/src", "-Ibootloader/src/config/default", ...(debug ? ["-g", "-D__DEBUG"] : [])];
        run([`${xc32}/bin/xc32-gcc`, ...flags, "-c", probe, "-o", object]);
        const disassembly = run([`${xc32}/bin/xc32-objdump`, "-dr", object]);
        writeFileSync(`${directory}/${name}.txt`, disassembly);
        verify(disassembly);
        // Strictly compile the real caller too; no target code is executed.
        run([`${xc32}/bin/xc32-gcc`, ...flags, "-c", "bootloader/src/update_nvm.c", "-o", `${directory}/${name}-adapter.o`]);
        console.log(`PASS: ${name}: consecutive target unlock stores, strict adapter compilation`);
    }
    console.log("No target programmed; object checks do not qualify hardware timing or USB bus exclusion.");
} catch (error) {
    console.error(String(error));
    process.exit(1);
}

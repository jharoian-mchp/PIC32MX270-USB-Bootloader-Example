# Toolchain checks only; actual firmware build targets follow MCC generation.
XC32_ROOT ?= /workspace/ChrisK/tools/xc32/v6.00
PIC32MX_DFP_ROOT ?= /workspace/ChrisK/tools/packs/Microchip/PIC32MX_DFP/1.7.380
SMOKE_DIR := build/toolchain-smoke

.PHONY: toolchain-check

toolchain-check:
	@mkdir -p "$(SMOKE_DIR)"
	"$(XC32_ROOT)/bin/xc32-gcc" --version
	"$(XC32_ROOT)/bin/xc32-gcc" -mprocessor=32MX270F256D -mdfp="$(PIC32MX_DFP_ROOT)" -O2 -Wall -Wextra -Werror tools/toolchain-smoke.c -Wl,-Map="$(SMOKE_DIR)/toolchain-smoke.map" -o "$(SMOKE_DIR)/toolchain-smoke.elf"
	"$(XC32_ROOT)/bin/xc32-bin2hex" "$(SMOKE_DIR)/toolchain-smoke.elf"
	"$(XC32_ROOT)/bin/xc32-size" "$(SMOKE_DIR)/toolchain-smoke.elf"
	"$(XC32_ROOT)/bin/xc32-objdump" -f "$(SMOKE_DIR)/toolchain-smoke.elf"
	@test -s "$(SMOKE_DIR)/toolchain-smoke.hex"
	@echo "PASS: XC32 compile, link and HEX generation. Do not program this toolchain-only test image."

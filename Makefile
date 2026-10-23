# Generated application builds and user-owned code checks.
XC32_ROOT ?= /workspace/ChrisK/tools/xc32/v6.00
PIC32MX_DFP_ROOT ?= /workspace/ChrisK/tools/packs/Microchip/PIC32MX_DFP/1.7.380
SMOKE_DIR := build/toolchain-smoke

APP_DIR := build/application
APP_SOURCES := application/src/main.c application/src/app_led.c \
 application/src/config/default/initialization.c application/src/config/default/interrupts.c \
 application/src/config/default/exceptions.c application/src/config/default/stdio/xc32_monitor.c \
 application/src/config/default/peripheral/i2c/plib_i2c_smbus_common.c \
 application/src/config/default/peripheral/clk/plib_clk.c \
 application/src/config/default/peripheral/evic/plib_evic.c \
 application/src/config/default/peripheral/gpio/plib_gpio.c \
 application/src/config/default/peripheral/i2c/master/plib_i2c1_master.c \
 application/src/config/default/peripheral/tmr/plib_tmr2.c
HOST_CC ?= cc
PYTHON ?= python3
# Extra diagnostics beyond the default Windows MPLAB X build flags.
APP_WARNINGS ?= -Wall -Wextra
BTL_DIR := build/bootloader
BTL_LD ?= bootloader/src/config/default/btl.ld
# Isolated target preflight/port is compile-only, never a diagnostic input.
BTL_SOURCES := $(filter-out bootloader/src/isolated_nvm_target.c,$(sort $(shell find bootloader/src -name '*.c')))
BTL_WARNINGS ?= -Wall -Wextra

.PHONY: debugger-rehearsal-source-check debugger-rehearsal-image-test debugger-rehearsal-check toolchain-check nvm-codegen-check isolated-nvm-check usb-restart-audit usb-retirement-check usb-cancellation-check bootloader-restart-check restart-image-audit application bootloader test test-sanitize update-audit

restart-image-audit: bootloader bootloader-restart-check
	XC32_ROOT="$(XC32_ROOT)" bun tools/audit-restart-image.ts

# Temporary expanded read-only diagnostic. Existing app flash may overlap;
# separate explicit programming approval/backup is required before delivery.
bootloader-power-debug:
	@mkdir -p build/power-debug
	@sed '/kseg0_program_mem.*ORIGIN = 0x9D000000/s/LENGTH = 32768/LENGTH = 49152/' bootloader/src/config/default/btl.ld >build/power-debug/btl-debug.ld
	@grep -q 'LENGTH = 49152' build/power-debug/btl-debug.ld
	$(MAKE) bootloader BTL_DIR=build/power-debug BTL_LD=build/power-debug/btl-debug.ld BTL_WARNINGS='-Wall -Wextra -Os -flto -DUSB_RETIREMENT_HOOKS=1 -DUSB_RESTART_DIAGNOSTIC=1 -DUSB_RESTART_DEBUG_LAYOUT=1 -include bootloader/src/usb_restart_debug_layout.h -Wl,--undefined=USB_RestartOwnerRequest,--undefined=USB_RestartOwnerAck'
	@$(XC32_ROOT)/bin/xc32-nm build/power-debug/bootloader.elf >build/power-debug/symbols.txt
	@! grep -Eq 'UPDATE_(Session|Dispatch|LinkDispatch|Lifecycle|Flash|Nvm)|NVM_(RowWrite|PageErase|WordWrite)|ISOLATED_Nvm|run_Application' build/power-debug/symbols.txt

power-debug-image-audit: bootloader bootloader-power-debug
	XC32_ROOT="$(XC32_ROOT)" bun tools/audit-restart-image.ts build/power-debug/bootloader.elf --power-debug

# Compile/link evidence only, NOT a hardware-delivery or update-enabled image.
# Coalesce opt-in data sections to reduce XC32 .dinit/alignment overhead. Keep
# function GC and LTO; audit retained placement and excluded symbols after link.
# Normal recovery and the historical 48KB debug target retain their own flags.
bootloader-restart-check:
	$(MAKE) bootloader BTL_DIR=build/restart-check BTL_WARNINGS="-Wall -Wextra -Os -flto -fno-data-sections -DUSB_RETIREMENT_HOOKS=1 -DUSB_RESTART_DIAGNOSTIC=1 -Wl,--undefined=USB_RestartOwnerRequest,--undefined=USB_RestartOwnerAck"
	@"$(XC32_ROOT)/bin/xc32-nm" build/restart-check/bootloader.elf > build/restart-check/symbols.txt
	@grep -q 'USB_RestartOwnerRequest' build/restart-check/symbols.txt
	@grep -q 'USB_RestartOwnerAck' build/restart-check/symbols.txt
	@! grep -Eq 'UPDATE_(Session|Dispatch|LinkDispatch|Lifecycle|Flash|Nvm)|NVM_(RowWrite|PageErase)|ISOLATED_Nvm' build/restart-check/symbols.txt
	@echo "PASS: single USB owner links within unchanged linker reservation; no hardware delivery/host restart command."


usb-cancellation-check:
	bun tools/usb-retirement-overlay.ts --check
	HOST_CC="$(HOST_CC)" bun tools/test-usb-cancellation.ts

usb-retirement-check:
	bun tools/usb-retirement-overlay.ts --check
	@mkdir -p build/retirement-hooks
	@set -e; for f in bootloader/src/usb_retirement.c bootloader/src/usb_restart_owner.c bootloader/src/config/default/initialization.c bootloader/src/config/default/usb/src/usb_device.c bootloader/src/config/default/usb/src/usb_device_hid.c bootloader/src/config/default/driver/usb/usbfs/src/drv_usbfs.c bootloader/src/config/default/driver/usb/usbfs/src/drv_usbfs_device.c; do \
	"$(XC32_ROOT)/bin/xc32-gcc" -mprocessor=32MX270F256D -mdfp="$(PIC32MX_DFP_ROOT)" -O2 -DUSB_RETIREMENT_HOOKS=1 -Ibootloader/src -Ibootloader/src/config/default -c "$$f" -o "build/retirement-hooks/$$(basename "$$f" .c).o"; done
	HOST_CC="$(HOST_CC)" bun tools/test-usb-retirement.ts

# Reproduces selected generated restart ownership gaps; not target approval.
usb-restart-audit:
	HOST_CC="$(HOST_CC)" bun tools/audit-usb-restart.ts

# Compile-only inactive isolated owner and target; no linked image or page selection.
isolated-nvm-check:
	@mkdir -p build/isolated-nvm
	"$(XC32_ROOT)/bin/xc32-gcc" -mprocessor=32MX270F256D -mdfp="$(PIC32MX_DFP_ROOT)" -std=c11 -Os -Wall -Wextra -Werror -Ibootloader/src -c bootloader/src/isolated_nvm.c -o build/isolated-nvm/owner-xc32.o
	"$(XC32_ROOT)/bin/xc32-gcc" -mprocessor=32MX270F256D -mdfp="$(PIC32MX_DFP_ROOT)" -std=c11 -Os -Wall -Wextra -Werror -Ibootloader/src -c bootloader/src/isolated_nvm_target.c -o build/isolated-nvm/target-xc32.o
	HOST_CC="$(HOST_CC)" bun tools/test-isolated-nvm.ts

# Separate RAM-only library, compile-only real reads; no main/configuration/image.
debugger-rehearsal-check:
	@mkdir -p build/debugger-rehearsal
	"$(XC32_ROOT)/bin/xc32-gcc" -mprocessor=32MX270F256D -mdfp="$(PIC32MX_DFP_ROOT)" -std=c11 -Os -g -Wall -Wextra -Werror -Irehearsal/src -c rehearsal/src/debugger_rehearsal.c -o build/debugger-rehearsal/rehearsal-xc32.o
	HOST_CC="$(HOST_CC)" XC32_ROOT="$(XC32_ROOT)" bun tools/test-debugger-rehearsal.ts

# Standalone sources compile ONLY; no linker, HEX conversion or IDE binding.
debugger-rehearsal-source-check: debugger-rehearsal-check
	@set -e; for f in main.c configuration.c exception_stops.S; do \
	"$(XC32_ROOT)/bin/xc32-gcc" -mprocessor=32MX270F256D -mdfp="$(PIC32MX_DFP_ROOT)" -std=c11 -Os -g -Wall -Wextra -Werror -Irehearsal/src -c "rehearsal/src/$$f" -o "build/debugger-rehearsal/$${f%.*}-xc32.o"; done
	HOST_CC="$(HOST_CC)" XC32_ROOT="$(XC32_ROOT)" bun tools/test-debugger-rehearsal-entry.ts

# Synthetic parser fixtures only: no target link or hardware tools.
debugger-rehearsal-image-test:
	bun tools/test-debugger-rehearsal-image.ts

nvm-codegen-check:
	XC32_ROOT="$(XC32_ROOT)" PIC32MX_DFP_ROOT="$(PIC32MX_DFP_ROOT)" bun tools/check-nvm-codegen.ts

bootloader:
	@mkdir -p "$(BTL_DIR)"
	"$(XC32_ROOT)/bin/xc32-gcc" -mprocessor=32MX270F256D -mdfp="$(PIC32MX_DFP_ROOT)" -O2 -fno-common -ffunction-sections -fdata-sections $(BTL_WARNINGS) -Ibootloader/src -Ibootloader/src/config/default $(BTL_SOURCES) -Wl,--script=$(BTL_LD),--defsym=_min_heap_size=512,--gc-sections,-Map="$(BTL_DIR)/bootloader.map" -o "$(BTL_DIR)/bootloader.elf"
	"$(XC32_ROOT)/bin/xc32-bin2hex" "$(BTL_DIR)/bootloader.elf"
	"$(XC32_ROOT)/bin/xc32-size" "$(BTL_DIR)/bootloader.elf"
	@test -s "$(BTL_DIR)/bootloader.hex"
	@echo "Built enumeration-only diagnostics: no firmware-update tasks. Hardware test instructions: docs/usb-enumeration.md."

application:
	@mkdir -p "$(APP_DIR)"
	"$(XC32_ROOT)/bin/xc32-gcc" -mprocessor=32MX270F256D -mdfp="$(PIC32MX_DFP_ROOT)" -O1 -fno-common -ffunction-sections -fdata-sections $(APP_WARNINGS) -Iapplication/src -Iapplication/src/config/default $(APP_SOURCES) -Wl,--script=application/src/config/default/p32MX270F256D.ld,--defsym=_min_heap_size=512,--gc-sections,-Map="$(APP_DIR)/app.map" -o "$(APP_DIR)/app.elf"
	"$(XC32_ROOT)/bin/xc32-bin2hex" "$(APP_DIR)/app.elf"
	"$(XC32_ROOT)/bin/xc32-size" "$(APP_DIR)/app.elf"
	@test -s "$(APP_DIR)/app.hex"

test:
	@mkdir -p build/tests
	HOST_CC="$(HOST_CC)" bun tools/test-debugger-rehearsal.ts
	HOST_CC="$(HOST_CC)" bun tools/test-debugger-rehearsal-entry.ts
	bun tools/test-debugger-rehearsal-image.ts
	HOST_CC="$(HOST_CC)" bun tools/test-isolated-nvm.ts
	$(HOST_CC) -std=c11 -O2 -Wall -Wextra -Werror -Ibootloader/src bootloader/src/update_wire.c tests/usb_debug_layout_test.c -o build/tests/usb_normal_layout_test
	./build/tests/usb_normal_layout_test
	$(HOST_CC) -std=c11 -O2 -Wall -Wextra -Werror -DUSB_RETIREMENT_HOOKS=1 -DUSB_RESTART_DIAGNOSTIC=1 -DUSB_RESTART_DEBUG_LAYOUT=1 -Ibootloader/src -include bootloader/src/usb_restart_debug_layout.h bootloader/src/update_wire.c tests/usb_debug_layout_test.c -o build/tests/usb_debug_layout_test
	./build/tests/usb_debug_layout_test
	$(HOST_CC) -std=c11 -O2 -Wall -Wextra -Werror -Itests/mocks -Iapplication/src application/src/app_led.c tests/app_led_test.c -o build/tests/app_led_test
	./build/tests/app_led_test
	$(HOST_CC) -std=c11 -O2 -Wall -Wextra -Werror -Itests/usb_mocks -Ibootloader/src bootloader/src/board_usb_diag.c tests/board_usb_diag_test.c -o build/tests/board_usb_diag_test
	./build/tests/board_usb_diag_test
	$(HOST_CC) -std=c11 -O2 -Wall -Wextra -Werror -DUSB_RETIREMENT_HOOKS=1 -DUSB_RESTART_DIAGNOSTIC=1 -Itests/usb_owner_mocks -Itests/usb_mocks -Ibootloader/src bootloader/src/board_usb_diag.c tests/board_usb_diag_test.c -o build/tests/board_owner_handover_test
	./build/tests/board_owner_handover_test
	$(HOST_CC) -std=c11 -O2 -Wall -Wextra -Werror -Ibootloader/src -Itests/usb_mocks bootloader/src/usb_probe.c bootloader/src/update_wire.c bootloader/src/update_protocol.c tests/usb_probe_test.c -o build/tests/usb_probe_test
	./build/tests/usb_probe_test
	$(HOST_CC) -std=c11 -O2 -Wall -Wextra -Werror -DUSB_RETIREMENT_HOOKS=1 -DUSB_RESTART_DIAGNOSTIC=1 -Itests/usb_owner_mocks -Ibootloader/src -Itests/usb_mocks bootloader/src/usb_probe.c bootloader/src/update_wire.c bootloader/src/update_protocol.c tests/usb_probe_test.c -o build/tests/usb_pause_test
	./build/tests/usb_pause_test
	$(HOST_CC) -std=c11 -O2 -Wall -Wextra -Werror -DUSB_RETIREMENT_HOOKS=1 -DUSB_RESTART_DIAGNOSTIC=1 -Itests/usb_owner_mocks -Ibootloader/src -Itests/usb_mocks bootloader/src/usb_probe.c bootloader/src/update_wire.c bootloader/src/update_protocol.c bootloader/src/usb_restart_owner.c bootloader/src/usb_restart.c bootloader/src/usb_restart_journal.c tests/usb_pause_owner_test.c -o build/tests/usb_pause_owner_test
	./build/tests/usb_pause_owner_test
	./build/tests/usb_pause_owner_test failed-ack
	./build/tests/usb_pause_owner_test expired-ack
	./build/tests/usb_pause_owner_test reset-wait
	$(HOST_CC) -std=c11 -O2 -Wall -Wextra -Werror -Ibootloader/src bootloader/src/usb_restart.c tests/usb_restart_test.c -o build/tests/usb_restart_test
	./build/tests/usb_restart_test
	$(HOST_CC) -std=c11 -O2 -Wall -Wextra -Werror -Ibootloader/src bootloader/src/usb_restart_journal.c tests/usb_restart_journal_test.c -o build/tests/usb_restart_journal_test
	./build/tests/usb_restart_journal_test
	$(HOST_CC) -std=c11 -O2 -Wall -Wextra -Werror -Ibootloader/src bootloader/src/usb_irp_ledger.c tests/usb_irp_ledger_test.c -o build/tests/usb_irp_ledger_test
	./build/tests/usb_irp_ledger_test
	$(HOST_CC) -std=c11 -O2 -Wall -Wextra -Werror -DUSB_RETIREMENT_HOOKS=1 -Itests/usb_owner_mocks -Itests/usb_mocks -Ibootloader/src bootloader/src/usb_restart_owner.c bootloader/src/usb_restart.c bootloader/src/usb_restart_journal.c tests/usb_restart_owner_test.c -o build/tests/usb_restart_owner_test
	./build/tests/usb_restart_owner_test
	./build/tests/usb_restart_owner_test error
	./build/tests/usb_restart_owner_test ack-reset
	./build/tests/usb_restart_owner_test failed-ack
	./build/tests/usb_restart_owner_test expired-ack
	./build/tests/usb_restart_owner_test reset-wait
	./build/tests/usb_restart_owner_test off-failure
	$(HOST_CC) -std=c11 -O2 -Wall -Wextra -Werror -DUSB_RETIREMENT_HOOKS=1 -DUSB_RESTART_DIAGNOSTIC=1 -DUSB_RESTART_OWNER_INTERNAL_TEST=1 -Itests/usb_owner_mocks -Itests/usb_mocks -Ibootloader/src bootloader/src/usb_restart.c bootloader/src/usb_restart_journal.c tests/usb_restart_owner_test.c -o build/tests/usb_owner_checkpoint_test
	./build/tests/usb_owner_checkpoint_test
	$(HOST_CC) -std=c11 -O2 -Wall -Wextra -Werror -Ibootloader/src bootloader/src/update_protocol.c tests/update_protocol_test.c -o build/tests/update_protocol_test
	./build/tests/update_protocol_test
	$(HOST_CC) -std=c11 -O2 -Wall -Wextra -Werror -Ibootloader/src bootloader/src/update_protocol.c bootloader/src/update_hex.c bootloader/src/update_flash.c tests/update_storage_test.c -o build/tests/update_storage_test
	./build/tests/update_storage_test
	$(HOST_CC) -std=c11 -O2 -Wall -Wextra -Werror -Ibootloader/src bootloader/src/update_protocol.c bootloader/src/update_hex.c bootloader/src/update_flash.c bootloader/src/update_image.c bootloader/src/update_session.c tests/update_session_test.c -o build/tests/update_session_test
	./build/tests/update_session_test
	$(HOST_CC) -std=c11 -O2 -Wall -Wextra -Werror -Ibootloader/src bootloader/src/update_protocol.c tests/vendor_host_vectors_test.c -o build/tests/vendor_host_vectors_test
	./build/tests/vendor_host_vectors_test
	$(HOST_CC) -std=c11 -O2 -Wall -Wextra -Werror -Ibootloader/src bootloader/src/update_wire.c tests/update_wire_test.c -o build/tests/update_wire_test
	./build/tests/update_wire_test
	$(HOST_CC) -std=c11 -O2 -Wall -Wextra -Werror -Ibootloader/src bootloader/src/update_protocol.c bootloader/src/update_hex.c bootloader/src/update_flash.c bootloader/src/update_image.c bootloader/src/update_session.c bootloader/src/update_wire.c bootloader/src/update_dispatch.c tests/update_dispatch_test.c -o build/tests/update_dispatch_test
	./build/tests/update_dispatch_test
	$(HOST_CC) -std=c11 -O2 -Wall -Wextra -Werror -Itests/nvm_mocks -Ibootloader/src bootloader/src/update_nvm.c bootloader/src/update_flash.c tests/update_nvm_test.c -o build/tests/update_nvm_test
	./build/tests/update_nvm_test
	$(HOST_CC) -std=c11 -O2 -Wall -Wextra -Werror -Ibootloader/src bootloader/src/update_protocol.c bootloader/src/update_hex.c bootloader/src/update_flash.c bootloader/src/update_image.c bootloader/src/update_session.c bootloader/src/update_lifecycle.c tests/update_lifecycle_test.c -o build/tests/update_lifecycle_test
	./build/tests/update_lifecycle_test
	$(HOST_CC) -std=c11 -O2 -Wall -Wextra -Werror -Ibootloader/src bootloader/src/update_protocol.c bootloader/src/update_flash.c bootloader/src/update_image.c bootloader/src/update_lifecycle_wire.c tests/update_lifecycle_wire_test.c -o build/tests/update_lifecycle_wire_test
	./build/tests/update_lifecycle_wire_test
	$(HOST_CC) -std=c11 -O2 -Wall -Wextra -Werror -Ibootloader/src bootloader/src/update_protocol.c bootloader/src/update_hex.c bootloader/src/update_flash.c bootloader/src/update_image.c bootloader/src/update_session.c bootloader/src/update_lifecycle.c bootloader/src/update_lifecycle_wire.c bootloader/src/update_link_dispatch.c tests/update_link_dispatch_test.c -o build/tests/update_link_dispatch_test
	./build/tests/update_link_dispatch_test
	PYTHONDONTWRITEBYTECODE=1 $(PYTHON) -m unittest discover -s host -v
	bun tools/check-nvm-codegen.ts --selftest
	HOST_CC="$(HOST_CC)" bun tools/audit-usb-restart.ts
	bun tools/usb-retirement-overlay.ts --check
	HOST_CC="$(HOST_CC)" bun tools/test-usb-retirement.ts
	HOST_CC="$(HOST_CC)" bun tools/test-usb-cancellation.ts
	HOST_CC="$(HOST_CC)" bun tools/test-usb-hid-submission.ts
	HOST_CC="$(HOST_CC)" bun tools/test-usb-rebuild.ts

# Rebuild all mock tests with sanitizers, including Python-plan -> C integration.
test-sanitize:
	$(MAKE) test HOST_CC="$(HOST_CC) -g -fsanitize=address,undefined -fno-omit-frame-pointer -fno-pie -no-pie"

# A passing audit means known defects reproduced, not that updates are safe.
update-audit:
	HOST_CC="$(HOST_CC)" sh tests/run-update-audit.sh

toolchain-check:
	@mkdir -p "$(SMOKE_DIR)"
	"$(XC32_ROOT)/bin/xc32-gcc" --version
	"$(XC32_ROOT)/bin/xc32-gcc" -mprocessor=32MX270F256D -mdfp="$(PIC32MX_DFP_ROOT)" -O2 -Wall -Wextra -Werror tools/toolchain-smoke.c -Wl,-Map="$(SMOKE_DIR)/toolchain-smoke.map" -o "$(SMOKE_DIR)/toolchain-smoke.elf"
	"$(XC32_ROOT)/bin/xc32-bin2hex" "$(SMOKE_DIR)/toolchain-smoke.elf"
	"$(XC32_ROOT)/bin/xc32-size" "$(SMOKE_DIR)/toolchain-smoke.elf"
	"$(XC32_ROOT)/bin/xc32-objdump" -f "$(SMOKE_DIR)/toolchain-smoke.elf"
	@test -s "$(SMOKE_DIR)/toolchain-smoke.hex"
	@echo "PASS: XC32 compile, link and HEX generation. Do not program this toolchain-only test image."

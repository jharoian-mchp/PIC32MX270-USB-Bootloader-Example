#!/bin/sh
# Exact generated-code safety reproductions. All peripheral/flash APIs are mocked.
set -eu
cd "$(dirname "$0")/.."
ulimit -c 0
mkdir -p build/update-audit
compiler=${HOST_CC:-cc}
flags='-std=c11 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -fno-pie -no-pie -Wall -Wextra -Wno-unused-parameter'
# Deliberately compile vendor code without -Werror so existing diagnostics do not
# conceal the memory-safety tests. No generated source is changed.
"$compiler" $flags -Itests/update_mocks -Ibootloader/src/config/default tests/update_nvm_audit.c -o build/update-audit/nvm-audit
"$compiler" $flags -Itests/update_mocks -Ibootloader/src/config/default tests/update_protocol_audit.c -o build/update-audit/protocol-audit
build/update-audit/nvm-audit valid
build/update-audit/protocol-audit valid
for case in upper-end cross-end write-error erase-bounds ignored-region trailing; do
    build/update-audit/nvm-audit "$case"
done
for case in split-dle program-retry; do
    build/update-audit/protocol-audit "$case"
done
expected_fault() {
    binary=$1
    case=$2
    pattern=$3
    log="build/update-audit/$case.log"
    if ASAN_OPTIONS=detect_leaks=0 "$binary" "$case" >"$log" 2>&1; then
        echo "Unexpected success: $case" >&2
        exit 1
    fi
    if ! grep -Eq "$pattern" "$log"; then
        echo "Failure did not reproduce expected sanitizer evidence: $case" >&2
        cat "$log" >&2
        exit 1
    fi
    echo "REPRODUCED sanitizer fault: $case (see $log)"
}
expected_fault build/update-audit/nvm-audit truncated 'ERROR: AddressSanitizer: heap-buffer-overflow'
expected_fault build/update-audit/protocol-audit overflow 'runtime error: index 512 out of bounds|ERROR: AddressSanitizer: global-buffer-overflow'
expected_fault build/update-audit/protocol-audit short-crc-command 'ERROR: AddressSanitizer: SEGV'
expected_fault build/update-audit/protocol-audit crc-range 'ERROR: AddressSanitizer: SEGV'
echo 'PASS: valid baselines and expected safety defects reproduced against exact generated code. This is NOT a safe-update approval; hardware update tasks remain disabled.'

/* Compile-only strong hook/exception destinations. No C state, stack,
 * reporting, peripherals, trap, ERET/DERET, reset or resume. Selected CRT
 * vector linkage and complete final executive paths still need link audit. */
.set nomips16
.set noreorder
.section .text.rehearsal_hooks,code
.align 2
.globl _on_reset
.ent _on_reset
_on_reset:
    jr $31
    nop
.end _on_reset
.size _on_reset, .-_on_reset
.globl _on_bootstrap
.ent _on_bootstrap
_on_bootstrap:
    jr $31
    nop
.end _on_bootstrap
.size _on_bootstrap, .-_on_bootstrap
.section .text.rehearsal_exception_stop,code
.align 2
.globl _nmi_handler
.globl _bootstrap_exception_handler
.globl _general_exception_handler
.globl _general_exception_context
.globl _DefaultInterrupt
.ent _nmi_handler
_nmi_handler:
_bootstrap_exception_handler:
_general_exception_handler:
_general_exception_context:
_DefaultInterrupt:
    b _nmi_handler
    nop
.end _nmi_handler
.size _nmi_handler, .-_nmi_handler
.size _bootstrap_exception_handler, .-_bootstrap_exception_handler
.size _general_exception_handler, .-_general_exception_handler
.size _general_exception_context, .-_general_exception_context
.size _DefaultInterrupt, .-_DefaultInterrupt

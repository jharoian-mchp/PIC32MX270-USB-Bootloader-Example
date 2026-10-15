#ifndef UPDATE_NVM_PORT_H
#define UPDATE_NVM_PORT_H
/* PIC32MX270F256D-only hardware operations; substituted in host tests. */
#include <xc.h>
#if !defined(__32MX270F256D__)
#error "This NVM port is qualified for compilation only on PIC32MX270F256D"
#endif
#include <sys/kmem.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
static inline uint32_t UPDATE_NvmPortInterruptDisable(void)
{return __builtin_disable_interrupts();}
static inline void UPDATE_NvmPortInterruptRestore(uint32_t status)
{__builtin_mtc0(12,0,status);}
static inline bool UPDATE_NvmPortBusy(void){return NVMCONbits.WR!=0U;}
static inline bool UPDATE_NvmPortDmaEnabled(void){return DMACONbits.ON!=0U;}
static inline uint32_t UPDATE_NvmPortError(void)
{return NVMCON&(_NVMCON_WRERR_MASK|_NVMCON_LVDERR_MASK);}
static inline bool UPDATE_NvmPortLowVoltage(void){return NVMCONbits.LVDSTAT!=0U;}
static inline void UPDATE_NvmPortWait(uint32_t ticks)
{
    uint32_t since=__builtin_mfc0(9,0);
    while((uint32_t)(__builtin_mfc0(9,0)-since)<ticks){}
}
static inline bool UPDATE_NvmPortSource(const uint32_t *source,uint32_t *physical)
{
    uintptr_t address=(uintptr_t)source;
    /* Require uncached KSEG1 SRAM, entire 128-byte row, word alignment. */
    if(address<0xA0000000U||address>0xA000FF80U||(address&3U)!=0U)return false;
    *physical=(uint32_t)KVA_TO_PA(address);return true;
}
static inline void UPDATE_NvmPortMode(uint32_t operation)
{NVMCONCLR=_NVMCON_WREN_MASK;NVMCON=operation|_NVMCON_WREN_MASK;}
static inline void UPDATE_NvmPortAddress(uint32_t address){NVMADDR=address;}
static inline void UPDATE_NvmPortSourceAddress(uint32_t address)
{__asm__ volatile("sync":::"memory");NVMSRCADDR=address;}
static inline __attribute__((always_inline)) void UPDATE_NvmPortLaunch(void)
{
    /* One compiler/assembler-ordered block: prepare operands BEFORE the keys;
     * no inserted loads, spills or scheduling between the unlock/WR stores.
     * IRQ/bus exclusion remains the caller's responsibility. */
    __asm__ volatile(
        ".set push\n\t"
        ".set noreorder\n\t"
        "sw $zero, 0(%0)\n\t"
        "sw %2, 0(%0)\n\t"
        "sw %3, 0(%0)\n\t"
        "sw %4, 0(%1)\n\t"
        ".set pop\n\t"
        :
        : "r" (&NVMKEY), "r" (&NVMCONSET), "r" (0xAA996655U),
          "r" (0x556699AAU), "r" (_NVMCON_WR_MASK)
        : "memory");
}
static inline void UPDATE_NvmPortDisarm(void){NVMCONCLR=_NVMCON_WREN_MASK;}
static inline bool UPDATE_NvmPortRead(uint32_t address,uint8_t *out,size_t size)
{memcpy(out,(const void *)PA_TO_KVA1(address),size);return true;}
#endif

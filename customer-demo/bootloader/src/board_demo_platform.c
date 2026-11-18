#include "board_demo_platform.h"
#include "board_demo.h"
#include "config/default/definitions.h"

/* Nominal 48 MHz SYSCLK gives Count 24 MHz. Accumulate deltas across wraps;
 * cooperative calls must be less than one Count wrap (~179 s) apart. */
static uint32_t last_count, remainder, milliseconds;
static bool clock_started;
static uint32_t Now(void *context)
{
    (void)context;
    uint32_t count = _CP0_GET_COUNT();
    if (!clock_started) { last_count=count; clock_started=true; return 0; }
    uint32_t delta=count-last_count; last_count=count;
    milliseconds += delta/24000U;
    remainder += delta%24000U;
    milliseconds += remainder/24000U; remainder %= 24000U;
    return milliseconds;
}
static bool WriteRead(void *ctx,uint8_t address,const uint8_t *tx,size_t nt,uint8_t *rx,size_t nr)
{ (void)ctx; return I2C1_WriteRead(address,(uint8_t *)tx,nt,rx,nr); }
static bool Write(void *ctx,uint8_t address,const uint8_t *tx,size_t n)
{ (void)ctx; return I2C1_Write(address,(uint8_t *)tx,n); }
static bool Busy(void *ctx) { (void)ctx; return I2C1_IsBusy(); }
static bool Error(void *ctx) { (void)ctx; return I2C1_ErrorGet()!=I2C_ERROR_NONE; }
static bool AdcStart(void *ctx) { (void)ctx; ADC_SamplingStart(); return true; }
static bool AdcReady(void *ctx) { (void)ctx; return ADC_ResultIsReady(); }
static uint16_t AdcResult(void *ctx) { (void)ctx; return (uint16_t)ADC_ResultGet(ADC_RESULT_BUFFER_0); }
void BOARD_DEMO_PlatformInitialize(void)
{
    static const BOARD_DEMO_OPS ops={Now,WriteRead,Write,Busy,Error,AdcStart,AdcReady,AdcResult};
    BOARD_DEMO_Initialize(&ops,NULL);
}
void BOARD_DEMO_PlatformTasks(void) { BOARD_DEMO_Tasks(); }
bool bootloader_Trigger(void)
{
    /* GPIO initialisation precedes this hook; explicitly establish RB2 input.
     * Four low samples over nominal 6 ms, bounded total 10 ms. Called with
     * interrupts disabled before stock run_Application and peripheral init. */
    ANSELBCLR=4U; TRISBSET=4U; CNPUBCLR=4U; CNPDBCLR=4U;
    uint32_t start=_CP0_GET_COUNT(), previous=start;
    unsigned low=0U;
    for (unsigned polls=0U; polls<1000000U &&
         (uint32_t)(_CP0_GET_COUNT()-start)<240000U; ++polls) {
        uint32_t now=_CP0_GET_COUNT();
        if (low==0U || (uint32_t)(now-previous)>=48000U) {
            if (BOOT_REQUEST_Get()!=0U) return false;
            previous=now;
            if (++low==4U) return true;
        }
    }
    return false;
}

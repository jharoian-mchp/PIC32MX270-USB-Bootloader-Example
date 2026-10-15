#include "update_nvm.h"
#include "update_image.h"
#include <update_nvm_port.h>
#define LOW_VOLTAGE 0x1000U

static void Settle(UPDATE_NVM *n)
{
    if(n->pending&&!UPDATE_NvmPortBusy())
    {
        /* No NVM register write until >=500 ns after completion. Conservative
         * 1 us core-count wait starts when main first observes WR clear. */
        UPDATE_NvmPortWait(n->settle_ticks);UPDATE_NvmPortDisarm();n->pending=false;
    }
}
static bool Busy(void *context)
{
    UPDATE_NVM *n=context;if(!n->initialized)return true;
    if(UPDATE_NvmPortBusy())return true;
    Settle(n);return false;
}
static uint32_t Error(void *context)
{
    UPDATE_NVM *n=context;
    return n->launch_error|UPDATE_NvmPortError();
}
static bool Span(uint32_t address,uint32_t size)
{
    return address>=UPDATE_APP_PHYSICAL_START&&address<UPDATE_FLASH_PHYSICAL_END&&
           size<=UPDATE_FLASH_PHYSICAL_END-address;
}
static bool Start(UPDATE_NVM *n,uint32_t address,const uint32_t *source,bool erase)
{
    uint32_t size=erase?UPDATE_ERASE_SIZE:UPDATE_ROW_SIZE,physical=0;
    if(!n->initialized||!Span(address,size)||(address&(size-1U))!=0U||
       (!erase&&!UPDATE_NvmPortSource(source,&physical)))return false;
    uint32_t interrupts=UPDATE_NvmPortInterruptDisable();
    if(UPDATE_NvmPortBusy()||n->pending||UPDATE_NvmPortDmaEnabled()||
       UPDATE_NvmPortUsbActive()||Error(n)!=0U)
    {UPDATE_NvmPortInterruptRestore(interrupts);return false;}
    UPDATE_NvmPortMode(erase?4U:3U);
    UPDATE_NvmPortAddress(address);
    if(!erase)UPDATE_NvmPortSourceAddress(physical);
    /* Programming specification supplies 6 us LVD startup; use 10 us. Core
     * counter remains running with interrupts disabled; never software ticks. */
    UPDATE_NvmPortWait(n->lvd_ticks);
    if(UPDATE_NvmPortLowVoltage())
    {
        n->launch_error=LOW_VOLTAGE;UPDATE_NvmPortDisarm();
        UPDATE_NvmPortInterruptRestore(interrupts);return false;
    }
    n->pending=true;UPDATE_NvmPortLaunch();
    /* PIC32 Flash fetch stalls may complete before launch returns. */
    Settle(n);UPDATE_NvmPortInterruptRestore(interrupts);return true;
}
static bool Erase(void *context,uint32_t address){return Start(context,address,NULL,true);}
static bool Write(void *context,const uint32_t *source,uint32_t address)
{return Start(context,address,source,false);}
static bool Read(void *context,uint32_t address,uint8_t *out,size_t size)
{
    UPDATE_NVM *n=context;
    if(!n->initialized||out==NULL||size==0U||size>UINT32_MAX||
       !Span(address,(uint32_t)size)||Busy(n))return false;
    return UPDATE_NvmPortRead(address,out,size);
}
bool UPDATE_NvmInit(UPDATE_NVM *n,uint32_t hz,UPDATE_FLASH_BACKEND *backend)
{
    if(n==NULL||backend==NULL||hz<4000000U||hz>50000000U||
       UPDATE_NvmPortBusy()||UPDATE_NvmPortDmaEnabled()||UPDATE_NvmPortUsbActive()||
       UPDATE_NvmPortError()!=0U)return false;
    *n=(UPDATE_NVM){.lvd_ticks=(hz+199999U)/200000U,
                   .settle_ticks=(hz+1999999U)/2000000U,.initialized=true};
    *backend=(UPDATE_FLASH_BACKEND){n,Busy,Error,Erase,Write,Read};return true;
}

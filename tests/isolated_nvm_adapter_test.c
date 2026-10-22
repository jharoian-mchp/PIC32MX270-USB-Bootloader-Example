#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "isolated_nvm.h"
#include "update_nvm.h"
#include <update_nvm_port.h>
static ISOLATED_NVM owner;static ISOLATED_NVM_MAILBOX mailbox;static UPDATE_NVM nvm;
static uint32_t count,address,operation,error;
static unsigned launches,settles,disarms,reads;
static bool wr,automatic=true,irq,usb,dma,low;
static const uint32_t *source;static uint8_t memory[1024];
uint32_t UPDATE_NvmPortInterruptDisable(void){assert(!irq);return 0;}
void UPDATE_NvmPortInterruptRestore(uint32_t v){assert(v==0&&!irq);}
bool UPDATE_NvmPortBusy(void){return wr;}
bool UPDATE_NvmPortDmaEnabled(void){return dma;}
bool UPDATE_NvmPortUsbActive(void){return usb;}
uint32_t UPDATE_NvmPortError(void){return error;}
bool UPDATE_NvmPortLowVoltage(void){return low;}
void UPDATE_NvmPortWait(uint32_t ticks){if(ticks==24)++settles;count+=ticks;}
bool UPDATE_NvmPortSource(const uint32_t *row,uint32_t *p)
{assert(row==owner.flash.row);source=row;*p=0x1200;return true;}
void UPDATE_NvmPortMode(uint32_t op){assert(!irq&&!wr&&!usb&&!dma);operation=op;}
void UPDATE_NvmPortAddress(uint32_t a){assert(a==owner.config.page);address=a;}
void UPDATE_NvmPortSourceAddress(uint32_t p){assert(p==0x1200);}
static void Complete(void){assert(wr&&address==owner.config.page);wr=false;if(operation==4)memset(memory,255,1024);else memcpy(memory,source,128);}
void UPDATE_NvmPortLaunch(void){assert(!irq&&!usb&&!dma);wr=true;++launches;if(automatic)Complete();}
void UPDATE_NvmPortDisarm(void){assert(!wr);++disarms;}
bool UPDATE_NvmPortRead(uint32_t a,uint8_t *out,size_t n)
{assert(!wr&&!nvm.pending&&a>=owner.config.page&&n<=owner.config.page+1024-a);++reads;memcpy(out,memory+a-owner.config.page,n);return true;}
static uint32_t Count(void *c){(void)c;return count;}
static uint32_t Unsafe(void *c){(void)c;return usb||dma||irq;}
static bool InitAdapter(void *c,UPDATE_FLASH_BACKEND *b){(void)c;return UPDATE_NvmInit(&nvm,48000000,b);}
static void Reset(void)
{
    count=address=operation=error=launches=settles=disarms=reads=0;wr=irq=usb=dma=low=false;automatic=true;
    memset(&nvm,0,sizeof(nvm));memset(memory,0x33,sizeof(memory));
    ISOLATED_NVM_CONFIG c={0x1D008000,24000000,24,240000,24000000};
    ISOLATED_NVM_PORT p={0,Count,Unsafe,InitAdapter};assert(ISOLATED_NvmInit(&owner,&mailbox,&c,&p));
}
static void Step(bool button,uint32_t ticks){count+=ticks;ISOLATED_NvmTasks(&owner,button);}
static void Arm(void)
{Step(false,0);Step(false,24);mailbox.token=ISOLATED_NVM_TOKEN;Step(false,0);Step(true,1);Step(true,24);Step(false,1);Step(false,24);assert(owner.report.state==ISOLATED_RUNNING);}
static void To(ISOLATED_NVM_PHASE phase)
{for(unsigned i=0;i<100&&owner.report.phase!=(unsigned)phase&&!owner.failed;i++)Step(false,24);assert(!owner.failed&&owner.report.phase==(unsigned)phase);}
int main(void)
{
    Reset();Arm();for(unsigned i=0;i<100&&owner.report.state!=ISOLATED_PASS_SETTLED&&!owner.failed;i++)Step(false,24);
    assert(owner.report.state==ISOLATED_PASS_SETTLED&&launches==2&&settles==2&&disarms==2&&reads==17&&!nvm.pending);
    /* Real adapter's pending completion remains owned past terminal timeout. */
    Reset();Arm();To(ISOLATED_PHASE_ROW);automatic=false;Step(false,24);
    assert(launches==2&&wr&&nvm.pending&&source==owner.flash.row);
    uint32_t saved=source[0];Step(false,24000000);assert(owner.failed&&owner.unsettled&&wr&&nvm.pending);
    unsigned before_reads=reads,before_disarms=disarms;Step(false,24);
    assert(source[0]==saved&&reads==before_reads&&disarms==before_disarms);
    Complete();Step(false,24);assert(owner.report.state==ISOLATED_FAIL_SETTLED&&!nvm.pending&&disarms==before_disarms+1&&reads==before_reads&&launches==2);
    /* Low voltage may perform setup/disarm but cannot launch or retry. */
    Reset();Arm();To(ISOLATED_PHASE_ERASE);low=true;Step(false,24);assert(owner.failed&&launches==0&&disarms==1&&nvm.launch_error!=0);
    Step(false,24);assert(owner.report.state==ISOLATED_FAIL_SETTLED&&launches==0);
    puts("PASS: inactive isolated owner through actual NVM adapter mocked port; launch/settle/disarm, timeout source lifetime and low-voltage rejection.");
}

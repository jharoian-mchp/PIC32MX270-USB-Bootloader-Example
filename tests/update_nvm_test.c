#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "update_nvm.h"
#include "update_image.h"
static bool busy,dma,usb_power,usb_busy,low,enabled=true,auto_finish,reject_source;
static uint32_t error,operation,address,physical;
static unsigned launches,reads;
static uint32_t waits[8];static unsigned wait_count;
static char events[64];static size_t event_count;
static const uint32_t *row;
static uint8_t memory[UPDATE_FLASH_PHYSICAL_END-UPDATE_APP_PHYSICAL_START];
static void Event(char c){assert(event_count<sizeof(events));events[event_count++]=c;}
uint32_t UPDATE_NvmPortInterruptDisable(void){Event('D');bool old=enabled;enabled=false;return old;}
void UPDATE_NvmPortInterruptRestore(uint32_t status){Event('R');enabled=status!=0;}
bool UPDATE_NvmPortBusy(void){return busy;}
bool UPDATE_NvmPortDmaEnabled(void){return dma;}
bool UPDATE_NvmPortUsbActive(void){return usb_power||usb_busy;}
uint32_t UPDATE_NvmPortError(void){return error;}
bool UPDATE_NvmPortLowVoltage(void){return low;}
void UPDATE_NvmPortWait(uint32_t ticks)
{Event('W');assert(wait_count<8);waits[wait_count++]=ticks;}
bool UPDATE_NvmPortSource(const uint32_t *source,uint32_t *p)
{
    if(reject_source||source==NULL||((uintptr_t)source&3U))return false;
    row=source;*p=0x1200;return true;
}
void UPDATE_NvmPortMode(uint32_t op){Event('M');assert(!enabled&&!busy);operation=op;}
void UPDATE_NvmPortAddress(uint32_t a){Event('A');address=a;}
void UPDATE_NvmPortSourceAddress(uint32_t a){Event('S');physical=a;}
static void Finish(void)
{
    assert(busy);busy=false;
    if(operation==4)memset(memory+address-UPDATE_APP_PHYSICAL_START,255,1024);
    else memcpy(memory+address-UPDATE_APP_PHYSICAL_START,row,128);
}
void UPDATE_NvmPortLaunch(void)
{Event('L');assert(!enabled&&!dma&&!usb_power&&!usb_busy);++launches;busy=true;if(auto_finish)Finish();}
void UPDATE_NvmPortDisarm(void){Event('C');assert(!busy);}
bool UPDATE_NvmPortRead(uint32_t a,uint8_t *out,size_t size)
{Event('Q');++reads;assert(!busy);memcpy(out,memory+a-UPDATE_APP_PHYSICAL_START,size);return true;}
static void Reset(void)
{
    busy=dma=usb_power=usb_busy=low=auto_finish=reject_source=false;enabled=true;
    error=launches=reads=wait_count=event_count=0;row=NULL;memset(memory,255,sizeof(memory));
}
static void Events(const char *expected)
{assert(event_count==strlen(expected));assert(!memcmp(events,expected,event_count));event_count=0;}
int main(void)
{
    UPDATE_NVM n;UPDATE_FLASH_BACKEND b;_Alignas(16) uint32_t data[32];memset(data,0x10,sizeof(data));
    uint8_t out[128];Reset();assert(UPDATE_NvmInit(&n,48000000,&b));
    assert(n.lvd_ticks==240&&n.settle_ticks==24);
    assert(!UPDATE_NvmInit(NULL,48000000,&b));assert(!UPDATE_NvmInit(&n,48000000,NULL));
    assert(!UPDATE_NvmInit(&n,3999999,&b));assert(!UPDATE_NvmInit(&n,50000001,&b));
    busy=true;assert(!UPDATE_NvmInit(&n,48000000,&b));busy=false;
    dma=true;assert(!UPDATE_NvmInit(&n,48000000,&b));dma=false;
    usb_power=true;assert(!UPDATE_NvmInit(&n,48000000,&b));usb_power=false;
    usb_busy=true;assert(!UPDATE_NvmInit(&n,48000000,&b));usb_busy=false;
    error=0x2000;assert(!UPDATE_NvmInit(&n,48000000,&b));error=0;
    assert(!b.erase(b.context,UPDATE_APP_PHYSICAL_START-1024));
    assert(!b.erase(b.context,UPDATE_FLASH_PHYSICAL_END));
    assert(!b.erase(b.context,UPDATE_APP_PHYSICAL_START+1));
    assert(!b.write(b.context,data,UPDATE_FLASH_PHYSICAL_END-127));
    assert(!b.write(b.context,NULL,UPDATE_APP_PHYSICAL_START));
    reject_source=true;assert(!b.write(b.context,data,UPDATE_APP_PHYSICAL_START));reject_source=false;
    assert(launches==0);Events("");
    dma=true;assert(!b.erase(b.context,UPDATE_APP_PHYSICAL_START));dma=false;Events("DR");
    usb_power=true;
    assert(!b.erase(b.context,UPDATE_APP_PHYSICAL_START));Events("DR");
    assert(!b.write(b.context,data,UPDATE_APP_PHYSICAL_START));Events("DR");
    usb_power=false;usb_busy=true;
    assert(!b.erase(b.context,UPDATE_APP_PHYSICAL_START));Events("DR");
    assert(!b.write(b.context,data,UPDATE_APP_PHYSICAL_START));Events("DR");
    usb_busy=false;assert(launches==0&&wait_count==0);
    busy=true;assert(!b.erase(b.context,UPDATE_APP_PHYSICAL_START));busy=false;Events("DR");
    error=0x2000;assert(!b.erase(b.context,UPDATE_APP_PHYSICAL_START));error=0;Events("DR");
    assert(b.write(b.context,data,UPDATE_APP_PHYSICAL_START));Events("DMASWLR");
    assert(operation==3&&address==UPDATE_APP_PHYSICAL_START&&physical==0x1200&&row==data);
    assert(wait_count==1&&waits[0]==240&&enabled&&n.pending);
    assert(b.busy(b.context));assert(!b.read(b.context,address,out,128));assert(reads==0);
    assert(!b.erase(b.context,UPDATE_IMAGE_META_ADDRESS));Events("DR");
    Finish();assert(!b.busy(b.context));Events("WC");
    assert(wait_count==2&&waits[1]==24&&!n.pending);
    assert(b.read(b.context,address,out,128));Events("Q");assert(!memcmp(out,data,128));
    assert(!b.read(b.context,UPDATE_FLASH_PHYSICAL_END,out,1));
    assert(!b.read(b.context,UPDATE_FLASH_PHYSICAL_END-1,out,2));
    assert(!b.read(b.context,address,out,0));assert(!b.read(b.context,address,NULL,128));
    wait_count=0;auto_finish=true;enabled=false;
    assert(b.erase(b.context,UPDATE_IMAGE_META_ADDRESS));Events("DMAWLWCR");
    assert(!enabled&&!n.pending&&waits[0]==240&&waits[1]==24);enabled=true;
    error=0x2000;assert(b.error(b.context)==0x2000);
    error=0; /* Errors exposed without automatic hardware clearing. */
    low=true;wait_count=0;unsigned old=launches;
    assert(!b.erase(b.context,UPDATE_APP_PHYSICAL_START));Events("DMAWCR");
    assert(launches==old&&b.error(b.context)==0x1000);
    low=false;assert(!b.erase(b.context,UPDATE_APP_PHYSICAL_START));Events("DR");
    /* Flash-layer copied source remains owned through asynchronous timeout. */
    Reset();assert(UPDATE_NvmInit(&n,48000000,&b));UPDATE_FLASH f;
    assert(UPDATE_FlashInit(&f,&b,UPDATE_APP_PHYSICAL_START,UPDATE_FLASH_PHYSICAL_END,100));
    assert(UPDATE_FlashWrite(&f,UPDATE_APP_PHYSICAL_START,(uint8_t *)data,0));
    assert(row==f.row);memset(data,0xEE,sizeof(data));
    assert(UPDATE_FlashPoll(&f,101)==UPDATE_FLASH_TIMEOUT&&busy);
    assert(!UPDATE_FlashReset(&f));assert(row[0]==0x10101010);Finish();
    assert(UPDATE_FlashReset(&f));
    /* Typical target stall: launch returns completed, then poll readback. */
    Reset();assert(UPDATE_NvmInit(&n,4000000,&b));assert(n.lvd_ticks==20&&n.settle_ticks==2);
    auto_finish=true;assert(UPDATE_FlashInit(&f,&b,UPDATE_APP_PHYSICAL_START,UPDATE_FLASH_PHYSICAL_END,100));
    assert(UPDATE_FlashWrite(&f,UPDATE_APP_PHYSICAL_START,(uint8_t *)data,0));
    assert(!busy&&UPDATE_FlashPoll(&f,1)==UPDATE_FLASH_OK);
    puts("PASS: NVM physical bounds/rows, explicit clock delays/WR settle, IRQ restore, DMA/USB-power/busy/LVD/errors, uncached-source port contract, stalled/asynchronous completion and timeout source ownership. No hardware binding enabled.");
    return 0;
}

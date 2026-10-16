#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
typedef uintptr_t USB_DEVICE_HANDLE,SYS_MODULE_OBJ;
typedef void (*USB_DEVICE_EVENT_HANDLER)(void);
typedef void (*USB_DEVICE_HID_EVENT_HANDLER)(void);
typedef int SYS_MODULE_INIT;
#define SYS_MODULE_OBJ_INVALID ((uintptr_t)-1)
#define USB_DEVICE_HANDLE_INVALID ((uintptr_t)-1)
#define DRV_USBFS_INDEX_0 0
#define USB_DEVICE_INDEX_0 0
static bool irq=true,poison;static unsigned step,reject;static char trace[20];static unsigned n;
static int drvUSBFSInit,usbDevInitData;
static struct {uintptr_t drvUSBFSObject,usbDevObject0;}sysObj;
static uint32_t Disable(void){uint32_t old=irq;irq=false;return old;}
static void Restore(uint32_t old){irq=old!=0;}
#define __builtin_disable_interrupts Disable
#define __builtin_mtc0(reg,sel,value) Restore(value)
static bool Call(char c){assert(!irq);trace[n++]=c;return ++step!=reject;}
static bool USB_RetirementPrepare(uint32_t generation){assert(generation==2);return Call('P');}
static uintptr_t DRV_USBFS_Initialize(unsigned i,SYS_MODULE_INIT *data)
{assert(i==0&&data==&drvUSBFSInit);return Call('D')?0:SYS_MODULE_OBJ_INVALID;}
static uintptr_t USB_DEVICE_Initialize(unsigned i,SYS_MODULE_INIT *data)
{assert(i==0&&data==&usbDevInitData);return Call('V')?0:SYS_MODULE_OBJ_INVALID;}
static void USB_DEVICE_Tasks(uintptr_t object){assert(object==0);trace[n++]='T';}
static bool USB_RetirementBindDevice(USB_DEVICE_EVENT_HANDLER cb,uintptr_t context,uintptr_t *handle)
{assert(cb&&context==123);if(!Call('B'))return false;*handle=456;return true;}
static bool USB_RetirementBindHid(USB_DEVICE_HID_EVENT_HANDLER cb,uintptr_t context)
{assert(cb&&context==123);return Call('H');}
static bool USB_RetirementReopen(uint32_t generation){assert(generation==2);return Call('R');}
static void USB_RetirementFault(void){poison=true;}
static void Callback(void){}
#include "rebuild.inc"
int main(void)
{
    uintptr_t handle=999;
    for(reject=1;reject<=6;reject++)
    {
        n=step=0;poison=false;handle=999;
        assert(!USB_RebuildDetached(2,Callback,Callback,123,&handle));assert(irq&&handle==999);
        assert(poison==(reject!=1));
    }
    reject=0;n=step=0;poison=false;
    assert(USB_RebuildDetached(2,Callback,Callback,123,&handle));
    assert(irq&&!poison&&handle==456&&n==7&&!memcmp(trace,"PDVTBHR",7));
    n=0;handle=999;assert(!USB_RebuildDetached(2,NULL,Callback,123,&handle)&&n==0&&handle==999);
    puts("PASS: exact USB-only rebuild preflight, layer/callback/reopen ordering, private handle publication and partial-failure quarantine; mocked initializer APIs, no hardware restart.");return 0;
}

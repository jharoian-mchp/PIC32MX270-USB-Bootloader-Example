/* Exact generated protocol source, host-only mocked datastream/flash layer. */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../bootloader/src/config/default/bootloader/bootloader_usb_device_hid.c"

static bool reject_record;
static unsigned erase_calls,program_calls;
uint16_t bootloader_GetVersion(void) { return 0x0309U; }
void bootloader_TriggerReset(void) {}
void bootloader_NvmAppErase(uint32_t a,uint32_t e) { (void)a;(void)e;++erase_calls; }
void bootloader_NVMPageWrite(uint32_t a,uint8_t *d) { (void)a;(void)d; }
bool bootloader_NvmIsBusy(void) { return false; }
HEX_RECORD_STATUS bootloader_NvmProgramHexRecord(uint8_t *r,uint32_t n)
{ (void)r;(void)n;++program_calls;return reject_record?HEX_REC_CRC_ERROR:HEX_REC_NORMAL; }
DATASTREAM_HANDLE DATASTREAM_Open(DRV_IO_INTENT i) { (void)i;return 1; }
void DATASTREAM_Close(void) {}
void DATASTREAM_Tasks(void) {}
DRV_CLIENT_STATUS DATASTREAM_ClientStatus(DATASTREAM_HANDLE h) { (void)h;return DRV_CLIENT_STATUS_READY; }
void DATASTREAM_BufferEventHandlerSet(DATASTREAM_HANDLE h,DATASTREAM_HandlerType c,uintptr_t x)
{ (void)h;(void)c;(void)x; }
int DATASTREAM_Data_Read(DATASTREAM_HANDLE h,uint8_t *p,size_t n) { (void)h;(void)p;return (int)n; }
int DATASTREAM_Data_Write(DATASTREAM_HANDLE h,uint8_t *p,const size_t n) { (void)h;(void)p;return (int)n; }

static void Reset(void)
{
    memset(&dataBuff,0,sizeof(dataBuff));
    btlData=(BOOTLOADER_DATA){.currentState=BOOTLOADER_WAIT_FOR_DONE,
       .prevState=BOOTLOADER_GET_COMMAND,.buffSize=512};
    reject_record=false;erase_calls=program_calls=0;
}
static size_t Frame(uint8_t command,const uint8_t *payload,size_t n,uint8_t *out)
{
    uint8_t decoded[64];assert(n+1U<=sizeof(decoded));decoded[0]=command;
    if(n)memcpy(decoded+1,payload,n);
    uint16_t crc=(uint16_t)bootloader_CalculateCrc(decoded,(uint32_t)n+1U);
    size_t pos=0;out[pos++]=SOH;
    for(size_t i=0;i<n+3U;i++)
    {
        uint8_t b=i<n+1U?decoded[i]:(i==n+1U?(uint8_t)crc:(uint8_t)(crc>>8));
        if(b==SOH||b==EOT||b==DLE)out[pos++]=DLE;
        out[pos++]=b;
    }
    out[pos++]=EOT;return pos;
}
int main(int argc,char **argv)
{
    assert(argc==2);const char *test=argv[1];Reset();
    if(!strcmp(test,"valid"))
    {
        size_t n=Frame(READ_BOOT_INFO,NULL,0,dataBuff.buffers.inputBuff);
        bootloader_BufferEventHandler(DATASTREAM_BUFFER_EVENT_COMPLETE,1,n);
        assert(btlData.usrBufferEventComplete&&btlData.cmdBuffLen==3);
        bootloader_ProcessBuffer(&btlData);
        assert(btlData.currentState==BOOTLOADER_SEND_RESPONSE&&btlData.buffSize==3);
        assert(dataBuff.buffers.inputBuff[1]==3&&dataBuff.buffers.inputBuff[2]==9);
        puts("PASS: exact generated protocol recognizes valid CRC-protected READ_BOOT_INFO.");
    }
    else if(!strcmp(test,"split-dle"))
    {
        size_t n=Frame(READ_BOOT_INFO,NULL,0,dataBuff.buffers.inputBuff);
        assert(dataBuff.buffers.inputBuff[1]==DLE);
        uint8_t remaining[64];memcpy(remaining,dataBuff.buffers.inputBuff+2,n-2U);
        dataBuff.buffers.inputBuff[2]=0x99; /* Not part of first transfer. */
        bootloader_BufferEventHandler(DATASTREAM_BUFFER_EVENT_COMPLETE,1,2);
        assert(btlData.cmdBuffLen==1&&dataBuff.buffers.procBuff[0]==0x99);
        memcpy(dataBuff.buffers.inputBuff,remaining,n-2U);
        bootloader_BufferEventHandler(DATASTREAM_BUFFER_EVENT_COMPLETE,1,n-2U);
        assert(!btlData.usrBufferEventComplete);
        puts("REPRODUCED: DLE at transfer end reads stale byte and loses split frame.");
    }
    else if(!strcmp(test,"overflow"))
    {
        dataBuff.buffers.inputBuff[0]=SOH;
        memset(dataBuff.buffers.inputBuff+1,0x55,511);
        bootloader_BufferEventHandler(DATASTREAM_BUFFER_EVENT_COMPLETE,1,512);
        memset(dataBuff.buffers.inputBuff,0x55,512);
        bootloader_BufferEventHandler(DATASTREAM_BUFFER_EVENT_COMPLETE,1,512);
        puts("FAIL: oversized frame unexpectedly survived.");return 1;
    }
    else if(!strcmp(test,"short-crc-command"))
    {
        /* A valid 1-byte command+CRC frame does not supply the eight parameters. */
        dataBuff.buffers.procBuff[0]=READ_CRC;
        btlData.cmdBuffLen=3;
        uint32_t address=0xFFFFFFFFU,length=1;
        memcpy(dataBuff.buffers.procBuff+1,&address,4);
        memcpy(dataBuff.buffers.procBuff+5,&length,4);
        bootloader_ProcessBuffer(&btlData); /* ASan catches unchecked address read. */
        puts("FAIL: unchecked CRC address unexpectedly survived.");return 1;
    }
    else if(!strcmp(test,"crc-range"))
    {
        uint32_t address=0xFFFFFFFFU,length=1;
        dataBuff.buffers.procBuff[0]=READ_CRC;btlData.cmdBuffLen=11;
        memcpy(dataBuff.buffers.procBuff+1,&address,4);
        memcpy(dataBuff.buffers.procBuff+5,&length,4);
        bootloader_ProcessBuffer(&btlData); /* Valid-size command, invalid memory range. */
        puts("FAIL: unchecked CRC range unexpectedly survived.");return 1;
    }
    else if(!strcmp(test,"program-retry"))
    {
        reject_record=true;dataBuff.buffers.procBuff[0]=PROGRAM_FLASH;
        btlData.cmdBuffLen=8;btlData.currentState=BOOTLOADER_PROCESS_COMMAND;
        bootloader_USB_DEVICE_HID_Tasks();bootloader_USB_DEVICE_HID_Tasks();
        assert(program_calls==2&&btlData.currentState==BOOTLOADER_PROCESS_COMMAND);
        puts("REPRODUCED: failed program command reprocessed indefinitely without response/resync.");
    }
    else { fprintf(stderr,"Unknown case: %s\n",test); return 2; }
    return 0;
}

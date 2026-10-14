/* Include the exact generated source in a host-only translation unit so tests
 * can reset its static state. No source edits or physical flash access. */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../bootloader/src/config/default/bootloader/bootloader_nvm_interface.c"

static unsigned writes, erases;
static uint32_t last_write, last_erase;
static bool reject_operation;
static uint8_t last_row[PAGE_SIZE];
bool NVM_IsBusy(void) { return false; }
bool NVM_RowWrite(uint32_t *data,uint32_t addr)
{ ++writes; last_write=addr; memcpy(last_row,data,PAGE_SIZE); return !reject_operation; }
bool NVM_PageErase(uint32_t addr)
{ ++erases; last_erase=addr; return !reject_operation; }
uint32_t NVM_ErrorGet(void) { return reject_operation ? 1U : 0U; }

static void Reset(void)
{
    nvm_data=(T_NVM_DATA){.progAddr=APP_START_ADDRESS,.prevAddr=APP_START_ADDRESS};
    nvmDataInitDone=false; writes=erases=0; last_write=last_erase=0; reject_operation=false;
    memset(last_row,0,sizeof(last_row));
}
static size_t Record(uint8_t *dst,uint8_t type,uint16_t address,const uint8_t *data,uint8_t n)
{
    dst[0]=n; dst[1]=(uint8_t)(address>>8); dst[2]=(uint8_t)address; dst[3]=type;
    if(n)memcpy(dst+4,data,n);
    uint8_t sum=0;for(size_t i=0;i<(size_t)n+4U;i++)sum+=dst[i];
    dst[n+4U]=(uint8_t)(0U-sum);return (size_t)n+5U;
}
static void Upper(uint16_t upper)
{
    uint8_t data[2]={(uint8_t)(upper>>8),(uint8_t)upper},r[7];
    size_t n=Record(r,EXT_LIN_ADRS_RECORD,0,data,2);
    assert(bootloader_NvmProgramHexRecord(r,(uint32_t)n)==HEX_REC_NORMAL);
}
static void End(void)
{
    uint8_t r[5]; Record(r,END_OF_FILE_RECORD,0,NULL,0);
    assert(bootloader_NvmProgramHexRecord(r,sizeof(r))==HEX_REC_NORMAL);
}
int main(int argc,char **argv)
{
    assert(argc==2);Reset();const char *test=argv[1];uint8_t r[64],data[4]={0x11,0x22,0x33,0x44};size_t n;
    if(!strcmp(test,"valid"))
    {
        Upper(0x1D00);n=Record(r,DATA_RECORD,0x8000,data,4);
        assert(bootloader_NvmProgramHexRecord(r,(uint32_t)n)==HEX_REC_NORMAL);End();
        assert(writes==1&&last_write==APP_START_ADDRESS&&!memcmp(last_row,data,4));
        for(size_t i=4;i<PAGE_SIZE;i++)assert(last_row[i]==0xFF);
        n=Record(r,DATA_RECORD,0x8000,data,4);r[n-1]^=1;
        assert(bootloader_NvmProgramHexRecord(r,(uint32_t)n)==HEX_REC_CRC_ERROR);
        puts("PASS: exact generated parser valid record, EOF row padding and checksum rejection.");
    }
    else if(!strcmp(test,"truncated"))
    {
        uint8_t *short_record=malloc(5);assert(short_record);
        memcpy(short_record,(uint8_t[]){255,0,0,0,0},5);
        (void)bootloader_NvmProgramHexRecord(short_record,5);free(short_record);
        puts("FAIL: malformed record unexpectedly survived.");return 1;
    }
    else if(!strcmp(test,"upper-end"))
    {
        Upper(0x1D04);n=Record(r,DATA_RECORD,0,data,1);
        assert(bootloader_NvmProgramHexRecord(r,(uint32_t)n)==HEX_REC_NORMAL);End();
        assert(writes==1&&last_write==FLASH_END_ADDRESS);
        puts("REPRODUCED: generated parser writes a row starting at exclusive FLASH_END_ADDRESS.");
    }
    else if(!strcmp(test,"cross-end"))
    {
        Upper(0x1D03);n=Record(r,DATA_RECORD,0xFFFF,data,4);
        assert(bootloader_NvmProgramHexRecord(r,(uint32_t)n)==HEX_REC_NORMAL);End();
        assert(writes==2&&last_write==FLASH_END_ADDRESS);
        puts("REPRODUCED: record crossing flash end queues a second row beyond flash.");
    }
    else if(!strcmp(test,"write-error"))
    {
        Upper(0x1D00);n=Record(r,DATA_RECORD,0x8000,data,4);
        assert(bootloader_NvmProgramHexRecord(r,(uint32_t)n)==HEX_REC_NORMAL);
        reject_operation=true;End();assert(writes==1);
        puts("REPRODUCED: rejected NVM row write still returns HEX_REC_NORMAL.");
    }
    else if(!strcmp(test,"erase-bounds"))
    {
        bootloader_NvmAppErase(FLASH_START,FLASH_START+ERASE_BLOCK_SIZE);
        assert(erases==1&&last_erase==FLASH_START);
        puts("REPRODUCED: erase helper accepts a bootloader-region address without bounds check.");
    }
    else if(!strcmp(test,"ignored-region"))
    {
        Upper(0x1FC0);n=Record(r,DATA_RECORD,0xBF0,data,4);
        assert(bootloader_NvmProgramHexRecord(r,(uint32_t)n)==HEX_REC_NORMAL);End();
        assert(writes==0);
        puts("REPRODUCED: config-region data silently skipped yet acknowledged normal.");
    }
    else if(!strcmp(test,"trailing"))
    {
        n=Record(r,EXT_LIN_ADRS_RECORD,0,(uint8_t[]){0x1D,0},2);r[n]=0x42;
        assert(bootloader_NvmProgramHexRecord(r,(uint32_t)n+1U)==HEX_REC_NORMAL);
        puts("REPRODUCED: trailing incomplete record byte accepted as normal.");
    }
    else { fprintf(stderr,"Unknown case: %s\n",test); return 2; }
    return 0;
}

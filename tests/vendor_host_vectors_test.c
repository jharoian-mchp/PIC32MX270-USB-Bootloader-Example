/* Concrete packets captured from the vendor Java packet-processor classes,
 * offline with byte-array streams; no HID hardware/commands sent. */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "update_protocol.h"
static const uint8_t info[]={1,16,1,0x21,16,16,4};
static const uint8_t erase[]={1,2,0x42,0x20,4};
static const uint8_t reset[]={1,5,0xA5,0x50,4};
static const uint8_t checksum[]={1,8,2,2,3,3,3,3,4,5,0x65,0xB8,4};
static void Decode(const uint8_t *p,size_t n,uint8_t command)
{
    UPDATE_FRAME f;size_t used;UPDATE_COMMAND c;UPDATE_FrameReset(&f);
    assert(UPDATE_FrameFeed(&f,p,n,&used)==UPDATE_FRAME_READY&&used==n);
    assert(f.length==1&&f.bytes[0]==command);
    assert(UPDATE_CommandValidate(f.bytes,f.length,&c)==UPDATE_COMMAND_VALID);
}
int main(void)
{
    Decode(info,sizeof(info),1);Decode(erase,sizeof(erase),2);Decode(reset,sizeof(reset),5);
    UPDATE_FRAME f;size_t used;UPDATE_FrameReset(&f);
    /* Direct synthetic payload wasn't pre-escaped, demonstrating the packer's
     * assumption that file/model has already escaped reserved payload bytes. */
    assert(UPDATE_FrameFeed(&f,checksum,sizeof(checksum),&used)==UPDATE_FRAME_CRC_ERROR);
    uint8_t decoded[]={8,2,2,3,3,3,3,4,5};UPDATE_COMMAND c;
    assert(UPDATE_CommandValidate(decoded,sizeof(decoded),&c)==UPDATE_COMMAND_UNSUPPORTED);
    assert(UPDATE_Crc16((uint8_t[]){1},1)==0x1021);
    assert(UPDATE_Crc16((uint8_t[]){2},1)==0x2042);
    puts("PASS: vendor info/erase/reset framing vectors; bare erase has no manifest; checksum ID8 is unsupported and payload escaping is a host-model responsibility.");
    return 0;
}

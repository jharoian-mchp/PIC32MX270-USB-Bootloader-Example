#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "update_wire.h"
int main(void)
{
    uint8_t out[28];size_t length;UPDATE_WIRE_QUERY q={.opcode=0x60,.request_id=1};
    assert(UPDATE_WireCapabilities(&q,out,sizeof(out),&length)&&length==28);
    uint32_t origin=(uint32_t)out[16]|((uint32_t)out[17]<<8)|((uint32_t)out[18]<<16)|((uint32_t)out[19]<<24);
#if USB_RESTART_DEBUG_LAYOUT
    assert(origin==0x1D00C000U);
#else
    assert(origin==0x1D008000U);
#endif
    assert(out[14]==0xFE&&out[15]==1); /* max510 unchanged */
    puts("PASS: explicit debug/normal capabilities bounds, no upload dispatch.");return 0;
}

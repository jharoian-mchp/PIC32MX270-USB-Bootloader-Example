#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "update_wire.h"
int main(void)
{
    uint8_t query[]={0x60,1,1,0,0,0};UPDATE_WIRE_QUERY q;
    assert(UPDATE_WireQuery(query,sizeof(query),&q)&&q.request_id==1);
    uint8_t expected[]={0xE0,1,0,0,1,0,0,0,'P','K','B','L',1,0,0xFE,1,
        0,0x80,0,0x1D,0,0xFC,3,0x1D,0x80,0,0,4};
    uint8_t response[28];size_t n=0;
    assert(UPDATE_WireCapabilities(&q,response,sizeof(response),&n)&&n==28);
    assert(!memcmp(response,expected,sizeof(response)));
    memset(response,0xAA,sizeof(response));
    assert(!UPDATE_WireCapabilities(&q,response,sizeof(response)-1,&n)&&n==0);
    for(size_t i=0;i<sizeof(response);i++)assert(response[i]==0xAA);
    for(size_t i=0;i<sizeof(query);i++)assert(!UPDATE_WireQuery(query,i,&q));
    query[1]=2;assert(!UPDATE_WireQuery(query,sizeof(query),&q));query[1]=1;
    query[2]=0;assert(!UPDATE_WireQuery(query,sizeof(query),&q));query[2]=1;
    query[0]=2;assert(!UPDATE_WireQuery(query,sizeof(query),&q));query[0]=0x61;
    assert(UPDATE_WireQuery(query,sizeof(query),&q));
    assert(!UPDATE_WireCapabilities(&q,response,sizeof(response),&n));
    assert(!UPDATE_WireQuery(NULL,6,&q));assert(!UPDATE_WireQuery(query,6,NULL));
    puts("PASS: C/Python read-only versioned query/capabilities wire vector, strict envelope lengths/version/request-ID and capacity; no command execution.");
    return 0;
}

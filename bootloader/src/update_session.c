#include "update_session.h"
#include <string.h>

static bool QueueRow(void *context,uint32_t address,const uint8_t *row)
{
    UPDATE_SESSION *s=context;
    if(s->queued>=UPDATE_ROW_QUEUE)return false;
    UPDATE_QUEUED_ROW *q=&s->queue[s->queued++];q->address=address;
    memcpy(q->bytes,row,UPDATE_ROW_SIZE);return true;
}
static UPDATE_SESSION_STATUS Failed(UPDATE_SESSION *s)
{
    UPDATE_HexAbort(&s->hex);s->status=UPDATE_SESSION_FAILED;
    return s->status;
}
bool UPDATE_SessionInit(UPDATE_SESSION *s,const UPDATE_FLASH_BACKEND *b,uint32_t timeout_ms)
{
    if(s==NULL)return false;
    *s=(UPDATE_SESSION){0};
    if(!UPDATE_FlashInit(&s->flash,b,UPDATE_APP_PHYSICAL_START,UPDATE_FLASH_PHYSICAL_END,timeout_ms))
        return false;
    s->timeout_ms=timeout_ms;s->initialized=true;return true;
}
bool UPDATE_SessionBegin(UPDATE_SESSION *s,uint32_t length,uint32_t crc,uint32_t now)
{
    if(s==NULL||!s->initialized||!UPDATE_ImageParameters(length)||
       (s->status!=UPDATE_SESSION_IDLE&&s->status!=UPDATE_SESSION_COMPLETE&&
        s->status!=UPDATE_SESSION_ABORTED)||!UPDATE_FlashReset(&s->flash))return false;
    s->length=length;s->expected_crc=crc;s->erase_address=UPDATE_APP_PHYSICAL_START;
    s->erase_end=(UPDATE_APP_PHYSICAL_START+length+UPDATE_ERASE_SIZE-1U)&~(UPDATE_ERASE_SIZE-1U);
    s->operation_active=false;s->batch_size=s->batch_offset=0;s->queued=s->queue_index=0;
    UPDATE_HexAbort(&s->hex);
    if(!UPDATE_FlashErase(&s->flash,UPDATE_IMAGE_META_ADDRESS,now)){Failed(s);return false;}
    s->operation_active=true;s->status=UPDATE_SESSION_ERASING_META;
    return true;
}
bool UPDATE_SessionSubmit(UPDATE_SESSION *s,const uint8_t *records,size_t size)
{
    if(s==NULL||!s->initialized||s->status!=UPDATE_SESSION_READY||records==NULL||
       size==0U||size>sizeof(s->batch))return false;
    if(UPDATE_HexCheck(&s->hex,records,size)!=UPDATE_HEX_ACCEPTED){Failed(s);return false;}
    memcpy(s->batch,records,size);s->batch_size=size;s->batch_offset=0;
    s->queued=s->queue_index=0;s->status=UPDATE_SESSION_PROGRAMMING;return true;
}
void UPDATE_SessionAbort(UPDATE_SESSION *s)
{
    if(s==NULL||!s->initialized||s->status==UPDATE_SESSION_IDLE||
       s->status==UPDATE_SESSION_ABORTED)return;
    UPDATE_HexAbort(&s->hex);s->batch_size=s->batch_offset=0;
    /* Queued rows no longer submitted; active flash owns a separate row copy. */
    s->queued=s->queue_index=0;s->status=UPDATE_SESSION_ABORTING;
}
static bool OperationDone(UPDATE_SESSION *s,uint32_t now)
{
    if(!s->operation_active)return true;
    UPDATE_FLASH_STATUS st=UPDATE_FlashPoll(&s->flash,now);
    if(st==UPDATE_FLASH_BUSY||st==UPDATE_FLASH_VERIFY)return false;
    s->operation_active=false;
    if(st!=UPDATE_FLASH_OK){Failed(s);return false;}
    (void)UPDATE_FlashReset(&s->flash);return true;
}
UPDATE_SESSION_STATUS UPDATE_SessionPoll(UPDATE_SESSION *s,uint32_t now)
{
    if(s==NULL||!s->initialized)return UPDATE_SESSION_FAILED;
    if(s->status==UPDATE_SESSION_ABORTING)
    {
        if(s->flash.backend.busy(s->flash.backend.context))return s->status;
        if(s->operation_active)
        {
            UPDATE_FLASH_STATUS st=UPDATE_FlashPoll(&s->flash,now);
            if(st==UPDATE_FLASH_VERIFY)return s->status;
            s->operation_active=false;
        }
        if(!UPDATE_FlashReset(&s->flash))return s->status;
        if(!UPDATE_FlashErase(&s->flash,UPDATE_IMAGE_META_ADDRESS,now))return Failed(s);
        s->operation_active=true;s->status=UPDATE_SESSION_ERASING_META;
        /* Abort sentinel: after verified metadata erase stop without app erase. */
        s->erase_address=s->erase_end=0;return s->status;
    }
    if(s->status==UPDATE_SESSION_IDLE||s->status==UPDATE_SESSION_FAILED||
       s->status==UPDATE_SESSION_ABORTED||s->status==UPDATE_SESSION_COMPLETE||
       s->status==UPDATE_SESSION_READY||s->status==UPDATE_SESSION_EOF)return s->status;
    if(!OperationDone(s,now))return s->status;
    switch(s->status)
    {
        case UPDATE_SESSION_ERASING_META:
        case UPDATE_SESSION_ERASING_APP:
            if(s->erase_end==0U){s->status=UPDATE_SESSION_ABORTED;break;}
            if(s->erase_address<s->erase_end)
            {
                uint32_t address=s->erase_address;s->erase_address+=UPDATE_ERASE_SIZE;
                if(!UPDATE_FlashErase(&s->flash,address,now))return Failed(s);
                s->operation_active=true;s->status=UPDATE_SESSION_ERASING_APP;
            }
            else
            {
                if(!UPDATE_HexBeginImage(&s->hex,s->length,QueueRow,s))return Failed(s);
                s->status=UPDATE_SESSION_READY;
            }
            break;
        case UPDATE_SESSION_PROGRAMMING:
            if(s->queue_index<s->queued)
            {
                UPDATE_QUEUED_ROW *q=&s->queue[s->queue_index++];
                if(!UPDATE_FlashWrite(&s->flash,q->address,q->bytes,now))return Failed(s);
                s->operation_active=true;
            }
            else if(s->batch_offset<s->batch_size)
            {
                /* Whole batch was preflighted. Process one complete record into
                 * at most three row copies, then drain/verify before continuing. */
                size_t size=(size_t)s->batch[s->batch_offset]+5U;
                s->queued=s->queue_index=0;
                UPDATE_HEX_STATUS st=UPDATE_HexFeed(&s->hex,s->batch+s->batch_offset,size);
                if(st!=UPDATE_HEX_ACCEPTED&&st!=UPDATE_HEX_FINISHED)return Failed(s);
                s->batch_offset+=size;
            }
            else
            {
                s->status=s->hex.eof?UPDATE_SESSION_EOF:UPDATE_SESSION_READY;
                if(s->status==UPDATE_SESSION_EOF)
                {
                    if(!UPDATE_ImageCheckExpected(&s->check,&s->flash.backend,s->length,s->expected_crc))
                        return Failed(s);
                    s->operation_since=now;s->status=UPDATE_SESSION_VERIFYING;
                }
            }
            break;
        case UPDATE_SESSION_VERIFYING:
        {
            uint32_t previous_offset=s->check.offset;
            UPDATE_IMAGE_STATUS st=UPDATE_ImageCheckPoll(&s->check);
            if(s->check.offset!=previous_offset)s->operation_since=now;
            if(st==UPDATE_IMAGE_VALID)
            {
                uint8_t row[UPDATE_ROW_SIZE];UPDATE_ImageRecord(row,s->length,s->expected_crc);
                if(!UPDATE_FlashWrite(&s->flash,UPDATE_IMAGE_META_ADDRESS,row,now))return Failed(s);
                s->operation_active=true;s->status=UPDATE_SESSION_COMMITTING;
            }
            else if(st!=UPDATE_IMAGE_CHECKING||(uint32_t)(now-s->operation_since)>=s->timeout_ms)
                return Failed(s);
            break;
        }
        case UPDATE_SESSION_COMMITTING:
            /* Commit-row write already completed and passed flash readback. */
            s->status=UPDATE_SESSION_COMPLETE;
            break;
        default: break;
    }
    return s->status;
}

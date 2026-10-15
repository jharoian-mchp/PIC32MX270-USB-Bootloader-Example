#ifndef UPDATE_LIFECYCLE_H
#define UPDATE_LIFECYCLE_H
#include "update_session.h"
#define UPDATE_LIFECYCLE_RECORD_MAX 496U
/* Typed, inactive controller. No wire opcode/version/USB callback binding.
 * Epoch must be fresh on reset; generation/authentication is NOT implemented. */
typedef enum { UPDATE_STAGE_BEGIN=1,UPDATE_STAGE_BATCH,UPDATE_STAGE_ABORT } UPDATE_STAGE_KIND;
typedef enum {
    UPDATE_LINK=0,UPDATE_STAGED,UPDATE_ACK_WAIT,UPDATE_OFF_WAIT,
    UPDATE_OFF_RUNNING,UPDATE_RESULT,UPDATE_RECOVERY
} UPDATE_LIFECYCLE_STATE;
typedef enum {
    UPDATE_OUTCOME_NONE=0,UPDATE_OUTCOME_DONE,UPDATE_OUTCOME_CANCELLED,UPDATE_OUTCOME_FAILED
} UPDATE_LIFECYCLE_OUTCOME;
typedef struct {
    uint64_t epoch;
    uint32_t token,id,sequence,length,crc;
    UPDATE_STAGE_KIND kind;
    size_t size;
    uint8_t records[UPDATE_LIFECYCLE_RECORD_MAX];
} UPDATE_STAGED_OPERATION;
typedef struct {
    UPDATE_SESSION session;
    UPDATE_STAGED_OPERATION staged;
    uint64_t epoch;
    uint32_t token,last_id,next_sequence,ticket,since,phase_timeout_ms;
    UPDATE_LIFECYCLE_STATE state;
    UPDATE_LIFECYCLE_OUTCOME outcome;
    bool initialized,enabled,have_stage,execute_accepted,invoked,cleanup_started;
} UPDATE_LIFECYCLE;
/* Init ONLY new/quiescent storage. Target backend must already be USB-off.
 * phase_timeout bounds staging/ACK/off operation/reconnect+resume; mock-only
 * values do not qualify target deadlines. NVM flash timeout is separate. */
bool UPDATE_LifecycleInit(UPDATE_LIFECYCLE *c,const UPDATE_FLASH_BACKEND *backend,
                         uint64_t epoch,uint32_t flash_timeout,uint32_t phase_timeout,bool enabled);
/* No flash effects, including malformed HEX. Exact latest stage replays return
 * its ticket in any state; no lease refresh. New stage requires LINK. */
bool UPDATE_LifecycleStage(UPDATE_LIFECYCLE *c,const UPDATE_STAGED_OPERATION *op,
                          uint32_t now,uint32_t *ticket);
bool UPDATE_LifecycleExecute(UPDATE_LIFECYCLE *c,uint64_t epoch,uint32_t token,
                            uint32_t id,uint32_t ticket,uint32_t now);
/* Transport owner supplies successful LOCAL full pause-response transfer.
 * Host receipt is unknown. Failure/cancel/late completion cannot start NVM. */
void UPDATE_LifecycleAck(UPDATE_LIFECYCLE *c,uint32_t ticket,bool success,uint32_t now);
/* Main owner passes true ONLY when USBPWR=USBBUSY=DMA=0 and retains exclusive
 * ownership through operation/settlement. False forbids session polling/start;
 * controller never toggles USB or manufactures a hardware-idle guarantee. */
UPDATE_LIFECYCLE_STATE UPDATE_LifecyclePoll(UPDATE_LIFECYCLE *c,uint32_t now,bool buses_inactive);
/* Unexpected disconnect/reset notification/expiry cancels incomplete work.
 * Planned shutdown after Ack uses OFF_WAIT, not Cancel. DONE+COMPLETE is final;
 * explicit staged ABORT is required to invalidate an already completed image. */
void UPDATE_LifecycleCancel(UPDATE_LIFECYCLE *c,uint32_t now);
bool UPDATE_LifecycleResume(UPDATE_LIFECYCLE *c,uint64_t epoch,uint32_t token,
                           uint32_t id,uint32_t ticket,uint32_t now);
#endif

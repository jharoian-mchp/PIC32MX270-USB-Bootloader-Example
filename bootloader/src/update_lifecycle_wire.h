#ifndef UPDATE_LIFECYCLE_WIRE_H
#define UPDATE_LIFECYCLE_WIRE_H
#include "update_lifecycle.h"
#define UPDATE_LINK_VERSION 2U
#define UPDATE_LINK_HELLO 0x70U
#define UPDATE_LINK_STAGE 0x71U
#define UPDATE_LINK_EXECUTE 0x72U
#define UPDATE_LINK_QUERY 0x73U
#define UPDATE_LINK_RESUME 0x74U
#define UPDATE_LINK_HEADER_SIZE 24U
#define UPDATE_LINK_REPLY_SIZE 56U
#define UPDATE_LINK_STAGE_RECORD_MAX 478U
/* Pure inactive wire parser/encoder; NO dispatcher, response send or flash.
 * Nonzero RPC ID correlates response; mutation ID separately names stage.
 * Inputs/outputs must be distinct storage (no overlapping buffers/objects). */
typedef struct {
    uint8_t opcode;
    uint32_t rpc_id,token,operation_id,ticket;
    uint64_t epoch;
    UPDATE_STAGED_OPERATION operation;
} UPDATE_LINK_REQUEST;
typedef struct {
    uint64_t epoch;
    uint32_t token,operation_id,ticket,next_sequence,length,crc;
    uint8_t lifecycle,outcome,session,flash,hex,features;
} UPDATE_LINK_STATUS;
#define UPDATE_LINK_FEATURE_SCHEMA 0x01U
#define UPDATE_LINK_FEATURE_TYPED_CORE 0x02U
/* Bit2: write permission; bit3: USB pause transport binding. Current firmware
 * negotiates NONE of these: deployed endpoint only understands version1 CAPS. */
#define UPDATE_LINK_FEATURE_WRITES 0x04U
#define UPDATE_LINK_FEATURE_TRANSPORT 0x08U
bool UPDATE_LinkDecode(const uint8_t *payload,size_t size,UPDATE_LINK_REQUEST *out);
bool UPDATE_LinkEncode(const UPDATE_LINK_REQUEST *request,uint8_t result,
                       const UPDATE_LINK_STATUS *status,uint8_t *out,size_t capacity,size_t *length);
#endif

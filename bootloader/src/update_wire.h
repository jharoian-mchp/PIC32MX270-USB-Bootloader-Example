#ifndef UPDATE_WIRE_H
#define UPDATE_WIRE_H
#include "update_protocol.h"
#include "update_image.h"
#define UPDATE_WIRE_VERSION 1U
#define UPDATE_WIRE_CAPABILITIES 0x60U
#define UPDATE_WIRE_STATUS 0x61U
#define UPDATE_WIRE_REQUEST_SIZE 6U
#define UPDATE_WIRE_RESPONSE_SIZE 8U
#define UPDATE_WIRE_CAP_BODY_SIZE 20U
/* Pure, read-only schema. NOT an active transport/dispatcher or flash binding. */
typedef struct { uint8_t opcode; uint32_t request_id; } UPDATE_WIRE_QUERY;
bool UPDATE_WireQuery(const uint8_t *payload,size_t length,UPDATE_WIRE_QUERY *query);
/* Writes one successful capabilities response (28-byte payload, before CRC).
 * Layout must match host/pic32_hid.py; accepts only a capabilities query. */
bool UPDATE_WireCapabilities(const UPDATE_WIRE_QUERY *query,
                              uint8_t *response,size_t capacity,size_t *written);
#endif

#ifndef UPDATE_AUDIT_COMMON_H
#define UPDATE_AUDIT_COMMON_H
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#define CACHE_ALIGN __attribute__((aligned(16)))
#define FLASH_START 0x9D000000U
#define FLASH_LENGTH 0x40000U
#define FLASH_END_ADDRESS (FLASH_START + FLASH_LENGTH)
#define APP_START_ADDRESS 0x9D008000U
#define APP_JUMP_ADDRESS APP_START_ADDRESS
#define PAGE_SIZE 128U
#define ERASE_BLOCK_SIZE 1024U
#define PA_TO_KVA0(a) (((uint32_t)(a) & 0x1FFFFFFFU) | 0x80000000U)
uint16_t bootloader_GetVersion(void);
void bootloader_TriggerReset(void);
#endif

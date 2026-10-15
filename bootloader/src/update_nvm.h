#ifndef UPDATE_NVM_H
#define UPDATE_NVM_H
#include "update_flash.h"
/* Inactive PIC32MX270F256D backend. No diagnostic caller installs it.
 * Port clocks must be genuine SYSCLK/2 core count (4..50 MHz SYSCLK).
 * Initialise only when quiescent; adapter cannot cancel an active operation. */
typedef struct {
    uint32_t lvd_ticks, settle_ticks, launch_error;
    bool initialized, pending;
} UPDATE_NVM;
bool UPDATE_NvmInit(UPDATE_NVM *nvm,uint32_t sysclk_hz,UPDATE_FLASH_BACKEND *backend);
#endif

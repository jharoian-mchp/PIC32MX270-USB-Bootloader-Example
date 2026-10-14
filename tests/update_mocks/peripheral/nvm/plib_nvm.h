#ifndef UPDATE_AUDIT_NVM_H
#define UPDATE_AUDIT_NVM_H
#include <stdint.h>
#include <stdbool.h>
bool NVM_IsBusy(void);
bool NVM_RowWrite(uint32_t *data, uint32_t address);
bool NVM_PageErase(uint32_t address);
uint32_t NVM_ErrorGet(void);
#endif

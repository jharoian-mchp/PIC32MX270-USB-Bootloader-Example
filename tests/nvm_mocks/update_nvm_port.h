#ifndef UPDATE_NVM_PORT_H
#define UPDATE_NVM_PORT_H
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
uint32_t UPDATE_NvmPortInterruptDisable(void);
void UPDATE_NvmPortInterruptRestore(uint32_t status);
bool UPDATE_NvmPortBusy(void);
bool UPDATE_NvmPortDmaEnabled(void);
uint32_t UPDATE_NvmPortError(void);
bool UPDATE_NvmPortLowVoltage(void);
void UPDATE_NvmPortWait(uint32_t ticks);
bool UPDATE_NvmPortSource(const uint32_t *source,uint32_t *physical);
void UPDATE_NvmPortMode(uint32_t operation);
void UPDATE_NvmPortAddress(uint32_t address);
void UPDATE_NvmPortSourceAddress(uint32_t address);
void UPDATE_NvmPortLaunch(void);
void UPDATE_NvmPortDisarm(void);
bool UPDATE_NvmPortRead(uint32_t address,uint8_t *out,size_t size);
#endif

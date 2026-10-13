#ifndef TEST_DEFINITIONS_H
#define TEST_DEFINITIONS_H
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
typedef enum { I2C_ERROR_NONE, I2C_ERROR_NACK, I2C_ERROR_BUS_COLLISION } I2C_ERROR;
typedef void (*TMR_CALLBACK)(uint32_t status, uintptr_t context);
void TMR2_CallbackRegister(TMR_CALLBACK callback, uintptr_t context);
void TMR2_Start(void);
bool EVIC_INT_Disable(void);
void EVIC_INT_Restore(bool state);
void I2C1_TransferAbort(void);
bool I2C1_IsBusy(void);
I2C_ERROR I2C1_ErrorGet(void);
bool I2C1_WriteRead(uint16_t address, uint8_t *tx, size_t tx_size, uint8_t *rx, size_t rx_size);
bool I2C1_Write(uint16_t address, uint8_t *tx, size_t tx_size);
#endif

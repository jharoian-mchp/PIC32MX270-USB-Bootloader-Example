#ifndef BOARD_DEMO_H
#define BOARD_DEMO_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Hardware-independent, single-instance board adapter. now_ms must be a
 * monotonically advancing millisecond counter (unsigned wrap is supported). */
typedef struct
{
    uint32_t (*now_ms)(void *context);
    bool (*i2c_write_read)(void *context, uint8_t address,
                           const uint8_t *tx, size_t tx_length,
                           uint8_t *rx, size_t rx_length);
    bool (*i2c_write)(void *context, uint8_t address,
                      const uint8_t *tx, size_t tx_length);
    bool (*i2c_busy)(void *context);
    bool (*i2c_error)(void *context);
    bool (*adc_start)(void *context);
    bool (*adc_ready)(void *context);
    uint16_t (*adc_result)(void *context);
} BOARD_DEMO_OPS;

typedef enum
{
    BOARD_DEMO_FAULT_NONE = 0,
    BOARD_DEMO_FAULT_INTERFACE,
    BOARD_DEMO_FAULT_I2C,
    BOARD_DEMO_FAULT_MUX_READBACK,
    BOARD_DEMO_FAULT_ADC_START,
    BOARD_DEMO_FAULT_ADC_TIMEOUT
} BOARD_DEMO_FAULT;

/* TAD=1 us and SAMC=16 are platform ADC configuration requirements.
 * This adapter starts and polls one conversion at a time. */
/* One-shot per reset; reinitialization is ignored to quarantine uncertain I2C
 * buffer ownership following a timeout. */
void BOARD_DEMO_Initialize(const BOARD_DEMO_OPS *ops, void *context);
void BOARD_DEMO_Tasks(void);

/* Pure status getters; faults are terminal until hardware reset. */
bool BOARD_DEMO_Ready(void);
bool BOARD_DEMO_CablePresent(void);
BOARD_DEMO_FAULT BOARD_DEMO_Fault(void);

#endif

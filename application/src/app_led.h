#ifndef APP_LED_H
#define APP_LED_H

#include <stdbool.h>
#include <stdint.h>

/* User-owned application code; do not replace during MCC regeneration. */
typedef enum
{
    APP_LED_FAULT_NONE = 0,
    APP_LED_FAULT_NACK,
    APP_LED_FAULT_BUS_COLLISION,
    APP_LED_FAULT_TIMEOUT
} APP_LED_FAULT;

typedef struct
{
    bool ready;
    bool led_on;
    APP_LED_FAULT last_fault;
    uint32_t errors;
    uint32_t transitions;
} APP_LED_DIAGNOSTICS;

/* Watch this symbol in MPLAB X. Updated in main context, not in an ISR. */
extern volatile APP_LED_DIAGNOSTICS app_led_diagnostics;

void APP_LED_Initialize(void);
void APP_LED_Tasks(void);

#endif

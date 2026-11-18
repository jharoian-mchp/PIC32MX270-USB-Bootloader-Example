#ifndef CUSTOMER_DEMO_APP_LED_H
#define CUSTOMER_DEMO_APP_LED_H

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
    APP_LED_FAULT_NONE = 0,
    APP_LED_FAULT_NACK,
    APP_LED_FAULT_BUS_COLLISION,
    APP_LED_FAULT_TIMEOUT,
    APP_LED_FAULT_START_REJECTED,
    APP_LED_FAULT_READBACK_MISMATCH
} APP_LED_FAULT;

typedef struct
{
    bool ready;
    bool led_on;
    bool terminal;
    APP_LED_FAULT last_fault;
    uint32_t transitions;
} APP_LED_DIAGNOSTICS;

extern volatile APP_LED_DIAGNOSTICS app_led_diagnostics;

void APP_LED_Initialize(void);
void APP_LED_Tasks(void);

#endif

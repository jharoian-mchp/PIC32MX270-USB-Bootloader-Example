#ifndef BOARD_USB_DIAG_H
#define BOARD_USB_DIAG_H

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
    BOARD_USB_FAULT_NONE = 0,
    BOARD_USB_FAULT_I2C,
    BOARD_USB_FAULT_MUX_READBACK,
    BOARD_USB_FAULT_ADC_TIMEOUT,
    BOARD_USB_FAULT_USB_OPEN
} BOARD_USB_FAULT;

typedef struct
{
    uint32_t adc_samples;
    uint32_t adc_timeouts;
    uint32_t mux_errors;
    uint32_t attach_count;
    uint32_t detach_count;
    uint32_t usb_resets;
    uint32_t usb_errors;
    uint16_t adc_raw;
    uint16_t adc_min;
    uint16_t adc_max;
    uint8_t mux_latch;
    uint8_t mux_direction;
    bool mux_ready;
    bool cable_present;
    bool attached;
    bool configured;
    bool suspended;
    bool usb_open;
    bool boot_request_active;
    bool startup_decided;
    bool diagnostic_enabled;
    BOARD_USB_FAULT last_fault;
} BOARD_USB_DIAGNOSTICS;

/* Watch in MPLAB X. USB event fields can change in interrupt context. */
extern volatile BOARD_USB_DIAGNOSTICS board_usb_diagnostics;

void BOARD_USB_DIAG_Initialize(void);
void BOARD_USB_DIAG_Tasks(void);

#endif

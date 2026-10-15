#ifndef USB_MOCK_DEFINITIONS_H
#define USB_MOCK_DEFINITIONS_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#define CPU_CLOCK_FREQUENCY 48000000U
extern uint32_t mock_count;
#define _CP0_GET_COUNT() mock_count
extern uint32_t ANSELASET, TRISASET, CNPUACLR, CNPDACLR, AD1CON2;
extern uint32_t ANSELBCLR, TRISBSET, CNPUBCLR, CNPDBCLR;
extern bool mock_button_high;
#define BOOT_REQUEST_Get() mock_button_high
typedef enum { I2C_ERROR_NONE, I2C_ERROR_NACK, I2C_ERROR_BUS_COLLISION } I2C_ERROR;
bool I2C1_IsBusy(void);
I2C_ERROR I2C1_ErrorGet(void);
void I2C1_TransferAbort(void);
bool I2C1_Write(uint16_t address, uint8_t *data, size_t size);
bool I2C1_WriteRead(uint16_t address, uint8_t *data, size_t size, uint8_t *rx, size_t rx_size);
bool EVIC_INT_Disable(void);
void EVIC_INT_Restore(bool enabled);
#define ADC_RESULT_BUFFER_0 0
void ADC_SamplingStart(void);
bool ADC_ResultIsReady(void);
uint32_t ADC_ResultGet(int buffer);
void ADC_Initialize(void);
typedef uintptr_t USB_DEVICE_HANDLE;
#define USB_DEVICE_HANDLE_INVALID ((USB_DEVICE_HANDLE)-1)
#define USB_DEVICE_INDEX_0 0
#define USB_DEVICE_HID_INDEX_0 0
#define DRV_IO_INTENT_READWRITE 3
#define USB_DEVICE_CONTROL_STATUS_OK 0
#define USB_DEVICE_HID_EVENT_RESPONSE void
#define USB_DEVICE_HID_EVENT_RESPONSE_NONE

typedef int USB_DEVICE_HID_INDEX;
typedef struct { uint8_t duration; uint8_t reportID; } USB_DEVICE_HID_EVENT_DATA_SET_IDLE;
typedef uintptr_t USB_DEVICE_HID_TRANSFER_HANDLE;
#define USB_DEVICE_HID_TRANSFER_HANDLE_INVALID ((USB_DEVICE_HID_TRANSFER_HANDLE)-1)
typedef int USB_DEVICE_HID_RESULT;
#define USB_DEVICE_HID_RESULT_OK 0
#define USB_DEVICE_HID_RESULT_ERROR -1
#define USB_DEVICE_HID_RESULT_ERROR_TRANSFER_QUEUE_FULL -2
typedef struct { size_t length; USB_DEVICE_HID_TRANSFER_HANDLE handle; USB_DEVICE_HID_RESULT status; } USB_DEVICE_HID_EVENT_DATA_REPORT_RECEIVED;
typedef USB_DEVICE_HID_EVENT_DATA_REPORT_RECEIVED USB_DEVICE_HID_EVENT_DATA_REPORT_SENT;
typedef enum { USB_DEVICE_HID_EVENT_SET_IDLE, USB_DEVICE_HID_EVENT_GET_IDLE,
 USB_DEVICE_HID_EVENT_REPORT_RECEIVED, USB_DEVICE_HID_EVENT_REPORT_SENT, HID_OTHER } USB_DEVICE_HID_EVENT;
typedef enum { USB_DEVICE_EVENT_CONFIGURED, USB_DEVICE_EVENT_RESET,
 USB_DEVICE_EVENT_DECONFIGURED, USB_DEVICE_EVENT_SUSPENDED, USB_DEVICE_EVENT_RESUMED,
 USB_DEVICE_EVENT_ERROR, USB_DEVICE_EVENT_POWER_DETECTED, USB_DEVICE_EVENT_POWER_REMOVED } USB_DEVICE_EVENT;
typedef void (*USB_DEVICE_EVENT_HANDLER)(USB_DEVICE_EVENT, void *, uintptr_t);
typedef void (*USB_DEVICE_HID_EVENT_HANDLER)(USB_DEVICE_HID_INDEX, USB_DEVICE_HID_EVENT, void *, uintptr_t);
USB_DEVICE_HANDLE USB_DEVICE_Open(int index, int intent);
void USB_DEVICE_EventHandlerSet(USB_DEVICE_HANDLE handle, USB_DEVICE_EVENT_HANDLER callback, uintptr_t context);
void USB_DEVICE_Attach(USB_DEVICE_HANDLE handle);
void USB_DEVICE_Detach(USB_DEVICE_HANDLE handle);
void USB_DEVICE_Tasks(uintptr_t obj);
int USB_DEVICE_HID_EventHandlerSet(int index, USB_DEVICE_HID_EVENT_HANDLER callback, uintptr_t context);
int USB_DEVICE_ControlStatus(USB_DEVICE_HANDLE handle, int status);
int USB_DEVICE_ControlSend(USB_DEVICE_HANDLE handle, void *data, size_t size);
USB_DEVICE_HID_RESULT USB_DEVICE_HID_ReportSend(int index,USB_DEVICE_HID_TRANSFER_HANDLE *handle,void *buffer,size_t size);
USB_DEVICE_HID_RESULT USB_DEVICE_HID_ReportReceive(int index,USB_DEVICE_HID_TRANSFER_HANDLE *handle,void *buffer,size_t size);
USB_DEVICE_HID_RESULT USB_DEVICE_HID_TransferCancel(int index,USB_DEVICE_HID_TRANSFER_HANDLE handle);
#define USB_ERROR_NONE 0
int USB_DEVICE_IRPCancelAll(USB_DEVICE_HANDLE handle,uint8_t endpoint);
extern struct SYSTEM_OBJECTS { uintptr_t usbDevObject0; } sysObj;
#endif

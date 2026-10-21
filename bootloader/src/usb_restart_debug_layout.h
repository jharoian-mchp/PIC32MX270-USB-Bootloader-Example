#ifndef USB_RESTART_DEBUG_LAYOUT_H
#define USB_RESTART_DEBUG_LAYOUT_H
/* Temporary read-only diagnostic layout. Never a production/application image. */
#ifndef USB_RESTART_DEBUG_LAYOUT
#define USB_RESTART_DEBUG_LAYOUT 0
#endif
#if USB_RESTART_DEBUG_LAYOUT
#if !USB_RETIREMENT_HOOKS || !USB_RESTART_DIAGNOSTIC
#error "expanded layout requires read-only restart diagnostic and retirement hooks"
#endif
#define USB_RESTART_DEBUG_APP_ORIGIN 0x1D00C000U
#endif
#endif

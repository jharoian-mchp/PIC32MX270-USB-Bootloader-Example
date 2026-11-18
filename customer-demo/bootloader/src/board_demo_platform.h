#ifndef BOARD_DEMO_PLATFORM_H
#define BOARD_DEMO_PLATFORM_H
#include <stdbool.h>
void BOARD_DEMO_PlatformInitialize(void);
/* Called before normal Harmony tasks; owns only expander/ADC polling. */
void BOARD_DEMO_PlatformTasks(void);
/* Stock pre-initialisation override: sampled active-low SW1, no USB/NVM. */
bool bootloader_Trigger(void);
#endif

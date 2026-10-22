#ifndef MOCK_ISOLATED_TARGET_HW_H
#define MOCK_ISOLATED_TARGET_HW_H
#include "isolated_nvm_target.h"
ISOLATED_TARGET_RAW ISOLATED_TargetHwRead(void);
void ISOLATED_TargetHwGpio(void);
uint32_t ISOLATED_TargetHwCount(void);
bool ISOLATED_TargetHwPressed(void);
#endif

#include "stm32g4xx_hal.h"

#include "bms_types.h"

#include <stdbool.h>

#ifndef INC_PROTECTION_H_
#define INC_PROTECTION_H_

void gateOFF();
void gateON();
extern volatile bool ocCheck;


void vProtectionTaskFunction(void *pvParamters);

#endif /* INC_PROTECTION_H_ */

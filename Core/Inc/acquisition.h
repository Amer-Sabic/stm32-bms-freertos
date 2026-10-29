/*
 * acquisition.h
 *
 *  Created on: 28. okt 2026.
 *      Author: Korisnik
 */

#ifndef INC_ACQUISITION_H_
#define INC_ACQUISITION_H_
#include "stm32g4xx_hal.h"
#include "macros.h"
#include "bms_types.h"
#include <stdbool.h>

extern void (*cellFunctionArr[4])();

bool send_event(EventType_t event, uint32_t arg);

void vAcquisitionTaskFunction(void *pvParameters);
#endif /* INC_ACQUISITION_H_ */

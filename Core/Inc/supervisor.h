/*
 * supervisor.h
 *
 *  Created on: 28. okt 2026.
 *      Author: Korisnik
 */

#ifndef INC_SUPERVISOR_H_
#define INC_SUPERVISOR_H_
#include "main.h"
#include "protection.h"
extern IWDG_HandleTypeDef hiwdg;
void IWDG_Conf();
void vSupervisorTaskFunction(void *pvParameters);
#endif /* INC_SUPERVISOR_H_ */

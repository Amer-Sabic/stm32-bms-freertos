/*
 * bms_types.h
 *
 *  Created on: 28. okt 2026.
 *      Author: Korisnik
 */

#ifndef INC_BMS_TYPES_H_
#define INC_BMS_TYPES_H_


typedef enum{
	STANDBY,
	ON,
	FAULT
}ProtState_t;

typedef struct{
	ProtState_t state;
	uint32_t faultCode;
}ProtStatus_t;

typedef enum {
	EVENT_NONE = 0,
	EVENT_OVERCURRENT,
	EVENT_MEAS_ERROR,
	EVENT_MEAS_TIMEOUT,
	EVENT_ENABLE,
	EVENT_DISABLE,
	EVENT_RESET
}EventType_t;
typedef struct{
	EventType_t type;
	uint32_t arg;
}ProtEvent_t;


typedef struct{
	float cell[4];
	float temp;
	float curr;
}MeasData_t;
typedef struct{
	MeasData_t m;
	ProtStatus_t st;
}TxPackage_t;

#endif /* INC_BMS_TYPES_H_ */

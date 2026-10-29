
#include "protection.h"
#include "main.h"

void gateOFF(){
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_RESET);
}
void gateON(){
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_SET);
}
volatile bool ocCheck = 0;


static uint32_t check(const MeasData_t *m)
{
    uint32_t f = 0;
    for (int i = 0; i < 4; i++) {
        if (m->cell[i] > CELL_OV_V) f |= FAULT_CELL_OV;
        if (m->cell[i] < CELL_UV_V) f |= FAULT_CELL_UV;
    }
    if (m->temp > TEMP_MAX_C)       f |= FAULT_OT;
    if (m->curr > CURR_MAX_A)    f |= FAULT_OC;
    return f;
}

static ProtState_t handle_event(ProtState_t state, ProtEvent_t ev, ProtStatus_t *st, MeasData_t m){
	switch(ev.type){
	case EVENT_OVERCURRENT:
		st->faultCode |= FAULT_OC;gateOFF();
		return FAULT;
	case EVENT_MEAS_ERROR:
		st->faultCode |= FAULT_MEAS_ERR;gateOFF();
		return FAULT;
	case EVENT_MEAS_TIMEOUT:
		st->faultCode |= FAULT_MEAS_TIMEOUT;gateOFF();
		return FAULT;

	case EVENT_ENABLE:
		if(state == STANDBY  && !check(&m) && ocCheck == 0){
			gateON();state = ON;
		}
		return state;
	case EVENT_DISABLE:
		if(state != FAULT){gateOFF();state=STANDBY;}
		return state;
	case EVENT_RESET:
		if(state == FAULT && !check(&m)){
			ocCheck = 0;
			st->faultCode = 0;
			gateOFF();
			return STANDBY;
		}
		if(state == ON)state = STANDBY;
		return state;
	}
	return state;
}
void vProtectionTaskFunction(void *pvParamters){
	MeasData_t m = {0};ProtEvent_t event;
	ProtState_t state = STANDBY;
	ProtStatus_t st = {0};
	for(;;){

		while(xQueueReceive(eventQueue, &event, 0) == pdPASS)
			state = handle_event(state, event, &st, m);

		if(xQueueReceive(measQueue,&m,pdMS_TO_TICKS(200)) == pdTRUE){
			uint32_t f = check(&m);
			if(f && state != FAULT){
				state=FAULT;gateOFF();
				st.faultCode |= f;
			}
		}
		else if(state != FAULT){
			state = FAULT;
			gateOFF();
			st.faultCode |= FAULT_MEAS_TIMEOUT;
		}
		st.state = state;
		TxPackage_t TxP;
		TxP.m = m;
		TxP.st = st;
		xQueueSend(transmissionQueue, &TxP, 0);
		xEventGroupSetBits(wdgEvents, PROT_BIT);
	}
}

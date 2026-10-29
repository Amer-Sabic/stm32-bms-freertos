#include "acquisition.h"
#include "gpio_mux.h"
#include "adc_current.h"
#include "adc_cell.h"
#include "adc_temp.h"
#include "main.h"
bool send_event(EventType_t event, uint32_t arg){
	ProtEvent_t protEvent = {event,arg};
	return xQueueSend(eventQueue,&protEvent,0) == pdTRUE;
}

void (*cellFunctionArr[4])() = {Cell1ON,Cell2ON,Cell3ON,Cell4ON};

float k = 1;
void vAcquisitionTaskFunction(void *pvParameters){
	MeasData_t m = {0};
	for(;;){
		m.cell[0] = 0;m.cell[1] = 0;m.cell[2] = 0;m.cell[3] = 0;m.temp = 0;m.curr = 0;
		for(int i=0;i<4;i++){
			cellFunctionArr[i]();
			vTaskDelay(pdMS_TO_TICKS(2));
			HAL_ADC_Start(&hadc);

			if(HAL_ADC_PollForConversion(&hadc,10) == HAL_OK){
				float cellVoltage = (HAL_ADC_GetValue(&hadc) - 2048) / 8.2f / 4095.0f  * 83.0f * 3.3f;
				m.cell[i] = cellVoltage;
			}
			else{
				send_event(EVENT_MEAS_ERROR,0);
			}
		}
		HAL_ADC_Start(&hadc_temp);
		float temperature = 0;
		if (HAL_ADC_PollForConversion(&hadc_temp, 10) == HAL_OK) {
			temperature = HAL_ADC_GetValue(&hadc_temp) / 4095.0 * 3.3 * k;
		}
		else{
			send_event(EVENT_MEAS_ERROR,0);
		}
		m.temp = temperature;
		m.curr = current;
		xQueueSend(measQueue, &m, 0);
		xEventGroupSetBits(wdgEvents, ACQ_BIT);
		vTaskDelay(pdMS_TO_TICKS(100));
	}
}


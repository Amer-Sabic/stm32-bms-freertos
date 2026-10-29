#include "supervisor.h"
#include "adc_current.h"
IWDG_HandleTypeDef hiwdg = {0};

void IWDG_Conf(){
	hiwdg.Instance = IWDG;
	hiwdg.Init.Prescaler = IWDG_PRESCALER_32;
	hiwdg.Init.Reload = 1000 - 1;
	hiwdg.Init.Window = IWDG_WINDOW_DISABLE;

	if(HAL_IWDG_Init(&hiwdg) != HAL_OK){
		Error_Handler();
	}

}



void vSupervisorTaskFunction(void *pvParameters){
	IWDG_Conf();
	for(;;){
		EventBits_t bits = xEventGroupWaitBits(wdgEvents, ALL_BIT, pdTRUE,pdTRUE, SUP_PERIOD);
		if((bits & ALL_BIT) == ALL_BIT){
			HAL_IWDG_Refresh(&hiwdg);
		}
		else{
			gateOFF();
		}
	}
}

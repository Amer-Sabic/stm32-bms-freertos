#include "adc_temp.h"
#include "main.h"
ADC_HandleTypeDef hadc_temp = {0};
void ADC_Temp_Conf(){
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_ADC345_CLK_ENABLE();


	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = GPIO_PIN_9;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	hadc_temp.Instance = ADC5;
	hadc_temp.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
	hadc_temp.Init.Resolution = ADC_RESOLUTION_12B;
	hadc_temp.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	hadc_temp.Init.ScanConvMode = ADC_SCAN_DISABLE;
	hadc_temp.Init.ContinuousConvMode = DISABLE;
	hadc_temp.Init.DiscontinuousConvMode = DISABLE;
	hadc_temp.Init.ExternalTrigConv = ADC_SOFTWARE_START;
	hadc_temp.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
	hadc_temp.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;

	if (HAL_ADC_Init(&hadc_temp) != HAL_OK) {
		Error_Handler();
	}


	ADC_ChannelConfTypeDef sConfig = {0};
	sConfig.Channel = ADC_CHANNEL_2;
	sConfig.Rank = ADC_REGULAR_RANK_1;
	sConfig.SamplingTime = ADC_SAMPLETIME_47CYCLES_5;
	sConfig.SingleDiff = ADC_SINGLE_ENDED;
	sConfig.Offset = 0;

	if (HAL_ADC_ConfigChannel(&hadc_temp, &sConfig) != HAL_OK) {
        Error_Handler();
	}


    if (HAL_ADCEx_Calibration_Start(&hadc_temp, ADC_SINGLE_ENDED) != HAL_OK) {
    	Error_Handler();
    }
}

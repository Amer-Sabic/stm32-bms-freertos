#include "adc_current.h"
#include "macros.h"
#include "main.h"

TIM_Base_InitTypeDef TIM_Base_InitStruct = {0};
TIM_HandleTypeDef TIM_HandleStruct = {0};
TIM_MasterConfigTypeDef TIM_MasterConfigStruct = {0};
ADC_HandleTypeDef ADC_HandleStruct = {0};

DMA_InitTypeDef DMA_InitStruct = {0};
DMA_HandleTypeDef DMA_HandleStruct = {0};
volatile float current = 0;
static uint16_t adc_buf[ADC_BUF_LEN];


void TIM_Conf(){
	__HAL_RCC_TIM7_CLK_ENABLE();
	TIM_HandleStruct.Instance = TIM7;
	TIM_Base_InitStruct.Prescaler = 17 - 1;
	TIM_Base_InitStruct.Period = 100 - 1;
	TIM_Base_InitStruct.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
	TIM_Base_InitStruct.CounterMode = TIM_COUNTERMODE_UP;
	TIM_HandleStruct.Init = TIM_Base_InitStruct;
	if(HAL_TIM_Base_Init(&TIM_HandleStruct) != HAL_OK)
		Error_Handler();

	TIM_MasterConfigStruct.MasterOutputTrigger = TIM_TRGO_UPDATE;
	TIM_MasterConfigStruct.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;

	if(HAL_TIMEx_MasterConfigSynchronization(&TIM_HandleStruct, &TIM_MasterConfigStruct) != HAL_OK)
		Error_Handler();
}


void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef *h) { __NOP(); }
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *h){
	if(h->Instance != ADC2)return;
	int32_t sum = 0;
	for(int i=0;i<ADC_PAIRS;i++){
		sum += ((int32_t)adc_buf[2*i+1] - (int32_t)adc_buf[2*i]);
	}
	float average = (float)sum / ADC_PAIRS;
	current = average * (3.3f / 4095.f) / 8.2f / 0.01f;
}


void DMA_ADC_Configuration(){

	__HAL_RCC_DMAMUX1_CLK_ENABLE();
	__HAL_RCC_DMA1_CLK_ENABLE();
	DMA_HandleStruct.Instance = DMA1_Channel1;
	DMA_InitStruct.Request = DMA_REQUEST_ADC2;
	DMA_InitStruct.Direction = DMA_PERIPH_TO_MEMORY;
	DMA_InitStruct.Mode = DMA_CIRCULAR;
	DMA_InitStruct.PeriphInc = DMA_PINC_DISABLE;
	DMA_InitStruct.MemInc = DMA_MINC_ENABLE;
	DMA_InitStruct.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
	DMA_InitStruct.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
	DMA_InitStruct.Priority = DMA_PRIORITY_HIGH;
	DMA_HandleStruct.Init = DMA_InitStruct;
	if (HAL_DMA_Init(&DMA_HandleStruct) != HAL_OK) Error_Handler();
	__HAL_LINKDMA(&ADC_HandleStruct, DMA_Handle, DMA_HandleStruct);

	HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 5, 0);
	HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);
}

void CURR_Start(void)
{
    HAL_ADC_Start_DMA(&ADC_HandleStruct, (uint32_t*)adc_buf, ADC_BUF_LEN);
    HAL_TIM_Base_Start(&TIM_HandleStruct);
}

void AWD_Conf(){
	ADC_AnalogWDGConfTypeDef awd = {0};
	awd.Channel = ADC_CHANNEL_4;
	awd.ITMode = ENABLE;
	awd.HighThreshold = AWD_HIGH;
	awd.LowThreshold = AWD_LOW;
	awd.WatchdogNumber = ADC_ANALOGWATCHDOG_1;
	awd.WatchdogMode = ADC_ANALOGWATCHDOG_SINGLE_REG;

	if(HAL_ADC_AnalogWDGConfig(&ADC_HandleStruct, &awd) != HAL_OK)
		Error_Handler();
	HAL_NVIC_SetPriority(ADC1_2_IRQn, 5, 0);
	HAL_NVIC_EnableIRQ(ADC1_2_IRQn);
}
void ADC_Curr_Conf(){

	GPIO_InitTypeDef GPIO_InitStruct = {0};
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_ADC12_CLK_ENABLE();

	//PA7 configuration
	GPIO_InitStruct.Pin = CURR_MEASUREMENT_PIN_P;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;

	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	//PA5 configuration
	GPIO_InitStruct.Pin = CURR_MEASUREMENT_PIN_N;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;

	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	ADC_InitTypeDef ADC_InitStruct = {0};

	ADC_HandleStruct.Instance = ADC2;
	ADC_InitStruct.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
	ADC_InitStruct.Resolution = ADC_RESOLUTION_12B;
	ADC_InitStruct.DataAlign = ADC_DATAALIGN_RIGHT;
	ADC_InitStruct.DMAContinuousRequests = ENABLE;
	ADC_InitStruct.ContinuousConvMode = DISABLE;
	ADC_InitStruct.ExternalTrigConv = ADC_EXTERNALTRIG_T7_TRGO;
	ADC_InitStruct.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_RISING;
	ADC_InitStruct.EOCSelection = ADC_EOC_SINGLE_CONV;
	ADC_InitStruct.ScanConvMode = ADC_SCAN_ENABLE;
	ADC_InitStruct.NbrOfConversion = 2;
	ADC_InitStruct.Overrun = ADC_OVR_DATA_OVERWRITTEN;

	ADC_HandleStruct.Init = ADC_InitStruct;
	if(HAL_ADC_Init(&ADC_HandleStruct) != HAL_OK){
		Error_Handler();
	}


	ADC_ChannelConfTypeDef ADC_ChannelConfStruct = {0};
	ADC_ChannelConfStruct.Offset = ADC_OFFSET_NONE;
	ADC_ChannelConfStruct.SingleDiff = ADC_SINGLE_ENDED;
	ADC_ChannelConfStruct.SamplingTime = ADC_SAMPLETIME_12CYCLES_5;


	ADC_ChannelConfStruct.Channel = ADC_CHANNEL_13;
	ADC_ChannelConfStruct.Rank = ADC_REGULAR_RANK_1;
	if(HAL_ADC_ConfigChannel(&ADC_HandleStruct, &ADC_ChannelConfStruct) != HAL_OK){
		Error_Handler();
	}
	ADC_ChannelConfStruct.Channel = ADC_CHANNEL_4;
	ADC_ChannelConfStruct.Rank = ADC_REGULAR_RANK_2;
	if(HAL_ADC_ConfigChannel(&ADC_HandleStruct, &ADC_ChannelConfStruct) != HAL_OK){
			Error_Handler();
	}
	if (HAL_ADCEx_Calibration_Start(&ADC_HandleStruct, ADC_SINGLE_ENDED) != HAL_OK) Error_Handler();
}


void MOSFET_GATE_CTRL_PIN_Conf(){
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	//PA12 (GATE CONTROL) configuration
	GPIO_InitStruct.Pin = GATE_CTRL_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}



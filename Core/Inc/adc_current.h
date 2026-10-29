
#ifndef INC_ADC_CURRENT_H_
#define INC_ADC_CURRENT_H_

#include "stm32g4xx_hal.h"
#include "macros.h"

extern DMA_InitTypeDef DMA_InitStruct;
extern DMA_HandleTypeDef DMA_HandleStruct;


extern TIM_Base_InitTypeDef TIM_Base_InitStruct;
extern TIM_HandleTypeDef TIM_HandleStruct;
extern TIM_MasterConfigTypeDef TIM_MasterConfigStruct;
extern ADC_HandleTypeDef ADC_HandleStruct;


void TIM_Conf();
void DMA_ADC_Configuration();

void CURR_Start(void);

extern volatile float current;
void AWD_Conf();
void ADC_Curr_Conf();

void MOSFET_GATE_CTRL_PIN_Conf();


void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef *h);
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *h);




#endif /* INC_ADC_CURRENT_H_ */

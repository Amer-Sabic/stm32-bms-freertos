/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"

#include "gpio_mux.h"
#include "adc_cell.h"
#include "adc_temp.h"
#include "adc_current.h"
#include "acquisition.h"
#include "protection.h"
#include "communication.h"
#include "supervisor.h"
#include <stdbool.h>
#include "event_groups.h"
#include <string.h>


EventGroupHandle_t wdgEvents;
QueueHandle_t measQueue;
QueueHandle_t eventQueue;
QueueHandle_t transmissionQueue;

void SystemClock_Config(void);



void BMS_Configuration(){

	  //Clock enable for ports A and C
	  __HAL_RCC_GPIOA_CLK_ENABLE();
	  __HAL_RCC_GPIOC_CLK_ENABLE();

	  //MOSFET gate control pin configuration
	  MOSFET_GATE_CTRL_PIN_Conf();
	  //Multiplexers selection pins configuration
	  MUX_SEL_PINS_Conf();

	  //Timer configuration
	  TIM_Conf();

	  //ADC for current measurement configuration
	  ADC_Curr_Conf();

	  //DMA for current data configuration
	  DMA_ADC_Configuration();
	  CURR_Start();


	  //ADCs for voltage and temp measuring configuration
	  ADC_Cell_Conf();
	  ADC_Temp_Conf();

	  //Analog watchdog configuration
	  AWD_Conf();

	  //DMA and UART configuration
	  DMA_UART_Conf();
}

int main(void)
{
  HAL_Init();
  SystemClock_Config();

  BMS_Configuration();
  measQueue = xQueueCreate(4, sizeof(MeasData_t));
  eventQueue = xQueueCreate(8, sizeof(ProtEvent_t));
  transmissionQueue = xQueueCreate(4, sizeof(TxPackage_t));
  wdgEvents = xEventGroupCreate();


  xTaskCreate(vProtectionTaskFunction, "ProtectionFun", 512, NULL, 4, NULL);
  xTaskCreate(vAcquisitionTaskFunction, "AcquisitionFun", 256, NULL, 3, NULL);
  xTaskCreate(vEspTxTaskFunction, "CommunicationFun", 512, NULL, 2, &commTaskHandle);
  xTaskCreate(vSupervisorTaskFunction, "SupervisionFun", 256, NULL, 1, NULL);

  //Watchdog configuration
  gateOFF();
  vTaskStartScheduler();
  for(;;){}
}

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM6)
  {
    HAL_IncTick();
  }

}

void Error_Handler(void)
{

  __disable_irq();
  while (1)
  {
  }

}
#ifdef USE_FULL_ASSERT

void assert_failed(uint8_t *file, uint32_t line)
{

}
#endif

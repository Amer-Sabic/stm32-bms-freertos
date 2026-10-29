/*
 * communication.h
 *
 *  Created on: 28. okt 2026.
 *      Author: Korisnik
 */

#ifndef INC_COMMUNICATION_H_
#define INC_COMMUNICATION_H_

#include "stm32g4xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "bms_types.h"

extern UART_HandleTypeDef huart_esp;
extern DMA_HandleTypeDef hdma_esp_rx;
extern DMA_HandleTypeDef hdma_esp_tx;
extern TaskHandle_t commTaskHandle;


size_t build_frame(uint8_t *f, const TxPackage_t *TxP);

void DMA_UART_Conf();

void vEspTxTaskFunction(void *pvParameters);
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *h, uint16_t size);
#endif /* INC_COMMUNICATION_H_ */

#include "communication.h"
#include "main.h"

#include <string.h>

#include "macros.h"
#include "acquisition.h"
UART_HandleTypeDef huart_esp = {0};
DMA_HandleTypeDef hdma_esp_rx = {0};
DMA_HandleTypeDef hdma_esp_tx = {0};
TaskHandle_t commTaskHandle = NULL;
static uint8_t rxBuf[RX_BUF_LEN];
static uint16_t rxLen = 0;

static uint16_t crc16(const uint8_t *d, size_t n)
{
    uint16_t crc = 0xFFFF;
    while (n--) {
        crc ^= (uint16_t)(*d++) << 8;
        for (int i = 0; i < 8; i++)
            crc = (crc & 0x8000) ? (crc << 1) ^ 0x1021 : (crc << 1);
    }
    return crc;
}

static size_t put_f32(uint8_t *p, float v) { memcpy(p, &v, 4); return 4; }

size_t build_frame(uint8_t *f, const TxPackage_t *TxP){
	size_t i = 3;
	for(int k=0;k<4;k++)i += put_f32(&f[i], TxP->m.cell[k]);

	i += put_f32(&f[i],TxP->m.temp);
	i += put_f32(&f[i],TxP->m.curr);
	f[i++] = TxP->st.state;
	memcpy(&f[i],&TxP->st.faultCode,4);i+=4;


	f[0] = FRAME_START;
	f[1] = TYPE_TELEM;
	f[2] = (uint8_t)(i-3);

	uint16_t c = crc16(&f[1], i - 1);
	f[i++] = c >> 8;
	f[i++] = c & 0xFF;
    return i;
}
static void rx_parse(const uint8_t *b, uint16_t n)
{
    if (n < 5 || b[0] != FRAME_START) return;
    uint8_t len = b[2];
    if (n < (uint16_t)(5 + len)) return;

    uint16_t rx_crc = ((uint16_t)b[3 + len] << 8) | b[4 + len];
    if (crc16(&b[1], 2 + len) != rx_crc) return;

    switch (b[1]) {
    case CMD_ENABLE:
    	send_event(EVENT_ENABLE, 0);  break;
    case CMD_DISABLE: send_event(EVENT_DISABLE, 0); break;
    case CMD_RESET:   send_event(EVENT_RESET, 0);   break;
    default: break;
    }
}

void DMA_UART_Conf(){
	__HAL_RCC_USART3_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_DMAMUX1_CLK_ENABLE();
	__HAL_RCC_DMA1_CLK_ENABLE();

	GPIO_InitTypeDef gpio = {0};

	gpio.Pin = GPIO_PIN_10 | GPIO_PIN_11;
	gpio.Mode = GPIO_MODE_AF_PP;
	gpio.Speed = GPIO_SPEED_FREQ_HIGH;
	gpio.Alternate = GPIO_AF7_USART3;
	gpio.Pull = GPIO_NOPULL;

	HAL_GPIO_Init(GPIOC, &gpio);

	hdma_esp_rx.Instance = DMA1_Channel2;
	hdma_esp_rx.Init.Request = DMA_REQUEST_USART3_RX;
	hdma_esp_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
	hdma_esp_rx.Init.PeriphInc = DMA_PINC_DISABLE;
	hdma_esp_rx.Init.MemInc = DMA_MINC_ENABLE;
	hdma_esp_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
	hdma_esp_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
	hdma_esp_rx.Init.Mode = DMA_NORMAL;
	hdma_esp_rx.Init.Priority = DMA_PRIORITY_LOW;

	if(HAL_DMA_Init(&hdma_esp_rx) != HAL_OK)Error_Handler();
	__HAL_LINKDMA(&huart_esp,hdmarx, hdma_esp_rx);

	hdma_esp_tx.Instance = DMA1_Channel3;
	hdma_esp_tx.Init.Request = DMA_REQUEST_USART3_TX;
	hdma_esp_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
	hdma_esp_tx.Init.PeriphInc = DMA_PINC_DISABLE;
	hdma_esp_tx.Init.MemInc = DMA_MINC_ENABLE;
	hdma_esp_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
	hdma_esp_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
	hdma_esp_tx.Init.Mode = DMA_NORMAL;
	hdma_esp_tx.Init.Priority = DMA_PRIORITY_LOW;

	if(HAL_DMA_Init(&hdma_esp_tx) != HAL_OK)Error_Handler();
	__HAL_LINKDMA(&huart_esp,hdmatx, hdma_esp_tx);

	huart_esp.Instance = USART3;
	huart_esp.Init.BaudRate = 115200;
	huart_esp.Init.WordLength = UART_WORDLENGTH_8B;
	huart_esp.Init.Parity = UART_PARITY_NONE;
	huart_esp.Init.StopBits = UART_STOPBITS_1;
	huart_esp.Init.Mode = UART_MODE_TX_RX;
	huart_esp.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart_esp.Init.OverSampling = UART_OVERSAMPLING_16;

	if(HAL_UART_Init(&huart_esp) != HAL_OK)Error_Handler();

	HAL_NVIC_SetPriority(DMA1_Channel2_IRQn, 6, 0);
	HAL_NVIC_EnableIRQ(DMA1_Channel2_IRQn);
	HAL_NVIC_SetPriority(DMA1_Channel3_IRQn, 6, 0);
	HAL_NVIC_EnableIRQ(DMA1_Channel3_IRQn);
	HAL_NVIC_SetPriority(USART3_IRQn,6,0);
	HAL_NVIC_EnableIRQ(USART3_IRQn);

}

void vEspTxTaskFunction(void *pvParameters){
	TxPackage_t TxP;
	static uint8_t frame[48];
	HAL_UARTEx_ReceiveToIdle_DMA(&huart_esp, rxBuf, RX_BUF_LEN);
	for(;;){
		if(ulTaskNotifyTake(pdTRUE,pdMS_TO_TICKS(20)) > 0){
			rx_parse(rxBuf, rxLen);
			HAL_UARTEx_ReceiveToIdle_DMA(&huart_esp, rxBuf, RX_BUF_LEN);
		}
		if(xQueueReceive(transmissionQueue, &TxP,0) == pdTRUE){
			size_t length = build_frame(frame,&TxP);
			HAL_UART_Transmit_DMA(&huart_esp,frame,length);
		}
		xEventGroupSetBits(wdgEvents, COMM_BIT);
		vTaskDelay(pdMS_TO_TICKS(5));
	}

}


void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *h, uint16_t size)
{
	rxLen = size;
	BaseType_t higherPriorityWoken = pdFALSE;
	vTaskNotifyGiveFromISR(commTaskHandle, &higherPriorityWoken);
	portYIELD_FROM_ISR(higherPriorityWoken);
}

#ifndef __DEBUG_USART_H
#define __DEBUG_USART_H

#include "stm32f4xx.h"
#include <stdio.h>

//引脚定义
/*******************************************************/
#define DEBUG_USART USART1
#define DEBUG_USART_CLK RCC_APB2Periph_USART1
#define DEBUG_USART_BAUDRATE 115200 //串口波特率

#define DEBUG_USART_RX_GPIO_PORT GPIOB
#define DEBUG_USART_RX_GPIO_CLK RCC_AHB1Periph_GPIOB
#define DEBUG_USART_RX_PIN GPIO_Pin_7
#define DEBUG_USART_RX_AF GPIO_AF_USART1
#define DEBUG_USART_RX_SOURCE GPIO_PinSource7

#define DEBUG_USART_TX_GPIO_PORT GPIOB
#define DEBUG_USART_TX_GPIO_CLK RCC_AHB1Periph_GPIOB
#define DEBUG_USART_TX_PIN GPIO_Pin_6
#define DEBUG_USART_TX_AF GPIO_AF_USART1
#define DEBUG_USART_TX_SOURCE GPIO_PinSource6

#define DEBUG_USART_IRQHandler USART1_IRQHandler
#define DEBUG_USART_IRQ USART1_IRQn
/************************************************************/

void Debug_USART_Config(void);

void usb2uart_rx_dma_config(void);
void usb2uart_uart_send_bydma(uint8_t *data, uint32_t len);
#endif /* __USART1_H */

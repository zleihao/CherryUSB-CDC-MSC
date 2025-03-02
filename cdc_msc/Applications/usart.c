#include "usart.h"
#include "usbd_core.h"
#include "chry_ringbuffer.h"
#include "usbd_cdc_acm.h"

uint8_t receive[4096];
uint32_t g_uart_tx_transfer_length = 0;

 /**
  * @brief  ����Ƕ�������жϿ�����NVIC
  * @param  ��
  * @retval ��
  */
static void NVIC_Configuration(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;
  
  /* Ƕ�������жϿ�������ѡ�� */
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
  
  /* ����USARTΪ�ж�Դ */
  NVIC_InitStructure.NVIC_IRQChannel = DEBUG_USART_IRQ;
  /* �������ȼ�Ϊ1 */
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  /* �����ȼ�Ϊ1 */
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  /* ʹ���ж� */
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  /* ��ʼ������NVIC */
  NVIC_Init(&NVIC_InitStructure);
}


 /**
  * @brief  DEBUG_USART GPIO ����,����ģʽ���á�115200 8-N-1 ���жϽ���ģʽ
  * @param  ��
  * @retval ��
  */
void Debug_USART_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
		
	RCC_AHB1PeriphClockCmd(DEBUG_USART_RX_GPIO_CLK|DEBUG_USART_TX_GPIO_CLK,ENABLE);

	/* ʹ�� USART ʱ�� */
	RCC_APB2PeriphClockCmd(DEBUG_USART_CLK, ENABLE);

	/* GPIO��ʼ�� */
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;  
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

	/* ����Tx����Ϊ���ù���  */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Pin = DEBUG_USART_TX_PIN  ;  
	GPIO_Init(DEBUG_USART_TX_GPIO_PORT, &GPIO_InitStructure);

	/* ����Rx����Ϊ���ù��� */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Pin = DEBUG_USART_RX_PIN;
	GPIO_Init(DEBUG_USART_RX_GPIO_PORT, &GPIO_InitStructure);

	/* ���� PXx �� USARTx_Tx*/
	GPIO_PinAFConfig(DEBUG_USART_RX_GPIO_PORT,DEBUG_USART_RX_SOURCE,DEBUG_USART_RX_AF);
	/*  ���� PXx �� USARTx__Rx*/
	GPIO_PinAFConfig(DEBUG_USART_TX_GPIO_PORT,DEBUG_USART_TX_SOURCE,DEBUG_USART_TX_AF);


	NVIC_Configuration();
}

/*********************DMA*************************************/
void usb2uart_rx_dma_config(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
    DMA_InitTypeDef dma_init_struct;
    
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
	
    DMA_DeInit(DMA2_Stream2);
    
    dma_init_struct.DMA_Channel = DMA_Channel_4;
    dma_init_struct.DMA_PeripheralBaseAddr = USART1_BASE + 0x04;
    dma_init_struct.DMA_Memory0BaseAddr = (uint32_t)receive;
    dma_init_struct.DMA_DIR = DMA_DIR_PeripheralToMemory;
    dma_init_struct.DMA_BufferSize = sizeof(receive);
    dma_init_struct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    dma_init_struct.DMA_MemoryInc = DMA_MemoryInc_Enable;
    dma_init_struct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    dma_init_struct.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    dma_init_struct.DMA_Mode = DMA_Mode_Normal;
    dma_init_struct.DMA_Priority = DMA_Priority_High;
    dma_init_struct.DMA_FIFOMode = DMA_FIFOMode_Disable;
    dma_init_struct.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
    dma_init_struct.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
    dma_init_struct.DMA_MemoryBurst = DMA_MemoryBurst_Single;
    
    DMA_Init(DMA2_Stream2, &dma_init_struct);
    
	/* ���� DMA �ж� */
	/* Ƕ�������жϿ�������ѡ�� */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

	/* ����USARTΪ�ж�Դ */
	NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream4_IRQn;
	/* �������ȼ�Ϊ1 */
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	/* �����ȼ�Ϊ1 */
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	/* ʹ���ж� */
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	/* ��ʼ������NVIC */
	NVIC_Init(&NVIC_InitStructure);
	
	//ʹ��DMA��������ж�
	DMA_ITConfig(DMA2_Stream2, DMA_IT_TC, ENABLE);
	
	//ʹ�� DMA ͨ��
	DMA_Cmd(DMA2_Stream2, ENABLE);
	
	USART_DMACmd(USART1, USART_DMAReq_Rx, ENABLE);
	
	/* ���� USART1 �ж� */
	// ����USART���ջ���ǿ��ж�
	USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);

    // ���USART���б�־
	USART_ClearITPendingBit(USART1, USART_IT_IDLE);
    // ʹ��USART�����ж�
	USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);
	
    // ���USART�������
    USART_ClearITPendingBit(USART1, USART_IT_TC);
    // ʹ��USART��������ж�
	USART_ITConfig(USART1, USART_IT_TC, ENABLE);
}

void usb2uart_uart_send_bydma(uint8_t *data, uint32_t len)
{
	NVIC_InitTypeDef NVIC_InitStructure;
    DMA_InitTypeDef dma_init_struct;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
	
	g_uart_tx_transfer_length = len;
	
    DMA_DeInit(DMA2_Stream7);
    
    dma_init_struct.DMA_Channel = DMA_Channel_4;
    dma_init_struct.DMA_PeripheralBaseAddr = USART1_BASE + 0x04;
    dma_init_struct.DMA_Memory0BaseAddr = (uint32_t)data;
    dma_init_struct.DMA_DIR = DMA_DIR_MemoryToPeripheral;
    dma_init_struct.DMA_BufferSize = len;
    dma_init_struct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    dma_init_struct.DMA_MemoryInc = DMA_MemoryInc_Enable;
    dma_init_struct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    dma_init_struct.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    dma_init_struct.DMA_Mode = DMA_Mode_Normal;
    dma_init_struct.DMA_Priority = DMA_Priority_High;
    dma_init_struct.DMA_FIFOMode = DMA_FIFOMode_Disable;
    dma_init_struct.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
    dma_init_struct.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
    dma_init_struct.DMA_MemoryBurst = DMA_MemoryBurst_Single;
    
    DMA_Init(DMA2_Stream7, &dma_init_struct);
	
	/* ���� DMA �ж� */
	/* Ƕ�������жϿ�������ѡ�� */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

	/* ����USARTΪ�ж�Դ */
	NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream7_IRQn;
	/* �������ȼ�Ϊ1 */
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	/* �����ȼ�Ϊ1 */
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	/* ʹ���ж� */
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	/* ��ʼ������NVIC */
	NVIC_Init(&NVIC_InitStructure);
	
	//ʹ��DMA��������ж�
	DMA_ITConfig(DMA2_Stream7, DMA_IT_TC, ENABLE);
	
	USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);
	
	//ʹ�� DMA ͨ��
    DMA_Cmd(DMA2_Stream7, ENABLE);
}

// CDC UART���ûص�����
void usb2uart_uart_config_callback(struct cdc_line_coding *line_coding)
{
	// ����USART1
    USART_Cmd(USART1, DISABLE);
	
	USART_DeInit(USART1);
	
    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate = line_coding->dwDTERate;
    USART_InitStructure.USART_WordLength = (line_coding->bDataBits == 9) ? USART_WordLength_9b : USART_WordLength_8b;
	
    USART_InitStructure.USART_StopBits = (line_coding->bCharFormat == 1) ? USART_StopBits_1_5 : 
                                         (line_coding->bCharFormat == 2) ? USART_StopBits_2 : USART_StopBits_1;
	
    USART_InitStructure.USART_Parity = (line_coding->bParityType == 1) ? USART_Parity_Odd :
                                       (line_coding->bParityType == 2) ? USART_Parity_Even : USART_Parity_No;
	
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

    // Ӧ������
    USART_Init(USART1, &USART_InitStructure);

    // ʹ��USART1
    USART_Cmd(USART1, ENABLE);

    // ����DMA���գ�����usb2uart_rx_dma_config()�����Ѿ�ʵ�֣�
    usb2uart_rx_dma_config();
}

//USART1 rx
void DMA2_Stream4_IRQHandler(void)
{
	if (DMA_GetITStatus(DMA2_Stream4, DMA_IT_TCIF4) == SET) {
		DMA_ClearITPendingBit(DMA2_Stream4, DMA_IT_TCIF4);
	}
}

//usart1 tx
void DMA2_Stream7_IRQHandler(void)
{
	if (DMA_GetITStatus(DMA2_Stream7, DMA_IT_TCIF7) == SET) {
		extern void chry_dap_usb2uart_uart_send_complete(uint32_t size);
		
		chry_dap_usb2uart_uart_send_complete(g_uart_tx_transfer_length);
		DMA_ClearITPendingBit(DMA2_Stream7, DMA_IT_TCIF7);
	}
}

extern chry_ringbuffer_t g_uart_rx;

void USART1_IRQHandler(void)
{
    static uint32_t receive_len = 0;
    
    if (SET == USART_GetITStatus(USART1, USART_IT_IDLE)) {
        USART1->SR;
        USART1->DR;
		
		//���� DMA ����
		DMA_Cmd(DMA2_Stream2, DISABLE);
        
        receive_len = DMA2_Stream2->NDTR;
        
        /* �����յ������ݷŵ����λ������� */
        chry_ringbuffer_write(&g_uart_rx, receive, sizeof(receive) - receive_len);
        
        usb2uart_rx_dma_config();
    }
	
	//��� USART ��������жϱ�־
	if (SET == USART_GetITStatus(USART1, USART_IT_TC)) {
		USART_ClearITPendingBit(USART1, USART_IT_TC);
	}
}

/*************************************************************/

///�ض���c�⺯��printf�����ڣ��ض�����ʹ��printf����
int fputc(int ch, FILE *f)
{
		/* ����һ���ֽ����ݵ����� */
		USART_SendData(DEBUG_USART, (uint8_t) ch);
		
		/* �ȴ�������� */
		while (USART_GetFlagStatus(DEBUG_USART, USART_FLAG_TXE) == RESET);		
	
		return (ch);
}

///�ض���c�⺯��scanf�����ڣ���д����ʹ��scanf��getchar�Ⱥ���
int fgetc(FILE *f)
{
		/* �ȴ������������� */
		while (USART_GetFlagStatus(DEBUG_USART, USART_FLAG_RXNE) == RESET);

		return (int)USART_ReceiveData(DEBUG_USART);
}
/*********************************************END OF FILE**********************/

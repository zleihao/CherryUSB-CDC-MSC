#include "stm32f4xx.h"
#include "RTOS.h"
#include "stdio.h"
#include "usb_config.h"
#include "bsp_spi_flash.h"
#include "usart.h"
//==============================================================================
#include "ff.h"
#include "string.h"


void usb_dc_low_level_init(uint8_t busid)
{
  GPIO_InitTypeDef GPIO_InitStructure;   

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA , ENABLE);
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11 | 
                                GPIO_Pin_12;
  
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_Init(GPIOA, &GPIO_InitStructure);  
  
  GPIO_PinAFConfig(GPIOA,GPIO_PinSource11,GPIO_AF_OTG1_FS) ; 
  GPIO_PinAFConfig(GPIOA,GPIO_PinSource12,GPIO_AF_OTG1_FS) ;

  
  RCC_AHB2PeriphClockCmd( RCC_AHB2Periph_OTG_FS, ENABLE) ;

  //中断
  NVIC_InitTypeDef NVIC_InitStructure; 
  
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
  NVIC_InitStructure.NVIC_IRQChannel = OTG_FS_IRQn;

  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);  
#ifdef USB_OTG_HS_DEDICATED_EP1_ENABLED
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
  NVIC_InitStructure.NVIC_IRQChannel = OTG_HS_EP1_OUT_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);  
  
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
  NVIC_InitStructure.NVIC_IRQChannel = OTG_HS_EP1_IN_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);   
#endif
}

void usb_dc_low_level_deinit(uint8_t busid)
{


}

/**
* @brief  USB_OTG_BSP_uDelay
*         This function provides delay time in micro sec
* @param  usec : Value of delay required in micro sec
* @retval None
*/
//void USB_OTG_BSP_uDelay (const uint32_t usec)
//{
//  uint32_t count = 0;
//  const uint32_t utime = (14 * usec);
//  do
//  {
//    if ( ++count > utime )
//    {
//      return ;
//    }
//  }
//  while (1);
//}


/**
* @brief  USB_OTG_BSP_mDelay
*          This function provides delay time in milli sec
* @param  msec : Value of delay required in milli sec
* @retval None
*/
void usbd_dwc2_delay_ms(uint8_t ms)
{
  //USB_OTG_BSP_uDelay(ms * 1000);   
}

uint32_t usbd_get_dwc2_gccfg_conf(uint32_t reg_base)
{
  return ((1 << 16) | (1 << 21));
}

int kprintf(const char *fmt, ...)
{
	/* todo */
	return 0;
}

extern int led_init(void);
void bsp_init(void)
{
    led_init();
	Debug_USART_Config();
}

FATFS fs;													/* FatFs文件系统对象 */
uint8_t work[4096];

/**
  * @brief  主函数
  * @param  无
  * @retval 无
  */
int main(void)
{
    bsp_init();
	
	f_mount(&fs, "1:", 1);
	

    AppTaskCreate();
    
	while (1) {
        ;
	}
}


void OTG_FS_IRQHandler(void)
{
	extern void USBD_IRQHandler(uint8_t busid);
	USBD_IRQHandler(0);
}

/*********************************************END OF FILE**********************/


#include "RTOS.h"
#include "stm32f4xx.h"

int led_init(void)
{
    /*定义一个GPIO_InitTypeDef类型的结构体*/
    GPIO_InitTypeDef GPIO_InitStructure;

    /*开启LED相关的GPIO外设时钟*/
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

    /*选择要控制的GPIO引脚*/
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;

    /*设置引脚模式为输出模式*/
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;

    /*设置引脚的输出类型为推挽输出*/
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;

    /*设置引脚为上拉模式*/
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;

    /*设置引脚速率为2MHz */
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    /*调用库函数，使用上面配置的GPIO_InitStructure初始化GPIO*/
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    return 0;
}

static StaticTask_t led1_task_tcb;

static void led1_task(void *param)
{
    for (;;) {
        GPIO_ResetBits(GPIOB, GPIO_Pin_2);
        vTaskDelay(500);

        GPIO_SetBits(GPIOB, GPIO_Pin_2);
        vTaskDelay(500);
    }
}

void led_task_start()
{
    xTaskCreate(led1_task, "led1_task", 128, NULL, 2, (TaskHandle_t *)&led1_task_tcb);
}
INIT_APP_TASK(led_task_start);

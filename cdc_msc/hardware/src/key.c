#include "stm32f4xx.h"
#include "RTOS.h"

int key_init(void)
{
    /*定义一个GPIO_InitTypeDef类型的结构体*/
    GPIO_InitTypeDef GPIO_InitStructure;

    /*开启LED相关的GPIO外设时钟*/
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

    /*选择要控制的GPIO引脚*/
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    /*设置引脚模式为输出模式*/
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    /*设置引脚的输出类型为推挽输出*/
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    /*设置引脚为上拉模式*/
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;

    /*设置引脚速率为2MHz */
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    /*调用库函数，使用上面配置的GPIO_InitStructure初始化GPIO*/
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    return 0;
}

typedef enum {
    IDLE = 0,
    DEBOUNCE,
    PRESSEN,
    LONG_CHECK,
} key_state_t;

static key_state_t key_state = IDLE;
static uint16_t key_timer = 0;
#define LONG_PRESS_TIME 100

uint8_t key_short_press = 0;
uint8_t key_long_press = 0;

void key_scan(void)
{
    uint8_t key_down = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0);

    switch (key_state) {
    case IDLE:
        key_state = DEBOUNCE;
        key_timer = 0;
        break;

    case DEBOUNCE:
        key_timer++;
        if (key_timer >= 2) {
            if (key_down) { // 再次确认按键还在按着，防止抖动
                key_state = PRESSEN;
                key_timer = 0;
            } else {
                key_state = IDLE;
            }
        }
        break;

    case PRESSEN:
        if (key_down) {
            key_timer++;
            if (key_timer >= LONG_PRESS_TIME) {
                key_state = LONG_CHECK;
            }
        } else {
            key_state = IDLE;
            key_short_press = 1;
        }
        break;

    case LONG_CHECK:
        if (!key_down) {
            key_state = IDLE;
            key_long_press = 1;
        }
        break;
    }
}

void key_task(void *pvParameters)
{
    for (;;) {
        key_scan();

        vTaskDelay(10); // 每10ms扫描一次
    }
}

static StaticTask_t key_task_tcb;

void key_task_start()
{
    xTaskCreate(key_task, "key_task", 128, NULL, 4, (TaskHandle_t *)&key_task_tcb);
}
INIT_APP_TASK(key_task_start);
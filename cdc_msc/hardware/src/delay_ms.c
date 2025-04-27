#include "delay_ms.h"
#include <stdint.h>

//定时器
void timer_init(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);

    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_Period = 0xFFFF;
    TIM_TimeBaseStructure.TIM_Prescaler = (84000000 / 10000) - 1;

    TIM_TimeBaseInit(TIM6, &TIM_TimeBaseStructure);
}

void delay_ms(uint16_t tms)
{
    uint32_t counter = tms * 10;

    if (counter > 0xFFFF) {
        counter = 0xFFFF;
    }

    TIM6->CNT = 0;
    TIM_ClearFlag(TIM6, TIM_FLAG_Update); // 清除溢出标志

    TIM_Cmd(TIM6, ENABLE);

    while (TIM6->CNT < counter) {
        ;
    }

    TIM_Cmd(TIM6, DISABLE);
}
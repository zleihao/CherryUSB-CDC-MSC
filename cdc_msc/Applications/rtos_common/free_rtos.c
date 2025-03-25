#include "RTOS.h"

/* 不要修改这两个变量 */
int task_index;
init_task_start_t task_create_func[USER_TASK_NUM];

void AppTaskCreate(void)
{
    uint8_t i;
    taskENTER_CRITICAL();

    /* 依次创建任务 */
    for (i = 0; i < USER_TASK_NUM; i++) {
        if (task_create_func[i] == NULL) {
            continue;
        }
        (*task_create_func[i])();
    }

    taskEXIT_CRITICAL();

    vTaskStartScheduler();
}

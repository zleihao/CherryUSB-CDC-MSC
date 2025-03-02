#ifndef __RTOS_H
#define __RTOS_H

#include "FreeRTOS.h"
#include "task.h"

/* 工程中的 Task 总个数 */
#define USER_TASK_NUM    3

typedef void (*init_task_start_t)();

extern init_task_start_t task_create_func[USER_TASK_NUM];
extern int task_index;

/* 用来创建任务 */
#define INIT_APP_TASK(func)                                               \
    __attribute__((constructor)) static void auto_register_##func(void) { \
        if (task_index < USER_TASK_NUM) {                                 \
            task_create_func[task_index++] = func;                        \
        }                                                                 \
    }
    
void AppTaskCreate(void);

#endif

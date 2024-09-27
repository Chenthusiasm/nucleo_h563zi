/* Includes --------------------------------------------------------------------------------------*/

#include <stdbool.h>

#include "app_freertos.h"
#include "Mutex.h"
#include "RTOS.h"
#include "sys_command_line.h"


/* Internal typedef ------------------------------------------------------------------------------*/


/* Internal define -------------------------------------------------------------------------------*/

#define MUTEX_TIMEOUT_MS                (5u)
#define DELAY_MS                        (500u)
#define FINAL_DELAY_MS                  (2000u)


/* Internal macro --------------------------------------------------------------------------------*/


/* Internal variables ----------------------------------------------------------------------------*/

static Mutex mutex0;
static Mutex mutex1;


/* Internal function prototypes ------------------------------------------------------------------*/


/* Internal functions ----------------------------------------------------------------------------*/

static void initMutex(void) {
    ;
}

static void printMutexAquiredState(void) {
    printf("    mutex0=%d; mutex1=%d\n", mutex0.acquired, mutex1.acquired);
}


static void printResult(char const* function, bool result) {
    printf("    %s=%d\n", function, result);
}


/* External functions ----------------------------------------------------------------------------*/

void MainAppTask_Start(void *argument) {
    initMutex();
    for (;;) {
#if 0
        bool result;
        printf(">>> [%lu] start [line=%d]\n", osKernelGetTickCount(), __LINE__);
        printMutexAquiredState();
        osDelay(RTOS_ConvertMSToTicks(DELAY_MS));

        printf(">>> [%lu] acquire mutex0 [line=%d]\n", osKernelGetTickCount(), __LINE__);
        result = Mutex_Acquire(&mutex0, MUTEX_TIMEOUT_MS);
        printResult("Mutex_Acquire(&mutex0)", result);
        printMutexAquiredState();
        osDelay(RTOS_ConvertMSToTicks(DELAY_MS));

        printf(">>> [%lu] acquire mutex0 [line=%d]\n", osKernelGetTickCount(), __LINE__);
        result = Mutex_Acquire(&mutex0, MUTEX_TIMEOUT_MS);
        printResult("Mutex_Acquire(&mutex0)", result);
        printMutexAquiredState();
        osDelay(RTOS_ConvertMSToTicks(DELAY_MS));

        printf(">>> [%lu] acquire mutex1 [line=%d]\n", osKernelGetTickCount(), __LINE__);
        result = Mutex_Acquire(&mutex1, MUTEX_TIMEOUT_MS);
        printResult("Mutex_Acquire(&mutex1)", result);
        printMutexAquiredState();
        osDelay(RTOS_ConvertMSToTicks(DELAY_MS));

        printf(">>> [%lu] release mutex0 [line=%d]\n", osKernelGetTickCount(), __LINE__);
        result = Mutex_Release(&mutex0);
        printResult("Mutex_Release(&mutex0)", result);
        printMutexAquiredState();
        osDelay(RTOS_ConvertMSToTicks(DELAY_MS));

        printf(">>> [%lu] acquire mutex1 [line=%d]\n", osKernelGetTickCount(), __LINE__);
        result = Mutex_Acquire(&mutex1, MUTEX_TIMEOUT_MS);
        printResult("Mutex_Acquire(&mutex1)", result);
        printMutexAquiredState();
        osDelay(RTOS_ConvertMSToTicks(DELAY_MS));

        printf(">>> [%lu] acquire mutex0 [line=%d]\n", osKernelGetTickCount(), __LINE__);
        result = Mutex_Acquire(&mutex0, MUTEX_TIMEOUT_MS);
        printResult("Mutex_Acquire(&mutex0)", result);
        printMutexAquiredState();
        osDelay(RTOS_ConvertMSToTicks(DELAY_MS));

        printf(">>> [%lu] acquire mutex1 [line=%d]\n", osKernelGetTickCount(), __LINE__);
        result = Mutex_Acquire(&mutex1, MUTEX_TIMEOUT_MS);
        printResult("Mutex_Acquire(&mutex1)", result);
        printMutexAquiredState();
        osDelay(RTOS_ConvertMSToTicks(DELAY_MS));

        printf(">>> [%lu] reset [line=%d]\n", osKernelGetTickCount(), __LINE__);
        result = Mutex_Release(&mutex0);
        printResult("Mutex_Release(&mutex0)", result);
        result = Mutex_Release(&mutex1);
        printResult("Mutex_Release(&mutex1)", result);
        printMutexAquiredState();
        osDelay(RTOS_ConvertMSToTicks(FINAL_DELAY_MS));
        printf("\n");
#else
        osDelay(RTOS_ConvertMSToTicks(1u));
#endif
    }
}


/**
 *  @brief Initialization for the MainApp task.
 */
void MainAppTask_init(void) {
    ;
}


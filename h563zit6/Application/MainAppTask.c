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
#define LONG_DELAY_MS                   (5000u)


/* Internal macro --------------------------------------------------------------------------------*/


/* Internal variables ----------------------------------------------------------------------------*/

static Mutex mutex0;
static Mutex mutex1;


/* Internal function prototypes ------------------------------------------------------------------*/


/* Internal functions ----------------------------------------------------------------------------*/

static void initMutex(void) {
#if 0
    printf("initMutex()\n");
    mutex0 = Mutex_ctor(NULL);
    mutex1 = Mutex_ctor(NULL);

//    mutex0 = Mutex_ctor();
//    mutex1 = Mutex_ctor();
#endif
}

static void printMutexAquiredState(void) {
    printf("    mutex0=%d; mutex1=%d\n", mutex0.acquired, mutex1.acquired);
}


static void printResult(char const* function, bool result) {
    printf("    %s=%d\n", function, result);
}


/* External functions ----------------------------------------------------------------------------*/

/**
 *  @brief  The main entry point for the Main App task.
 *  The main entry point for the Main App task. At the moment, the Main App task is only used to
 *  test the driver code.
 *  @param[in]  argument    TODO
 */
void MainAppTask_Start(void *argument) {
#if 0
    initMutex();
    for (;;) {
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

//        printf(">>> [%lu] NULL [line=%d]\n", osKernelGetTickCount(), __LINE__);
//        result = Mutex_Acquire(NULL, MUTEX_TIMEOUT_MS);
//        printResult("Mutex_Acquire(NULL)", result);
    }
#else
    uint32_t count = 0u;
    for (;;) {
        printf("MainAppTask[%lu]\n", count++);
        osDelay(RTOS_ConvertMSToTicks(LONG_DELAY_MS));
    }
#endif
}


/**
 *  @brief Initialization for the MainApp task.
 */
void MainAppTask_Init(void) {
    ;
}

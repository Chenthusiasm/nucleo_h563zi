/* Includes --------------------------------------------------------------------------------------*/

#include <stdbool.h>

#include "app_freertos.h"
#include "Mutex.h"
#include "sys_command_line.h"
#include "RTOSHelper.h"


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
    printf("initMutex()\n");
    mutex0 = Mutex_ctor(TestMutexHandle);
    mutex1 = Mutex_ctor(TestMutexHandle);
}

static void printMutexAquiredState(void) {
    printf("    mutex0=%d; mutex1=%d\n", Mutex_IsAcquired(&mutex0), Mutex_IsAcquired(&mutex1));
}


static void printResult(char const* function, bool result) {
    printf("    %s=%d\n", function, result);
}


/* External functions ----------------------------------------------------------------------------*/

void MainAppTask_Start(void *argument) {
    initMutex();
    for (;;) {
        bool result;
        printf(">>> [%lu] start [line=%d]\n", osKernelGetTickCount(), __LINE__);
        printMutexAquiredState();
        osDelay(RTOSHelper_ConvertMSToTicks(DELAY_MS));

        printf(">>> [%lu] acquire mutex0 [line=%d]\n", osKernelGetTickCount(), __LINE__);
        result = Mutex_Acquire(&mutex0, MUTEX_TIMEOUT_MS);
        printResult("Mutex_Acquire(&mutex0)", result);
        printMutexAquiredState();
        osDelay(RTOSHelper_ConvertMSToTicks(DELAY_MS));

        printf(">>> [%lu] acquire mutex0 [line=%d]\n", osKernelGetTickCount(), __LINE__);
        result = Mutex_Acquire(&mutex0, MUTEX_TIMEOUT_MS);
        printResult("Mutex_Acquire(&mutex0)", result);
        printMutexAquiredState();
        osDelay(RTOSHelper_ConvertMSToTicks(DELAY_MS));

        printf(">>> [%lu] acquire mutex1 [line=%d]\n", osKernelGetTickCount(), __LINE__);
        result = Mutex_Acquire(&mutex1, MUTEX_TIMEOUT_MS);
        printResult("Mutex_Acquire(&mutex1)", result);
        printMutexAquiredState();
        osDelay(RTOSHelper_ConvertMSToTicks(DELAY_MS));

        printf(">>> [%lu] release mutex0 [line=%d]\n", osKernelGetTickCount(), __LINE__);
        result = Mutex_Release(&mutex0);
        printResult("Mutex_Release(&mutex0)", result);
        printMutexAquiredState();
        osDelay(RTOSHelper_ConvertMSToTicks(DELAY_MS));

        printf(">>> [%lu] acquire mutex1 [line=%d]\n", osKernelGetTickCount(), __LINE__);
        result = Mutex_Acquire(&mutex1, MUTEX_TIMEOUT_MS);
        printResult("Mutex_Acquire(&mutex1)", result);
        printMutexAquiredState();
        osDelay(RTOSHelper_ConvertMSToTicks(DELAY_MS));

        printf(">>> [%lu] acquire mutex0 [line=%d]\n", osKernelGetTickCount(), __LINE__);
        result = Mutex_Acquire(&mutex0, MUTEX_TIMEOUT_MS);
        printResult("Mutex_Acquire(&mutex0)", result);
        printMutexAquiredState();
        osDelay(RTOSHelper_ConvertMSToTicks(DELAY_MS));

        printf(">>> [%lu] acquire mutex1 [line=%d]\n", osKernelGetTickCount(), __LINE__);
        result = Mutex_Acquire(&mutex1, MUTEX_TIMEOUT_MS);
        printResult("Mutex_Acquire(&mutex1)", result);
        printMutexAquiredState();
        osDelay(RTOSHelper_ConvertMSToTicks(DELAY_MS));

        printf(">>> [%lu] reset [line=%d]\n", osKernelGetTickCount(), __LINE__);
        result = Mutex_Release(&mutex0);
        printResult("Mutex_Release(&mutex0)", result);
        result = Mutex_Release(&mutex1);
        printResult("Mutex_Release(&mutex1)", result);
        printMutexAquiredState();
        osDelay(RTOSHelper_ConvertMSToTicks(FINAL_DELAY_MS));
        printf("\n");
    }
}

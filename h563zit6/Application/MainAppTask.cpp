/* Includes --------------------------------------------------------------------------------------*/

#include "MainAppTask.h"

#include <stdbool.h>

#include "app_freertos.h"
#include "Mutex.h"
#include "RTOS.h"
#include "sys_command_line.h"
#include "spi.h"
#include "DiagnosticsQ.h"
#include "MainAppQ.h"


/* Internal typedef ------------------------------------------------------------------------------*/


/* Internal define -------------------------------------------------------------------------------*/

#define TASK_POLL_MS                    (1u)
#define TASK_POLL_TICKS                 (RTOS_ConvertMSToTicks(TASK_POLL_MS))
#define DEQUEUE_BATCH_BUDGET_MS         (3u)
#define DEQUEUE_BATCH_BUDGET_TICKS      (RTOS_ConvertMSToTicks(DEQUEUE_BATCH_BUDGET_MS))
#define HEARTBEAT_PERIOD_MS             (1000u)
#define HEARTBEAT_PERIOD_TICKS          (RTOS_ConvertMSToTicks(HEARTBEAT_PERIOD_MS))


/* Internal macro --------------------------------------------------------------------------------*/


/* Internal variables ----------------------------------------------------------------------------*/


/* Internal function prototypes ------------------------------------------------------------------*/


/* Internal functions ----------------------------------------------------------------------------*/

static void userButtonPressed(void) {
    HAL_GPIO_WritePin(LD2_YELLOW_GPIO_Port, LD2_YELLOW_Pin, GPIO_PIN_SET);
    DiagQ_Log(DiagSource_MainApp, "[USER] pressed");
}

static void userButtonReleased(void) {
    HAL_GPIO_WritePin(LD2_YELLOW_GPIO_Port, LD2_YELLOW_Pin, GPIO_PIN_RESET);
    DiagQ_Log(DiagSource_MainApp, "[USER] released");
}

static void processMessage(MainAppMsg_t const * message) {
    if (!message) {
        return;
    }
    switch (message->event) {
    case MainAppEvent_Sleep:
        osDelay(RTOS_ConvertMSToTicks(message->content.sleepTime_ms));
        break;
    case MainAppEvent_UserButtonChange:
        if (message->content.buttonTransition == ButtonTransition_Released) {
            userButtonReleased();
        }
        else if (message->content.buttonTransition == ButtonTransition_Pressed) {
            userButtonPressed();
        }
        break;
    default:
        // do nothing
        ;
    }
}

static void processHeartbeatLED(void) {
    static uint32_t lastToggleTime_ms = 0u;
    uint32_t time_ms = (osKernelGetTickCount() * 1000 ) / osKernelGetTickFreq();
    if ((uint32_t)(time_ms - lastToggleTime_ms) >= (HEARTBEAT_PERIOD_TICKS / 2)) {
        HAL_GPIO_TogglePin(LD1_GREEN_GPIO_Port, LD1_GREEN_Pin);
        lastToggleTime_ms = time_ms;
    }
}


/* External functions ----------------------------------------------------------------------------*/

/**
 *  @brief  The main entry point for the Main App task.
 *  The main entry point for the Main App task. At the moment, the Main App task is only used to
 *  test the driver code.
 *  @param[in]  argument    TODO
 */
void MainAppTask_Start(void *argument) {
    for (;;) {
        uint32_t const batchStart = osKernelGetTickCount();
        while ((osKernelGetTickCount() - batchStart) < DEQUEUE_BATCH_BUDGET_TICKS) {
            MainAppMsg_t message;
            if (osMessageQueueGet(MainAppQHandle, &message, nullptr, 0) != osOK) {
                break; // queue's empty, don't continue spinning
            }
            processMessage(&message);
        }
        processHeartbeatLED();
        osDelay(TASK_POLL_TICKS);
    }
}


/**
 *  @brief Initialization for the MainApp task.
 */
void MainAppTask_Init(void) {
}


/**
 * @brief General EXTI rising edge callback.
 *
 * @param[in] GPIO_Pin The GPIO pin that triggered the interrupt.
 */
void HAL_GPIO_EXTI_Rising_Callback(uint16_t GPIO_Pin) {
    if (GPIO_Pin == B1_USER_Pin) {
        // button polarity is inverted
        MainAppQ_UserButtonPressed();
    }
}


/**
 * @brief General EXTI falling edge callback.
 *
 * @param[in] GPIO_Pin The GPIO pin that triggered the interrupt.
 */
void HAL_GPIO_EXTI_Falling_Callback(uint16_t GPIO_Pin) {
    if (GPIO_Pin == B1_USER_Pin) {
        // button polarity is inverted
        MainAppQ_UserButtonReleased();
    }
}

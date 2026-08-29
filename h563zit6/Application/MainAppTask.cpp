/* Includes --------------------------------------------------------------------------------------*/

#include "MainAppTask.h"

#include <stdbool.h>

#include "app_freertos.h"
#include "Mutex.h"
#include "RTOS.h"
#include "sys_command_line.h"
#include "AD4080.h"
#include "spi.h"
#include "DiagnosticsQ.h"
#include "MainAppQ.h"


/* Internal typedef ------------------------------------------------------------------------------*/


/* Internal define -------------------------------------------------------------------------------*/

#define MUTEX_TIMEOUT_MS                (5u)
#define DELAY_MS                        (500u)
#define FINAL_DELAY_MS                  (2000u)
#define LONG_DELAY_MS                   (5000u)
#define TASK_POLL_MS                    (5u)


/* Internal macro --------------------------------------------------------------------------------*/


/* Internal variables ----------------------------------------------------------------------------*/

static AD4080_Handle ad4080;

static uint8_t ad4080ScratchValue = 0;


/* Internal function prototypes ------------------------------------------------------------------*/


/* Internal functions ----------------------------------------------------------------------------*/

static void initAD4080(void) {
    AD4080_Init(&ad4080, &hspi1, SPI_CS_GPIO_Port, SPI_CS_Pin);
}

static void userButtonPressed(void) {
    HAL_GPIO_WritePin(LD2_YELLOW_GPIO_Port, LD2_YELLOW_Pin, GPIO_PIN_SET);
    AD4080_ScratchPadLoopback(&ad4080, ad4080ScratchValue++);
    DiagQ_printf("[USER] pressed" ENDL);
}

static void userButtonReleased(void) {
    HAL_GPIO_WritePin(LD2_YELLOW_GPIO_Port, LD2_YELLOW_Pin, GPIO_PIN_RESET);
    AD4080_VerifyChipID(&ad4080);
    DiagQ_printf("[USER] released" ENDL);
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
    if ((uint32_t)(time_ms - lastToggleTime_ms) >= DELAY_MS) {
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
        processHeartbeatLED();
        MainAppMsg_t message;
        if (osMessageQueueGet(MainAppQHandle, &message, NULL, RTOS_ConvertMSToTicks(TASK_POLL_MS)) == osOK) {
            processMessage(&message);
        }
        osDelay(RTOS_ConvertMSToTicks(TASK_POLL_MS));
    }
}


/**
 *  @brief Initialization for the MainApp task.
 */
void MainAppTask_Init(void) {
    initAD4080();
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
        //userButtonPressed();
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
        //userButtonReleased();
    }
}

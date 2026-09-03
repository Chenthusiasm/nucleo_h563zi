/* Includes --------------------------------------------------------------------------------------*/

#include "MainAppTask.h"

#include <stdbool.h>
#include <type_traits>

#include "app_freertos.h"
#include "Mutex.h"
#include "RTOS.h"
#include "sys_command_line.h"
#include "AD408xConfig.hpp"
#include "AD408xRegisters.hpp"
#include "spi.h"
#include "DiagnosticsQ.h"
#include "MainAppQ.h"
#include "ADCCmdDefines.h"


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

static bool adcReady = false;
static bool adcTestedReadAll = false;

static AD408x::Config adcConfig(&hspi1, SPI_CS_GPIO_Port, SPI_CS_Pin);

static uint8_t ad408xScratchValue = 0;


/* Internal function prototypes ------------------------------------------------------------------*/


/* Internal functions ----------------------------------------------------------------------------*/

static void checkADCReady() {
    if (adcReady) {
        // ADC already confirmed ready, return early
        return;
    }
    AD408x::INTERFACE_STATUS_A::Fields fields = adcConfig.Read<AD408x::INTERFACE_STATUS_A>();
    adcReady = (fields.NOT_READY_ERR == 0);
    if (adcReady) {
        DiagQ_Log(DiagSource_MainApp, "AD408x ADC is ready!");
        
    }
}

static void userButtonPressed(void) {
    HAL_GPIO_WritePin(LD2_YELLOW_GPIO_Port, LD2_YELLOW_Pin, GPIO_PIN_SET);
    if (adcReady) {
        if (adcTestedReadAll) {
            adcConfig.ScratchPadLoopback(ad408xScratchValue++);
        } else {
            int invalid = adcConfig.TestReadAll();
            adcTestedReadAll = true;
            DiagQ_Log(DiagSource_MainApp, "AD408x: %d registers didn't match the reset values", invalid);
        }
    }
    DiagQ_Log(DiagSource_MainApp, "[USER] pressed");
}

static void userButtonReleased(void) {
    HAL_GPIO_WritePin(LD2_YELLOW_GPIO_Port, LD2_YELLOW_Pin, GPIO_PIN_RESET);
    if (adcReady && adcTestedReadAll) {
        adcConfig.VerifyChipID();
    }
    DiagQ_Log(DiagSource_MainApp, "[USER] released");
}

static void processADCCmd(ADCCmd_t adcCmd) {
    switch (adcCmd.subcmd) {
    case ADCCmdSubcmd_scratchpad:
        adcConfig.ScratchPadLoopback(adcCmd.values.scratchpad_val);
        break;
    case ADCCmdSubcmd_checkdefaults:
        adcConfig.TestReadAll();
        break;
    case ADCCmdSubcmd_info:
        adcConfig.VerifyChipID();
        break;
    default:
        DiagQ_Log(DiagSource_MainApp, "\"adc\" subcmd value %d not recognized", adcCmd.subcmd);
    }
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
    case MainAppEvent_ADCCmd:
        processADCCmd(message->content.adcCmd);
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
        checkADCReady();
        processHeartbeatLED();
        osDelay(TASK_POLL_TICKS);
    }
}


/**
 *  @brief Initialization for the MainApp task.
 */
void MainAppTask_Init(void) {
    adcConfig.Init();
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

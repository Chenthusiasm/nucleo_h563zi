/* Includes --------------------------------------------------------------------------------------*/

#include "DiagnosticsTask.h"

#include "DiagnosticsQ.h"
#include "app_freertos.h"
#include "ICACHE.h"
#include "RTOS.h"
#include "sys_command_line.h"
#include "usart.h"
#include "usbd_cdc_if.h"


/* Internal typedef ------------------------------------------------------------------------------*/


/* Internal define -------------------------------------------------------------------------------*/

#define TASK_POLL_MS                    (2u)
#define TASK_POLL_TICKS                 (RTOS_ConvertMSToTicks(TASK_POLL_MS))
#define DEQUEUE_BATCH_BUDGET_MS         (5u)
#define DEQUEUE_BATCH_BUDGET_TICKS      (RTOS_ConvertMSToTicks(DEQUEUE_BATCH_BUDGET_MS))


/* Internal macro --------------------------------------------------------------------------------*/


/* Internal variables ----------------------------------------------------------------------------*/


/* Internal function prototypes ------------------------------------------------------------------*/


/* Internal functions ----------------------------------------------------------------------------*/

static void printMessage(DiagMsg_t const* message) {
    uint32_t ms = (message->timestamp_ticks * 1000 ) / osKernelGetTickFreq();
    uint32_t ms_int = ms / 1000u;
    uint32_t ms_fraction = ms % 1000u;
    if (message->source == DiagSource_printf) {
        // print message as-is
        printf("[%lu.%03lums] %.*s",
            ms_int, ms_fraction, message->len, message->text);
    }
    else {
        // print message with ENDL appended so the message is printed on one line
        printf("[%lu.%03lums:%s] %.*s" ENDL,
            ms_int, ms_fraction, DiagQ_GetSourceStr(message->source),
            message->len, message->text);
    }
}


/* External functions ----------------------------------------------------------------------------*/

/**
 *  @brief  The main entry point for the Diagnostics task.
 *  The main entry point for the Diagnostics task. At the moment, the Diagnostics task only
 *  processes command line arguments.
 *  @param[in]  argument    TODO
 */
void DiagnosticsTask_Start(void *argument) {
    for (;;) {
        uint32_t const batchStart = osKernelGetTickCount();
        bool printedBatch = false;
        while ((osKernelGetTickCount() - batchStart) < DEQUEUE_BATCH_BUDGET_TICKS) {
            CLI_RUN();
            DiagMsg_t message;
            if (osMessageQueueGet(DiagnosticsQHandle, &message, nullptr, 0) != osOK) {
                break; // queue's empty, don't continue spinning
            }
            if (!printedBatch) {
                // the terminal is currently sitting at a bare prompt left over from the last command or the last batch
                // of asynchronous log output; move to a new line before printing so log text doesn't run onto the same
                // line as the prompt
                NL1();
            }
            printMessage(&message);
            printedBatch = true;
        }
        if (printedBatch) {
            // redraw the prompt so it doesn't get left behind under asynchronous log output, similar to how the Linux
            // terminal reprints the prompt once background output finishes
            PRINT_CLI_NAME();
        }
        osDelay(TASK_POLL_TICKS);
    }
}


/**
 *  @brief  Initialization for the Diagnostics task.
 */
void DiagnosticsTask_Init(void) {
    ICACHE_Init();
    USBD_StatusTypeDef status = USB_CDC_Init();
    (void) status;
    CLI_INIT();
}

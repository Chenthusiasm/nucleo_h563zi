/* Includes --------------------------------------------------------------------------------------*/

#include "app_freertos.h"
#include "RTOS.h"
#include "sys_command_line.h"
#include "usart.h"


/* Internal typedef ------------------------------------------------------------------------------*/


/* Internal define -------------------------------------------------------------------------------*/


/* Internal macro --------------------------------------------------------------------------------*/


/* Internal variables ----------------------------------------------------------------------------*/


/* Internal function prototypes ------------------------------------------------------------------*/


/* Internal functions ----------------------------------------------------------------------------*/


/* External functions ----------------------------------------------------------------------------*/

/**
 *  @brief  The main entry point for the Diagnostics task.
 *  The main entry point for the Diagnostics task. At the moment, the Diagnostics task only
 *  processes command line arguments.
 *  @param[in]  argument    TODO
 */
void DiagnosticsTask_Start(void *argument) {
    for (;;) {
        CLI_RUN();
        osDelay(RTOS_ConvertMSToTicks(1u));
    }
}


/**
 *  @brief  Initialization for the Diagnostics task.
 */
void DiagnosticsTask_Init(void) {
    CLI_INIT(&huart3, USART3_IRQn);
}

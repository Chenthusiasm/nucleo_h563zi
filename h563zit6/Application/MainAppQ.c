/* Includes --------------------------------------------------------------------------------------*/

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>

#include "MainAppQ.h"

#include "app_freertos.h"
#include "RTOS.h"


/* Internal typedef ------------------------------------------------------------------------------*/


/* Internal define -------------------------------------------------------------------------------*/


/* Internal macro --------------------------------------------------------------------------------*/


/* Internal variables ----------------------------------------------------------------------------*/


/* Internal function prototypes ------------------------------------------------------------------*/


/* Internal functions ----------------------------------------------------------------------------*/

static bool enqueueEvent(MainAppEvent_t event, MainAppMsgContent_t * content) {
    if (!content) {
        return false;
    }
    MainAppMsg_t message = {
        .event = event,
        .content = *content,
    };
    osStatus_t status = osMessageQueuePut(MainAppQHandle, &message, 0u, 0u);
    return (status == osOK);
}


/* External functions ----------------------------------------------------------------------------*/

void MainAppQ_Sleep_ms(uint32_t duration_ms) {
    MainAppMsgContent_t content = {
        .sleepTime_ms = duration_ms,
    };
    enqueueEvent(MainAppEvent_Sleep, &content);
}


void MainAppQ_UserButtonPressed(void) {
    MainAppMsgContent_t content = {
        .buttonTransition = ButtonTransition_Pressed,
    };
    enqueueEvent(MainAppEvent_UserButtonChange, &content);
}


void MainAppQ_UserButtonReleased(void) {
    MainAppMsgContent_t content = {
        .buttonTransition = ButtonTransition_Released,
    };
    enqueueEvent(MainAppEvent_UserButtonChange, &content);
}

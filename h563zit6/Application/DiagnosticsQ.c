/* Includes --------------------------------------------------------------------------------------*/

#include "DiagnosticsQ.h"

#include <stdarg.h>
#include <stdio.h>

#include "app_freertos.h"
#include "RTOS.h"


/* Internal typedef ------------------------------------------------------------------------------*/


/* Internal define -------------------------------------------------------------------------------*/


/* Internal macro --------------------------------------------------------------------------------*/


/* Internal variables ----------------------------------------------------------------------------*/


/* Internal function prototypes ------------------------------------------------------------------*/


/* Internal functions ----------------------------------------------------------------------------*/

static size_t format_message(char* msg, size_t len, const char *fmt, va_list args) {
    int n = vsnprintf(msg, len, fmt, args);
    size_t msgLen = (n < 0) ? (0) : ((size_t)((n >= len) ? (len - 1) : (n)));
    return msgLen;
}

/* External functions ----------------------------------------------------------------------------*/

char const* DiagQ_GetSourceStr(DiagSource_t source) {
    switch (source)
    {
    case DiagSource_MainApp:
        return "MainApp";
    case DiagSource_ISR:
        return "ISR";
    case DiagSource_printf:
        return "printf";
    case DiagSource_Other:
        return "OTHER";
    default:
        return "UNKNOWN";
    }
}

void DiagQ_printf(const char* fmt, ...) {
    DiagMsg_t message = {
        .source = DiagSource_printf,
        .timestamp_ticks = osKernelGetTickCount(),
    };

    va_list args;
    va_start(args, fmt);
    message.len = format_message(message.text, sizeof(message.text), fmt, args);
    va_end(args);
    osMessageQueuePut(DiagnosticsQHandle, &message, 0u, 0u);
}

void DiagQ_Log(DiagSource_t source, const char* fmt, ...) {
    DiagMsg_t message = {
        .source = source,
        .timestamp_ticks = osKernelGetTickCount(),
    };

    va_list args;
    va_start(args, fmt);
    message.len = format_message(message.text, sizeof(message.text), fmt, args);
    va_end(args);
    osMessageQueuePut(DiagnosticsQHandle, &message, 0u, 100u);
}

void DiagQ_LogFromISR(DiagSource_t source, const char* fmt, ...) {
    DiagMsg_t message = {
        .source = source,
        .timestamp_ticks = osKernelGetTickCount(),
    };

    va_list args;
    va_start(args, fmt);
    message.len = format_message(message.text, sizeof(message.text), fmt, args);
    va_end(args);
    osMessageQueuePut(DiagnosticsQHandle, &message, 0u, 0u);
}

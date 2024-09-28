/* Includes --------------------------------------------------------------------------------------*/

#include "app_freertos.h"
#include "RTOS.h"
#include "sys_command_line.h"
#include "usart.h"
#include "usbd_cdc_if.h"


/* Internal typedef ------------------------------------------------------------------------------*/


/* Internal define -------------------------------------------------------------------------------*/


/* Internal macro --------------------------------------------------------------------------------*/


/* Internal variables ----------------------------------------------------------------------------*/


/* Internal function prototypes ------------------------------------------------------------------*/


/* Internal functions ----------------------------------------------------------------------------*/

/**
 *  @brief Process USB VCP (virtual COM port).
 */
static void processUSB(void) {
    static uint16_t count = 0;
    ++count;
    if (count < 1000u) {
        return;
    }
    static uint8_t txMessageBuffer[] = "USB is working!\n";
    USB_CDC_Transmit(txMessageBuffer, sizeof(txMessageBuffer));
    count = 0;
}


static uint16_t usbReceiveCallback(uint8_t *const buffer, uint16_t length) {
    if (buffer == NULL) {
        return 0u;
    }
    if (length == 0u) {
        return 0u;
    }
    printf("%s", (char*)buffer);
    return length;
}


/* External functions ----------------------------------------------------------------------------*/

/**
 *  @brief The main entry point for the Diagnostics task.
 *  The main entry point for the Diagnostics task. At the moment, the Diagnostics task only
 *  processes command line arguments.
 *  @param[in]  argument    TODO
 */
void DiagnosticsTask_Start(void *argument) {
    for (;;) {
        osDelay(RTOS_ConvertMSToTicks(1u));
        CLI_RUN();
        processUSB();
    }
    if (DiagnosticsTaskHandle != NULL) {
        osThreadTerminate(DiagnosticsTaskHandle);
    }
}


/**
 *  @brief Initialization for the Diagnostics task.
 */
void DiagnosticsTask_init(void) {
    CLI_INIT(&huart3, USART3_IRQn);
    USBD_StatusTypeDef status = USB_CDC_Init();
    printf("USB_CDC_Init()=%u\n", status);
    status = USB_CDC_RegisterReceiveCallback(usbReceiveCallback);
    printf("USB_CDC_RegisterReceiveCallback()=%u\n", status);
}

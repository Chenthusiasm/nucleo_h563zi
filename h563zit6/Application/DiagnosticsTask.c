/* Includes --------------------------------------------------------------------------------------*/

#include "app_freertos.h"
#include "ICACHE.h"
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

extern USBD_HandleTypeDef hUsbDeviceFS;
static void processUSB(void) {
    static uint16_t count = 0u;
    if (count++ > 2000u) {
        static uint8_t const txMessageBuffer[] = "My USB is working!\n";
        USB_CDC_Transmit(txMessageBuffer, sizeof(txMessageBuffer));
        count = 0u;
    }
}


static uint16_t usbReceiveCallback(uint8_t* const Buf, uint16_t Len) {
    printf("%s", Buf);
    return Len;
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
        CLI_RUN();
        processUSB();
        osDelay(RTOS_ConvertMSToTicks(1u));
    }
}


/**
 *  @brief  Initialization for the Diagnostics task.
 */
void DiagnosticsTask_Init(void) {
    CLI_INIT(&huart3, USART3_IRQn);
    ICACHE_Init();
    USBD_StatusTypeDef status = USB_CDC_Init();
    USB_CDC_RegisterReceiveCallback(usbReceiveCallback);
}

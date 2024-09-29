/* Includes --------------------------------------------------------------------------------------*/

#include "app_freertos.h"
#include "icache.h"
#include "RTOS.h"
#include "sys_command_line.h"
#include "usart.h"
#include "usb.h"
#include "usbd_cdc_if.h"


/* Internal typedef ------------------------------------------------------------------------------*/


/* Internal define -------------------------------------------------------------------------------*/


/* Internal macro --------------------------------------------------------------------------------*/


/* Internal variables ----------------------------------------------------------------------------*/


/* Internal function prototypes ------------------------------------------------------------------*/


/* Internal functions ----------------------------------------------------------------------------*/

extern USBD_HandleTypeDef hUsbDeviceFS;
static void processUSB(void) {
    if (hUsbDeviceFS.pClassData == NULL) {
        return;
    }
    static uint16_t count = 0u;
    if (count++ > 2000u) {
        static uint8_t const txMessageBuffer[] = "My USB is working!\n";
        USBD_CDC_Transmit(txMessageBuffer, sizeof(txMessageBuffer));
        count = 0u;
    }
}


static size_t usbReceiveCallback(uint8_t* const Buf, size_t Len) {
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
    MX_ICACHE_Init();
    MX_USB_PCD_Init();
    USBD_CDC_RegisterReceiveCallback(usbReceiveCallback);
}

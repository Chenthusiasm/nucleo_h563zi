/**
 * @file    usbd_cdc_if.h
 * @brief   Interface file for the USB CDC (communications device class) driver.
 * @note    The driver is reentrant from the defined structures; the driver may not be reentrant
 *          if different instances of the structures are constructed using the same HW peripherals,
 *          specifically the USB peripheral.
 * @note    The driver is implemented as a singleton.
 * @note    In order to use the USB_CDC driver, the classic USB device middleware with CDC support
 *          must be added to the project. Also, disable the STM32CubeMX auto-generation of the USB
 *          code.
 * @note    If the ICACHE is enabled, the ICACHE_Support module must be included to deal with
 *          avoiding hard faults when accessing RO and OTP memory.
 * @note    Originally based on the work from the STM MCD Application Team with modifications based
 *          on the following knowledge base article and the intent to make platform USB CDC drivers.
 *          https://community.st.com/t5/stm32-mcus/how-to-use-stmicroelectronics-classic-usb-device-middleware-with/ta-p/599274
 * @section Legal Disclaimer
 *      ©2024 Whisker, All rights reserved. All contents of this source file and/or any other
 *      related source files are the explicit property of Whisker. Do not distribute. Do not copy.
 */

#ifndef USBD_CDC_IF_H_
#define USBD_CDC_IF_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------------------------- */

#include <stdbool.h>
#include <stdint.h>

#include "usbd_cdc.h"
#include "usbd_def.h"


/* External typedef ------------------------------------------------------------------------------*/

/**
 * @brief   USB CDC receive callback function that is invoked when data is received over USB.
 * @param[in]   buffer  Buffer of received data.
 * @param[in]   length  Number of data bytes received.
 * @return  The number of data bytes received that were processed; ideally should be equal to the
 *          length parameter.
 */
typedef uint16_t (*USB_CDC_ReceiveCallback_t)(uint8_t *const buffer, uint16_t length);

/* Defines ---------------------------------------------------------------------------------------*/


/* External macro --------------------------------------------------------------------------------*/


/* External variables ----------------------------------------------------------------------------*/


/* External function prototypes ------------------------------------------------------------------*/


/* External functions ----------------------------------------------------------------------------*/


/* External functions ----------------------------------------------------------------------------*/

USBD_StatusTypeDef USB_CDC_Init(void);
USBD_StatusTypeDef USB_CDC_Deinit(void);
USBD_StatusTypeDef USB_CDC_Transmit(uint8_t* const buffer, uint16_t length);
USBD_StatusTypeDef USB_CDC_RegisterReceiveCallback(USB_CDC_ReceiveCallback_t const callback);
USBD_StatusTypeDef USB_CDC_DeregisterReceiveCallback(void);


#ifdef __cplusplus
}
#endif

#endif /* USBD_CDC_IF_H_ */

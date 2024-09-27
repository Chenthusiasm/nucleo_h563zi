/**
 * @file    USB_CDC.h
 * @brief   Interface file for the USB CDC (communications device class) driver.
 * @note    The driver is reentrant from the defined structures; the driver may not be reentrant
 *          if different instances of the structures are constructed using the same HW peripherals,
 *          specifically the USB peripheral. To ensure reentrancy, the Mutex module must be
 *          configured to use an RTOS mutex.
 * @section Legal Disclaimer
 *      ©2024 Whisker, All rights reserved. All contents of this source file and/or any other
 *      related source files are the explicit property of Whisker. Do not distribute. Do not copy.
 */
#ifndef USB_CDC_H_
#define USB_CDC_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------------------------- */

#include <stdbool.h>
#include <stdint.h>

#include "stm32h5xx_hal.h"
#include "usbd_def.h"



/* External typedef ------------------------------------------------------------------------------*/

/**
 * @enum    USB_CDC_Err_t
 * @brief   Enumeration of the different USB CDC driver function return error codes.
 */
typedef enum {
    USB_CDC_ERR_NONE = 0u,          /*!< No error */
    USB_CDC_ERR_NULL_PARAM,         /*!< An input parameter is NULL which is invalid for that
                                         parameter; assert checks should catch these (possibly
                                         UNUSED) */
    USB_CDC_ERR_INVALID_PARAM,      /*!< An input parameter had an invalid value */
    USB_CDC_ERR_RESOURCE_BLOCKED,   /*!< The HW resource is currently blocked */
    USB_CDC_ERR_UNINITIALIZED,      /*!< The USB CDC is not initialized */
    USB_CDC_ERR_HAL_BUSY,           /*!< HAL-level busy error */
    USB_CDC_ERR_HAL_MEMORY,         /*!< HAL-level memory access error */
    USB_CDC_ERR_HAL_FAIL,           /*!< HAL-level general failure/error */
    USB_CDC_ERR_MW_BUSY,            /*!< USBD middleware busy error */
    USB_CDC_ERR_MW_MEMORY,          /*!< USBD middleware memory access error */
    USB_CDC_ERR_MW_FAIL,            /*!< USBD middleware general failure/error */
} USB_CDC_Err_t;


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

USB_CDC_Err_t USB_CDC_Init(void);
USB_CDC_Err_t USB_CDC_Deinit(void);
USB_CDC_Err_t USB_CDC_Transmit(uint8_t *const buffer, uint16_t length);


#ifdef __cplusplus
}
#endif

#endif /* USB_CDC_H_ */

/**
 * @file    USBCDC.h
 * @brief   Interface file for the USB CDC (communications device class) driver.
 * @note    The driver is reentrant from the defined structures; the driver may not be reentrant
 *          if different instances of the structures are constructed using the same HW peripherals,
 *          specifically the USB peripheral. To ensure reentrancy, the Mutex module must be
 *          configured to use an RTOS mutex.
 * @section Legal Disclaimer
 *      ©2024 Whisker, All rights reserved. All contents of this source file and/or any other
 *      related source files are the explicit property of Whisker. Do not distribute. Do not copy.
 */
#ifndef USBCDC_H_
#define USBCDC_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------------------------- */

#include <stdbool.h>
#include <stdint.h>

#include "Mutex.h"
#include "stm32h5xx_hal.h"


/* External typedef ------------------------------------------------------------------------------*/

/**
 * 
 */
typedef uint16_t (*USBCDC_ReceiveCallback_t)(uint8_t *const buffer, uint16_t length);


/**
 * @struct  PWM
 * @brief   Type definition of a structure that aggregates key components needed for the PWM signal
 *          generation to a pin.
 * @var USBCDC.usbHandle        Handle of the USB peripheral.
 * @var USBCDC.usbMutexPtr      Pointer to the USB Mutex.
 * @var USBCDC.receiveCallback  Receive callback function.
 */
typedef struct {
    USB_HandleTypeDef *usbHandle;
    Mutex* usbMutexPtr;
    USBCDC_ReceiveCallback_t receiveCallback;
} USBCDC;


/* Defines ---------------------------------------------------------------------------------------*/


/* External macro --------------------------------------------------------------------------------*/


/* External variables ----------------------------------------------------------------------------*/


/* External function prototypes ------------------------------------------------------------------*/


/* External functions ----------------------------------------------------------------------------*/

USBCDC USBCDC_ctor(USB_HandleTypeDef *const usbHandle, Mutex *const usbMutexPtr,
                   USBCDC_ReceiveCallback_t receiveCallback);
bool USBCDC_Init(USBCDC const *const self);
bool USBCDC_Transmit(USBCDC const *const self, uint8_t const *const buffer, uint16_t length);


#ifdef __cplusplus
}
#endif

#endif /* USBCDC */

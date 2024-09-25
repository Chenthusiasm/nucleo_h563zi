/**
 * @file    DIO.h
 * @brief   Interface file for the DIO (digital input/output) driver (AKA GPIO, general purpose
 *          input/output).
 * @section Legal Disclaimer
 *      ©2024 Whisker, All rights reserved. All contents of this source file and/or any other
 *      related source files are the explicit property of Whisker. Do not distribute. Do not copy.
 */
#ifndef DIO_H_
#define DIO_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------------------------- */

#include <stdbool.h>
#include <stdint.h>

#include "stm32h5xx_hal.h"


/* External typedef ------------------------------------------------------------------------------*/

/**
 * @brief   Type definition for the GPIO port pin.
 * @note    0u = pin 0, 1u = pin 1, 2u = pin 2 ... 15u = pin 15.
 */
typedef uint8_t DIO_Pin_t;


/**
 * @brief   Type definition for the GPIO port pin mask used by the HAL layer.
 * @note    See GPIO_PIN_x (where x = 0 - 15).
 */
typedef uint16_t DIO_PinMask_t;


/**
 * @enum    DIO_Err_t
 * @brief   Enumeration of the different DIO driver function return error codes.
 */
typedef enum {
    DIO_ERR_NONE = 0u,          /*!< No error */
    DIO_ERR_NULL_PARAM,         /*!< An input parameter is NULL which is invalid for that
                                         parameter; assert checks should catch these (possibly
                                         UNUSED) */
    DIO_ERR_INVALID_PARAM,      /*!< An input parameter had an invalid value */
    DIO_ERR_RESOURCE_BLOCKED,   /*!< The HW resource is currently blocked */
    DIO_ERR_INVALID_PIN,        /*!< The pin number is invalid */
    DIO_ERR_PIN_CONFIG,         /*!< The pin configuration doesn't support the function/action */
    DIO_ERR_CALLBACK_CONFIG,    /*!< The callback configuration is invalid */
} DIO_Err_t;


/**
 * @brief   Enumeration of the different transitions that can trigger an external interrupt on the
 *          DIO pin.
 */
typedef enum {
    DIO_TRANSITION_FALLING_EDGE = 0,
    DIO_TRANSITION_RISING_EDGE
} DIO_Transition_t;


/**
 * @brief   DIO external interrupt/event callback function.
 * @param[in]   pin         The pin number (not the GPIO pin mask defined by the HAL).
 * @param[in]   transition  The type of edge transition that triggered the external interrupt.
 */
typedef void (*DIO_EXTICallback_t)(DIO_Pin_t pin, DIO_Transition_t transition);


/**
 * @struct  DIO
 * @brief   Type definition of a structure that aggregates key components needed for the DIO to
 *          operate.
 * @var DIO.portHandle      Handle of the MCU GPIO port peripheral.
 * @var DIO.pin             The specific pin on the GPIO port.
 * @var DIO.extiCallback    The external interrupt/event callback function to invoke when the
 *                          configured transition is externally triggered.
 */
typedef struct {
    GPIO_TypeDef *portHandle;
    DIO_Pin_t pin;
    DIO_EXTICallback_t extiCallback;
} DIO;


/* Defines ---------------------------------------------------------------------------------------*/

/* Invalid pin number. */
#define DIO_INVALID_PIN                 (0xff)


/* External macro --------------------------------------------------------------------------------*/


/* External variables ----------------------------------------------------------------------------*/


/* External function prototypes ------------------------------------------------------------------*/


/* External functions ----------------------------------------------------------------------------*/

DIO DIO_ctor(GPIO_TypeDef *const portHandle, DIO_Pin_t pin, DIO_EXTICallback_t extiCallback);
DIO_Err_t DIO_Init(DIO const *const self);
DIO_Err_t DIO_EnableCallback(DIO const *const self, bool enable);
DIO_Err_t DIO_SetHigh(DIO const *const self);
DIO_Err_t DIO_SetLow(DIO const *const self);
DIO_Err_t DIO_Toggle(DIO const *const self);
bool DIO_IsSetHigh(DIO const *const self);
bool DIO_IsSetLow(DIO const *const self);
bool DIO_IsDigitalInput(DIO const *const self);
bool DIO_IsDigitalOutput(DIO const *const self);
DIO_Pin_t DIO_GetPin(DIO_PinMask_t pinMask);


#ifdef __cplusplus
}
#endif

#endif /* DIO_H_ */

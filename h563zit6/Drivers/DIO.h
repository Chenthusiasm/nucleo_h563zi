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
 * @brief Enumeration of the different DIO pins.
 */
typedef enum {
    DIO_PIN_0 = GPIO_PIN_0,
    DIO_PIN_1 = GPIO_PIN_1,
    DIO_PIN_2 = GPIO_PIN_2,
    DIO_PIN_3 = GPIO_PIN_3,
    DIO_PIN_4 = GPIO_PIN_4,
    DIO_PIN_5 = GPIO_PIN_5,
    DIO_PIN_6 = GPIO_PIN_6,
    DIO_PIN_7 = GPIO_PIN_7,
    DIO_PIN_8 = GPIO_PIN_8,
    DIO_PIN_9 = GPIO_PIN_9,
    DIO_PIN_10 = GPIO_PIN_10,
    DIO_PIN_11 = GPIO_PIN_11,
    DIO_PIN_12 = GPIO_PIN_12,
    DIO_PIN_13 = GPIO_PIN_13,
    DIO_PIN_14 = GPIO_PIN_14,
    DIO_PIN_15 = GPIO_PIN_15
} DIO_Pin_t;


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
 * @param[in]   transition  The type of edge transition that triggered the external interrupt.
 */
typedef void (*DIO_EXTICallback_t)(DIO_Transition_t transition);


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


/* External macro --------------------------------------------------------------------------------*/


/* External variables ----------------------------------------------------------------------------*/


/* External function prototypes ------------------------------------------------------------------*/


/* External functions ----------------------------------------------------------------------------*/

DIO DIO_ctor(GPIO_TypeDef *const portHandle, DIO_Pin_t pin, DIO_EXTICallback_t extiCallback);
bool DIO_Init(DIO const *const self);
void DIO_SetHigh(DIO const *const self);
void DIO_SetLow(DIO const *const self);
void DIO_Toggle(DIO const *const self);
bool DIO_IsSetHigh(DIO const *const self);
bool DIO_IsSetLow(DIO const *const self);


#ifdef __cplusplus
}
#endif

#endif /* DIO_H_ */

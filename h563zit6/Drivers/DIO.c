/**
 * @file    DIO.c
 * @brief   Interface file for the DIO (digital input/output) driver (AKA GPIO, general purpose
 *          input/output).
 * @section Legal Disclaimer
 *      ©2024 Whisker, All rights reserved. All contents of this source file and/or any other
 *      related source files are the explicit property of Whisker. Do not distribute. Do not copy.
 */

/* Includes --------------------------------------------------------------------------------------*/

#include <assert.h>

#include "DIO.h"


/* Internal typedef ------------------------------------------------------------------------------*/


/* Internal define -------------------------------------------------------------------------------*/


/* Internal macro --------------------------------------------------------------------------------*/


/* Internal variables ----------------------------------------------------------------------------*/


/* Internal constants ----------------------------------------------------------------------------*/


/* Internal function prototypes ------------------------------------------------------------------*/


/* Internal functions ----------------------------------------------------------------------------*/


/* External function prototypes ------------------------------------------------------------------*/


/* External functions ----------------------------------------------------------------------------*/

/**
 * @brief   Constructor that initializes the parameters of the Timer struct.
 * @param[in]   portHandle      Handle of the MCU GPIO port peripheral.
 * @param[in]   pin             DIO_Pin_t pin.
 * @param[in]   extiCallback    The external interrupt/event callback function to invoke when the
 *                              configured transition is externally triggered.
 * @return  The new DIO struct (to be copied upon assignment).
 */
DIO DIO_ctor(GPIO_TypeDef *const portHandle, DIO_Pin_t pin, DIO_EXTICallback_t extiCallback) {
    assert(portHandle != NULL);

    DIO self = {
        .portHandle = portHandle,
        .pin = pin,
        .extiCallback = extiCallback,
    };
    return self;
}


/**
 * @brief   Initializes the DIO instance.
 * @param[in]   self    Pointer to the DIO struct that represents the digital I/O instance.
 * @return  If the initialization was successful, true; otherwise, false.
 */
bool DIO_Init(DIO const *const self) {
    assert(self != NULL);

    // no init code; using the STM32CubeMX auto-generated initialization code
    return true;
}


/**
 * @brief   Set the DIO pin to logic level high.
 * @param[in]   self    Pointer to the DIO struct that represents the digital I/O instance.
 */
void DIO_SetHigh(DIO const *const self) {
    assert(self != NULL);

    HAL_GPIO_WritePin(self->portHandle, self->pin, GPIO_PIN_SET);
}


/**
 * @brief   Set the DIO pin to logic level low.
 * @param[in]   self    Pointer to the DIO struct that represents the digital I/O instance.
 */
void DIO_SetLow(DIO const *const self) {
    assert(self != NULL);

    HAL_GPIO_WritePin(self->portHandle, self->pin, GPIO_PIN_RESET);
}


/**
 * @brief   Toggle the DIO pin to the opposite logic level from its current level.
 * @param[in]   self    Pointer to the DIO struct that represents the digital I/O instance.
 */
void DIO_Toggle(DIO const *const self) {
    assert(self != NULL);

    HAL_GPIO_TogglePin(self->portHandle, self->pin);
}


/**
 * @brief   Check if the DIO pin is logic level high.
 * @param[in]   self    Pointer to the DIO struct that represents the digital I/O instance.
 * @return  If the DIO pin is logic level high, true; otherwise, false.
 */
bool DIO_IsSetHigh(DIO const *const self) {
    assert(self != NULL);

    return (HAL_GPIO_ReadPin(self->portHandle, self->pin) == GPIO_PIN_SET);
}


/**
 * @brief   Check if the DIO pin is logic level low.
 * @param[in]   self    Pointer to the DIO struct that represents the digital I/O instance.
 * @return  If the DIO pin is logic level low, true; otherwise, false.
 */
bool DIO_IsSetLow(DIO const *const self) {
    assert(self != NULL);

    return (HAL_GPIO_ReadPin(self->portHandle, self->pin) == GPIO_PIN_RESET);
}

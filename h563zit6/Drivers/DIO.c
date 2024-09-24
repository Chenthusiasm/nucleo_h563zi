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
#include "DIO_IRQ.h"


/* Internal typedef ------------------------------------------------------------------------------*/


/* Internal define -------------------------------------------------------------------------------*/

/* The number of pins on a DIO (GPIO) port. */
#define PORT_NUM_PINS                   (16u)


/* Internal macro --------------------------------------------------------------------------------*/


/* Internal variables ----------------------------------------------------------------------------*/


/* Internal constants ----------------------------------------------------------------------------*/

/* Map/lookup table of the pin number to GPIO pin mask used by the HAL. */
static uint16_t const GPIOPinMaskMap[] = {
    [ 0] = GPIO_PIN_0,
    [ 1] = GPIO_PIN_1,
    [ 2] = GPIO_PIN_2,
    [ 3] = GPIO_PIN_3,
    [ 4] = GPIO_PIN_4,
    [ 5] = GPIO_PIN_5,
    [ 6] = GPIO_PIN_6,
    [ 7] = GPIO_PIN_7,
    [ 8] = GPIO_PIN_8,
    [ 9] = GPIO_PIN_9,
    [10] = GPIO_PIN_10,
    [11] = GPIO_PIN_11,
    [12] = GPIO_PIN_12,
    [13] = GPIO_PIN_13,
    [14] = GPIO_PIN_14,
    [15] = GPIO_PIN_15,
};


/* Internal function prototypes ------------------------------------------------------------------*/


/* Internal functions ----------------------------------------------------------------------------*/


/* External function prototypes ------------------------------------------------------------------*/


/* External functions ----------------------------------------------------------------------------*/

/**
 * @brief   Constructor that initializes the parameters of the Timer struct.
 * @param[in]   portHandle      Handle of the MCU GPIO port peripheral.
 * @param[in]   pin             The pin number (not the GPIO pin mask defined by the HAL).
 * @param[in]   extiCallback    The external interrupt/event callback function to invoke when the
 *                              configured transition is externally triggered.
 * @return  The new DIO struct (to be copied upon assignment).
 */
DIO DIO_ctor(GPIO_TypeDef *const portHandle, uint8_t pin, DIO_EXTICallback_t extiCallback) {
    assert(portHandle != NULL);
    assert(pin < PORT_NUM_PINS);

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
 * @return  The specific DIO_Err_t which indicates the specific error code if the function couldn't
 *          be executed successfully. If the function executes successfully, DIO_ERR_NONE.
 */
DIO_Err_t DIO_Init(DIO const *const self) {
    assert(self != NULL);

    // no init code; using the STM32CubeMX auto-generated initialization code
    return DIO_ERR_NONE;
}


/**
 * @brief   Set the DIO pin to logic level high.
 * @param[in]   self    Pointer to the DIO struct that represents the digital I/O instance.
 * @param[in]   enable  Flag; true to enable, false to disable.
 * @return  The specific DIO_Err_t which indicates the specific error code if the function couldn't
 *          be executed successfully. If the function executes successfully, DIO_ERR_NONE.
 */
DIO_Err_t DIO_EnableCallback(DIO const *const self, bool enable) {
    assert(self != NULL);

    DIO_IRQ_Err_t err = DIO_IRQ_Enable(self->pin, enable);
    if (err == DIO_IRQ_ERR_INVALID_PARAM) {
        return DIO_ERR_INVALID_PIN;
    }
    if (err == DIO_IRQ_ERR_RESOURCE_BLOCKED) {
        return DIO_ERR_RESOURCE_BLOCKED;
    }
    if (err == DIO_IRQ_ERR_UNREGISTERED) {
        return DIO_ERR_CALLBACK_CONFIG;
    }
    return DIO_ERR_NONE;
}


/**
 * @brief   Set the DIO pin to logic level high.
 * @param[in]   self    Pointer to the DIO struct that represents the digital I/O instance.
 * @return  The specific DIO_Err_t which indicates the specific error code if the function couldn't
 *          be executed successfully. If the function executes successfully, DIO_ERR_NONE.
 */
DIO_Err_t DIO_SetHigh(DIO const *const self) {
    assert(self != NULL);

    if (self->pin >= PORT_NUM_PINS) {
        return DIO_ERR_INVALID_PIN;
    }
    HAL_GPIO_WritePin(self->portHandle, GPIOPinMaskMap[self->pin], GPIO_PIN_SET);
    return DIO_ERR_NONE;
}


/**
 * @brief   Set the DIO pin to logic level low.
 * @param[in]   self    Pointer to the DIO struct that represents the digital I/O instance.
 */
DIO_Err_t DIO_SetLow(DIO const *const self) {
    assert(self != NULL);

    if (self->pin >= PORT_NUM_PINS) {
        return DIO_ERR_INVALID_PIN;
    }
    HAL_GPIO_WritePin(self->portHandle, GPIOPinMaskMap[self->pin], GPIO_PIN_RESET);
    return DIO_ERR_NONE;
}


/**
 * @brief   Toggle the DIO pin to the opposite logic level from its current level.
 * @param[in]   self    Pointer to the DIO struct that represents the digital I/O instance.
 */
DIO_Err_t DIO_Toggle(DIO const *const self) {
    assert(self != NULL);

    if (self->pin >= PORT_NUM_PINS) {
        return DIO_ERR_INVALID_PIN;
    }
    HAL_GPIO_TogglePin(self->portHandle, GPIOPinMaskMap[self->pin]);
    return DIO_ERR_NONE;
}


/**
 * @brief   Check if the DIO pin is logic level high.
 * @param[in]   self    Pointer to the DIO struct that represents the digital I/O instance.
 * @return  If the DIO pin is logic level high, true; otherwise, false.
 */
bool DIO_IsSetHigh(DIO const *const self) {
    assert(self != NULL);

    if (self->pin >= PORT_NUM_PINS) {
        return false;
    }
    return (HAL_GPIO_ReadPin(self->portHandle, GPIOPinMaskMap[self->pin]) == GPIO_PIN_SET);
}


/**
 * @brief   Check if the DIO pin is logic level low.
 * @param[in]   self    Pointer to the DIO struct that represents the digital I/O instance.
 * @return  If the DIO pin is logic level low, true; otherwise, false.
 */
bool DIO_IsSetLow(DIO const *const self) {
    assert(self != NULL);

    if (self->pin >= PORT_NUM_PINS) {
        return false;
    }
    return (HAL_GPIO_ReadPin(self->portHandle, GPIOPinMaskMap[self->pin]) == GPIO_PIN_RESET);
}

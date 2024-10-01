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
#include "Helper.h"
#include "stm32h5xx_ll_gpio.h"


/* Internal typedef ------------------------------------------------------------------------------*/


/* Internal define -------------------------------------------------------------------------------*/

/* The number of pins on a DIO (GPIO) port. */
#define PORT_NUM_PINS                   (16u)


/* Internal macro --------------------------------------------------------------------------------*/


/* Internal variables ----------------------------------------------------------------------------*/


/* Internal constants ----------------------------------------------------------------------------*/

/* Map/lookup table of the pin number to GPIO pin mask used by the HAL. */
static DIO_PinMask_t const GPIOPinMaskMap[] = {
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


/* Lookup table of the GPIO port handles. */
static GPIO_TypeDef const *const GPIOPortHandleTable[] = {
    [0] = GPIOA,
    [1] = GPIOB,
    [2] = GPIOC,
    [3] = GPIOD,
    [4] = GPIOE,
    [5] = GPIOF,
    [6] = GPIOG,
    [7] = GPIOH,
    [8] = GPIOI,
};


/* Internal function prototypes ------------------------------------------------------------------*/


/* Internal functions ----------------------------------------------------------------------------*/

/**
 * @brief   Checks if EXTI is enabled for the given port and pin mask.
 * @param[in]   portHandle  Handle of the MCU GPIO port peripheral.
 * @param[in]   pinMask     The HAL pin mask.
 * @note    Refer to the TRM: EXTI_EXTICR1, EXTI_EXTICR2, EXTI_EXTICR3, and EXTI_EXTICR4.
 * @return  If the GPIO port and pin mask is configured for EXTI, true; otherwise, false.
 */
static bool isEXTIEnabled(GPIO_TypeDef const *const portHandle, DIO_PinMask_t pinMask) {
    if (IsPowerOfTwo(pinMask) == false) {
        return false;
    }
    // index: the specific EXTICRn register, where index=0 corresponds to EXTICR1
    // mask: the isolated nibble in pinMask where the bit is set (i.e pinMask=0x0400 mask=0x4)
    uint8_t index = 0u;
    uint16_t mask = pinMask;
    if (pinMask >= GPIO_PIN_4) {
        index += 1u;
        mask >>= 4u;
        if (pinMask >= GPIO_PIN_8) {
            index += 1u;
            mask >>= 4u;
            if (pinMask >= GPIO_PIN_12) {
                index += 1u;
                mask >>= 4u;
            }
        }
    }
    // convert the mask into a right shift value since the EXTICR registers are divided into 8-bit
    // values representing the specific GPIO port with the EXTI enabled
    uint8_t rightShift = 0u;
    if (mask == 0x0002) {
        rightShift = 8u;
    } else if (mask == 0x0004) {
        rightShift = 16u;
    } else if (mask == 0x0008) {
        rightShift = 24u;
    }
    // use the right shift to generate the value that corresponds to the specific GPIO port; see
    // GPIOPortHandleTable for the conversion
    uint32_t value = (EXTI->EXTICR[index] >> rightShift) & 0x000000ff;
    if (value >= (sizeof(GPIOPortHandleTable) / sizeof(GPIOPortHandleTable[0]))) {
        return false;
    }
    return (portHandle == GPIOPortHandleTable[value]);
}


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
DIO DIO_ctor(GPIO_TypeDef *const portHandle, DIO_Pin_t pin, DIO_EXTICallback_t extiCallback) {
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
 * @brief   Initializes the DIO instance and registers the external interrupt.
 * @param[in]   self    Pointer to the DIO struct that represents the digital I/O instance.
 * @return  The specific DIO_Err_t which indicates the specific error code if the function couldn't
 *          be executed successfully. If the function executes successfully, DIO_ERR_NONE.
 */
DIO_Err_t DIO_Init(DIO const *const self) {
    assert(self != NULL);

    if (self->extiCallback == NULL) {
        return DIO_ERR_NONE;
    }
    if (DIO_IsDigitalInput(self) != true) {
        return DIO_ERR_PIN_CONFIG;
    }
    DIO_IRQ_Err_t err = DIO_IRQ_Register(self->pin, self->extiCallback);
    switch (err) {
    case DIO_IRQ_ERR_UNINITIALIZED:
    case DIO_IRQ_ERR_INVALID_PARAM:
    case DIO_IRQ_ERR_REGISTERED:
        return DIO_ERR_CALLBACK_CONFIG;
    case DIO_IRQ_ERR_RESOURCE_BLOCKED:
        return DIO_ERR_RESOURCE_BLOCKED;
    default:
        return DIO_ERR_NONE;
    }
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

    if (DIO_IsDigitalInput(self) != true) {
        return DIO_ERR_PIN_CONFIG;
    }
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
    if (DIO_IsDigitalOutput(self) != true) {
        return DIO_ERR_PIN_CONFIG;
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
    if (DIO_IsDigitalOutput(self) != true) {
        return DIO_ERR_PIN_CONFIG;
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
    if (DIO_IsDigitalOutput(self) != true) {
        return DIO_ERR_PIN_CONFIG;
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


/**
 * @brief   Check if the DIO pin is configured for digital input.
 * @param[in]   self    Pointer to the DIO struct that represents the digital I/O instance.
 * @return  If the DIO pin is configured for digital input, true; otherwise, false.
 */
bool DIO_IsDigitalInput(DIO const *const self) {
    assert(self != NULL);

    return DIO_IsPortPinDigitalInput(self->portHandle, GPIOPinMaskMap[self->pin]);
}


/**
 * @brief   Check if the DIO pin is configured for digital output.
 * @param[in]   self    Pointer to the DIO struct that represents the digital I/O instance.
 * @return  If the DIO pin is configured for digital output, true; otherwise, false.
 */
bool DIO_IsDigitalOutput(DIO const *const self) {
    assert(self != NULL);

    return DIO_IsPortPinDigitalOutput(self->portHandle, GPIOPinMaskMap[self->pin]);
}


/**
 * @brief   Check if the DIO pin is configured for EXTI (external interrupt).
 * @param[in]   self    Pointer to the DIO struct that represents the digital I/O instance.
 * @return  If the DIO pin is configured for EXTI, true; otherwise, false.
 */
bool DIO_IsEXTI(DIO const *const self) {
    assert(self != NULL);

    return DIO_IsPortPinEXTI(self->portHandle, GPIOPinMaskMap[self->pin]);
}


/**
 * @brief   Converts the HAL pin mask to the pin number.
 * @param[in]   pinMask The HAL pin mask.
 * @note    A DIO instance is not needed; this is like a C++ static method.
 * @return  The HAL pin mask converted into the pin number; if the pin mask is invalid,
 *          return DIO_INVALID_PIN.
 */
DIO_Pin_t DIO_GetPin(DIO_PinMask_t pinMask) {
    DIO_Pin_t pin = DIO_INVALID_PIN;
    switch (pinMask) {
    case GPIO_PIN_0 : pin =  0u; break;
    case GPIO_PIN_1 : pin =  1u; break;
    case GPIO_PIN_2 : pin =  2u; break;
    case GPIO_PIN_3 : pin =  3u; break;
    case GPIO_PIN_4 : pin =  4u; break;
    case GPIO_PIN_5 : pin =  5u; break;
    case GPIO_PIN_6 : pin =  6u; break;
    case GPIO_PIN_7 : pin =  7u; break;
    case GPIO_PIN_8 : pin =  8u; break;
    case GPIO_PIN_9 : pin =  9u; break;
    case GPIO_PIN_10: pin = 10u; break;
    case GPIO_PIN_11: pin = 11u; break;
    case GPIO_PIN_12: pin = 12u; break;
    case GPIO_PIN_13: pin = 13u; break;
    case GPIO_PIN_14: pin = 14u; break;
    case GPIO_PIN_15: pin = 15u; break;
    default:
        // do nothing
        ;
    }
    return pin;
}


/**
 * @brief   Check if the GPIO port and pin mask is configured for digital input.
 * @param[in]   portHandle  Handle of the MCU GPIO port peripheral.
 * @param[in]   pinMask     The HAL pin mask.
 * @note    A DIO instance is not needed; this is like a C++ static method.
 * @return  If the GPIO port and pin mask is configured for digital input, true; otherwise, false.
 */
bool DIO_IsPortPinDigitalInput(GPIO_TypeDef const *const portHandle, DIO_PinMask_t pinMask) {
    if (portHandle == NULL) {
        return false;
    }
    return (LL_GPIO_GetPinMode(portHandle, pinMask) == LL_GPIO_MODE_INPUT);
}


/**
 * @brief   Check if the GPIO port and pin mask is configured for digital output.
 * @param[in]   portHandle  Handle of the MCU GPIO port peripheral.
 * @param[in]   pinMask     The HAL pin mask.
 * @note    A DIO instance is not needed; this is like a C++ static method.
 * @return  If the GPIO port and pin mask is configured for EXTI, true; otherwise, false.
 */
bool DIO_IsPortPinDigitalOutput(GPIO_TypeDef const *const portHandle, DIO_PinMask_t pinMask) {
    if (portHandle == NULL) {
        return false;
    }
    return (LL_GPIO_GetPinMode(portHandle, pinMask) == LL_GPIO_MODE_OUTPUT);

}


/**
 * @brief   Check if the GPIO port and pin mask is configured for EXTI (external interrupt).
 * @param[in]   portHandle  Handle of the MCU GPIO port peripheral.
 * @param[in]   pinMask     The HAL pin mask.
 * @note    A DIO instance is not needed; this is like a C++ static method.
 * @return  If the GPIO port and pin mask is configured for EXTI, true; otherwise, false.
 */
bool DIO_IsPortPinEXTI(GPIO_TypeDef const *const portHandle, DIO_PinMask_t pinMask) {
    if (portHandle == NULL) {
        return false;
    }
    return ((LL_GPIO_GetPinMode(portHandle, pinMask) == LL_GPIO_MODE_INPUT) &&
            isEXTIEnabled(portHandle, pinMask));
}

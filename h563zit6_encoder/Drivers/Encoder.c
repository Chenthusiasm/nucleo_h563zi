/**
 * @file    Encoder.c
 * @brief   Implementation of the quadrature encoder driver.
 * @note    The driver is reentrant from the defined structures; the driver may not be reentrant
 *          if different instances of the structures are constructed using the same HW peripherals.
 * @section Legal Disclaimer
 *      ©2024 Whisker, All rights reserved. All contents of this source file and/or any other
 *      related source files are the explicit property of Whisker. Do not distribute. Do not copy.
 */

/* Includes --------------------------------------------------------------------------------------*/

#include <assert.h>

#include "Encoder.h"
#include "stm32h5xx_hal_def.h"


/* Internal typedef ------------------------------------------------------------------------------*/


/* Internal define -------------------------------------------------------------------------------*/

/* Maximum timer filter value for the encoder. */
#define MAX_FILTER_VALUE                (15u)


/* Internal macro --------------------------------------------------------------------------------*/


/* Internal variables ----------------------------------------------------------------------------*/


/* Internal constants ----------------------------------------------------------------------------*/


/* Internal function prototypes ------------------------------------------------------------------*/
#define USE_CUSTOM_HAL_INIT (true)
#if defined(USE_CUSTOM_HAL_INIT)
/**
 * @brief   Configure the specific GPIO pins used for the encoder function as TIM alternate
 *          function.
 * @param[in]   gpioHandle              Handle to the GPIO peripheral to configure.
 * @param[in]   gpioPinMask             Mask of the pins on the GPIO port to configure.
 * @param[in]   altFunctionSelection    GPIO alternate function selection for the specific GPIO
 *                                      pins. This will be to select the TIM alternate function.
 * @return  If the GPIO configuration is successful, true; otherwise, false.
 */
static bool configGPIO(GPIO_TypeDef *const gpioHandle, uint16_t gpioPinMask, uint8_t altFunctionSelection) {
    if (gpioHandle == GPIOA) {
        __HAL_RCC_GPIOA_CLK_ENABLE();
    } else if (gpioHandle == GPIOB) {
        __HAL_RCC_GPIOB_CLK_ENABLE();
    } else if (gpioHandle == GPIOC) {
        __HAL_RCC_GPIOC_CLK_ENABLE();
    } else if (gpioHandle == GPIOD) {
        __HAL_RCC_GPIOD_CLK_ENABLE();
    } else if (gpioHandle == GPIOE) {
        __HAL_RCC_GPIOE_CLK_ENABLE();
    } else {
        return false;
    }

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = gpioPinMask;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = altFunctionSelection;
    HAL_GPIO_Init(gpioHandle, &GPIO_InitStruct);

    return true;
}


/**
 * @brief   Initialize the MSP (MCU support package) for the encoder functionality; specifically
 *          enabling appropriate clocks and configuring the GPIO.
 * @param[in]   self    Pointer to the Encoder struct that represents the encoder instance.
 * @return  If the MSP configuration is successful, true; otherwise, false.
 */
static bool mspInit(Encoder const *const self) {
    assert(self != NULL);

    uint8_t altFunctionSelection = GPIO_AF1_TIM1;
    if (self->timHandle->Instance == TIM1) {
        __HAL_RCC_TIM1_CLK_ENABLE();
    } else if (self->timHandle->Instance == TIM2) {
        __HAL_RCC_TIM2_CLK_ENABLE();
        altFunctionSelection = GPIO_AF1_TIM2;
    } else if (self->timHandle->Instance == TIM3) {
        __HAL_RCC_TIM3_CLK_ENABLE();
        altFunctionSelection = GPIO_AF2_TIM3;
    } else if (self->timHandle->Instance == TIM4) {
        __HAL_RCC_TIM4_CLK_ENABLE();
        altFunctionSelection = GPIO_AF2_TIM4;
    } else if (self->timHandle->Instance == TIM5) {
        __HAL_RCC_TIM5_CLK_ENABLE();
        altFunctionSelection = GPIO_AF2_TIM5;
    } else if (self->timHandle->Instance == TIM8) {
        __HAL_RCC_TIM8_CLK_ENABLE();
        altFunctionSelection = GPIO_AF3_TIM8;
    } else {
        return false;
    }

    if (self->gpio[0].portHandle == self->gpio[1].portHandle) {
        if (configGPIO(self->gpio[0].portHandle,
                       self->gpio[0].pin | self->gpio[1].pin,
                       altFunctionSelection) == false) {
            return false;
        }
    } else {
        if (configGPIO(self->gpio[0].portHandle, self->gpio[0].pin, altFunctionSelection) == false) {
            return false;
        }
        if (configGPIO(self->gpio[0].portHandle, self->gpio[1].pin, altFunctionSelection) == false) {
            return false;
        }
    }
    return true;
}
#endif /* defined(USE_CUSTOM_HAL_INIT) */


/* Internal functions ----------------------------------------------------------------------------*/


/* External function prototypes ------------------------------------------------------------------*/


/* External functions ----------------------------------------------------------------------------*/

/**
 * @brief   Constructor that initializes the parameters of the Encoder struct.
 * @param[in]   timHandle           Handle to the TIM handle used to control the encoder function.
 * @param[in]   timPtr              Pointer to the specific HW TIM registers.
 * @param[in]   gpioPortHandleCh1   Handle to the GPIO port for encoder channel 1.
 * @param[in]   gpioPinCh1          Pin number for encoder channel 1.
 * @param[in]   gpioPortHandleCh2   Handle to the GPIO port for encoder channel 2.
 * @param[in]   gpioPinCh2          Pin number for encoder channel 2.
 * @return  The new Encoder struct (to be copied upon assignment).
 */
Encoder Encoder_ctor(TIM_HandleTypeDef *const timHandle, TIM_TypeDef *const timPtr,
                     GPIO_TypeDef *const gpioPortHandleCh1, uint16_t gpioPinCh1,
                     GPIO_TypeDef *const gpioPortHandleCh2, uint16_t gpioPinCh2) {
    assert(timHandle != NULL);
    assert(timPtr != NULL);
    assert(gpioPortHandleCh1 != NULL);
    assert(gpioPortHandleCh2 != NULL);

    Encoder self = {
        .timHandle = timHandle,
        .gpio = {
            [0] = {
                .portHandle = gpioPortHandleCh1,
                .pin = gpioPinCh1
            },
            [1] = {
                .portHandle = gpioPortHandleCh2,
                .pin = gpioPinCh2
            }
        }
    };
    self.timHandle->Instance = timPtr;

    return self;
}


/**
 * @brief   Initializes the encoder instance.
 * @param[in]   self        Pointer to the Encoder struct that represents the encoder instance.
 * @param[in]   maxCount    Maximum count value the TIM counts up to (1 - 65535; 0 is invalid).
 * @param[in]   filter      The encoder filter value (0 - 15).
 * @return  If the encoder is successfully initialized, true; otherwise, false.
 */
bool Encoder_Init(Encoder const *const self, uint16_t maxCount, uint8_t filter) {
#if defined(USE_CUSTOM_HAL_INIT)
    assert(self != NULL);
    assert(maxCount != 0u);

    self->timHandle->Init.Prescaler = 0u;
    self->timHandle->Init.CounterMode = TIM_COUNTERMODE_UP;
    self->timHandle->Init.Period = maxCount;
    self->timHandle->Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    self->timHandle->Init.RepetitionCounter = 0u;
    self->timHandle->Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    if (filter > MAX_FILTER_VALUE) {
        filter = MAX_FILTER_VALUE;
    }
    TIM_Encoder_InitTypeDef sConfig = {
        .EncoderMode = TIM_ENCODERMODE_TI12,
        .IC1Polarity = TIM_ICPOLARITY_RISING,
        .IC1Selection = TIM_ICSELECTION_DIRECTTI,
        .IC1Prescaler = TIM_ICPSC_DIV1,
        .IC1Filter = filter,
        .IC2Polarity = TIM_ICPOLARITY_RISING,
        .IC2Selection = TIM_ICSELECTION_DIRECTTI,
        .IC2Prescaler = TIM_ICPSC_DIV1,
        .IC2Filter = filter,
    };
    if (mspInit(self) == false) {
        return false;
    }
    if (HAL_TIM_Encoder_Init(self->timHandle, &sConfig) != HAL_OK) {
        return false;
    }

    TIM_MasterConfigTypeDef sMasterConfig = {
        .MasterOutputTrigger = TIM_TRGO_RESET,
        .MasterOutputTrigger2 = TIM_TRGO2_RESET,
        .MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE
    };
    if (HAL_TIMEx_MasterConfigSynchronization(self->timHandle, &sMasterConfig) != HAL_OK) {
        return false;
    }
#else
    UNUSED(self);
    UNUSED(maxCount);
    UNUSED(filter);
#endif /* defined(USE_CUSTOM_HAL_INIT) */
    return true;
}


/**
 * @brief   Starts the encoder counting.
 * @param[in]   self    Pointer to the Encoder struct that represents the encoder instance.
 */
void Encoder_Start(Encoder const *const self) {
    assert(self != NULL);

    HAL_TIM_Encoder_Start(self->timHandle, TIM_CHANNEL_ALL);
}


/**
 * @brief   Stops the encoder counting.
 * @param[in]   self    Pointer to the Encoder struct that represents the encoder instance.
 */
void Encoder_Stop(Encoder const *const self) {
    assert(self != NULL);

    HAL_TIM_Encoder_Stop(self->timHandle, TIM_CHANNEL_ALL);
}


/**
 * @brief   Accessor to get the encoder TIM's maximum count.
 * @param[in]   self    Pointer to the Encoder struct that represents the encoder instance.
 * @return  The encoder's maximum count.
 */
uint16_t Encoder_GetMaxCount(Encoder const *const self) {
    assert(self != NULL);

    return self->timHandle->Init.Period;
}


/**
 * @brief   Accessor to get the encoder's current counter value.
 * @note    The current counter value is a signed 16-bit integer; this allows counter-clockwise
 *          rotation to decrement the counter into negative values until underflow in which the
 *          counter value will roll under to the maximum of an int16_t, 32767. By the same token, if
 *          clockwise rotation increments the counter and the value overflows, the counter value
 *          will roll over to the minimum of an int16_t, -32768.
 * @param[in]   self    Pointer to the Encoder struct that represents the encoder instance.
 * @return  The encoder's current counter value.
 */
int16_t Encoder_GetCounter(Encoder const *const self) {
    assert(self != NULL);

    return __HAL_TIM_GET_COUNTER(self->timHandle);
}


/**
 * @brief   Accessor to set the encoder's current counter value.
 * @note    The current counter value is a signed 16-bit integer; this allows counter-clockwise
 *          rotation to decrement the counter into negative values until underflow in which the
 *          counter value will roll under to the maximum of an int16_t, 32767. By the same token, if
 *          clockwise rotation increments the counter and the value overflows, the counter value
 *          will roll over to the minimum of an int16_t, -32768.
 * @param[in]   self    Pointer to the Encoder struct that represents the encoder instance.
 */
void Encoder_SetCounter(Encoder const *const self, int16_t count) {
    assert(self != NULL);

    uint16_t unsignedCount = (uint16_t)count;
    uint16_t maxCount = Encoder_GetMaxCount(self);
    if (unsignedCount > maxCount) {
        unsignedCount = unsignedCount % maxCount;
    }
    __HAL_TIM_SET_COUNTER(self->timHandle, unsignedCount);
}


/**
 * @brief   Accessor to set the encoder's current counter value to the reset value of 0.
 * @note    The current counter value is a signed 16-bit integer; this allows counter-clockwise
 *          rotation to decrement the counter into negative values until underflow in which the
 *          counter value will roll under to the maximum of an int16_t, 32767. By the same token, if
 *          clockwise rotation increments the counter and the value overflows, the counter value
 *          will roll over to the minimum of an int16_t, -32768.
 * @param[in]   self    Pointer to the Encoder struct that represents the encoder instance.
 */
void Encoder_ResetCounter(Encoder const *const self) {
    assert(self != NULL);

    Encoder_SetCounter(self, 0);
}

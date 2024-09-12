/**
 * @file    Encoder.c
 *
 *  @section    Legal Disclaimer
 *      ©2024 Whisker, All rights reserved. All contents of this source file and/or any other
 *      related source files are the explicit property of Whisker. Do not distribute. Do not copy.
 */

/* Includes --------------------------------------------------------------------------------------*/

#include <assert.h>

#include "Encoder.h"
#include "stm32h5xx_hal_def.h"


/* Internal typedef ------------------------------------------------------------------------------*/


/* Internal define -------------------------------------------------------------------------------*/


/* Internal macro --------------------------------------------------------------------------------*/


/* Internal variables ----------------------------------------------------------------------------*/


/* Internal constants ----------------------------------------------------------------------------*/


/* Internal function prototypes ------------------------------------------------------------------*/

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


static bool mspInit(Encoder const *const encoder) {
    assert(encoder != NULL);

    uint8_t altFunctionSelection = GPIO_AF1_TIM1;
    if (encoder->timHandle->Instance == TIM1) {
        __HAL_RCC_TIM1_CLK_ENABLE();
    } else if (encoder->timHandle->Instance == TIM2) {
        __HAL_RCC_TIM2_CLK_ENABLE();
        altFunctionSelection = GPIO_AF1_TIM2;
    } else if (encoder->timHandle->Instance == TIM3) {
        __HAL_RCC_TIM3_CLK_ENABLE();
        altFunctionSelection = GPIO_AF2_TIM3;
    } else if (encoder->timHandle->Instance == TIM4) {
        __HAL_RCC_TIM4_CLK_ENABLE();
        altFunctionSelection = GPIO_AF2_TIM4;
    } else if (encoder->timHandle->Instance == TIM5) {
        __HAL_RCC_TIM5_CLK_ENABLE();
        altFunctionSelection = GPIO_AF2_TIM5;
    } else if (encoder->timHandle->Instance == TIM8) {
        __HAL_RCC_TIM8_CLK_ENABLE();
        altFunctionSelection = GPIO_AF3_TIM8;
    } else {
        return false;
    }

    if (encoder->gpio[0].portHandle == encoder->gpio[1].portHandle) {
        if (configGPIO(encoder->gpio[0].portHandle,
                       encoder->gpio[0].pin | encoder->gpio[1].pin,
                       altFunctionSelection) == false) {
            return false;
        }
    } else {
        if (configGPIO(encoder->gpio[0].portHandle, encoder->gpio[0].pin, altFunctionSelection) == false) {
            return false;
        }
        if (configGPIO(encoder->gpio[0].portHandle, encoder->gpio[1].pin, altFunctionSelection) == false) {
            return false;
        }
    }
    return true;
}


/* Internal functions ----------------------------------------------------------------------------*/


/* External function prototypes ------------------------------------------------------------------*/


/* External functions ----------------------------------------------------------------------------*/

Encoder Encoder_ctor(TIM_HandleTypeDef *const timHandle, TIM_TypeDef *const timPtr,
                     GPIO_TypeDef *const gpioPortHandleCh1, uint16_t gpioPinCh1,
                     GPIO_TypeDef *const gpioPortHandleCh2, uint16_t gpioPinCh2) {
    assert(timHandle != NULL);
    assert(timPtr != NULL);
    assert(gpioPortHandleCh1 != NULL);
    assert(gpioPortHandleCh2 != NULL);

    Encoder encoder = {
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
    encoder.timHandle->Instance = timPtr;

    return encoder;
}


bool Encoder_Init(Encoder const *const encoder, uint16_t maxCount) {
    assert(encoder != NULL);
    assert(maxCount != 0u);

    encoder->timHandle->Init.Prescaler = 0u;
    encoder->timHandle->Init.CounterMode = TIM_COUNTERMODE_UP;
    encoder->timHandle->Init.Period = maxCount;
    encoder->timHandle->Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    encoder->timHandle->Init.RepetitionCounter = 0u;
    encoder->timHandle->Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    TIM_Encoder_InitTypeDef sConfig = {
        .EncoderMode = TIM_ENCODERMODE_TI12,
        .IC1Polarity = TIM_ICPOLARITY_RISING,
        .IC1Selection = TIM_ICSELECTION_DIRECTTI,
        .IC1Prescaler = TIM_ICPSC_DIV1,
        .IC1Filter = 0,
        .IC2Polarity = TIM_ICPOLARITY_RISING,
        .IC2Selection = TIM_ICSELECTION_DIRECTTI,
        .IC2Prescaler = TIM_ICPSC_DIV1,
        .IC2Filter = 0,
    };
    if (mspInit(encoder) == false) {
        return false;
    }
    if (HAL_TIM_Encoder_Init(encoder->timHandle, &sConfig) != HAL_OK) {
        return false;
    }

    TIM_MasterConfigTypeDef sMasterConfig = {
        .MasterOutputTrigger = TIM_TRGO_RESET,
        .MasterOutputTrigger2 = TIM_TRGO2_RESET,
        .MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE
    };
    if (HAL_TIMEx_MasterConfigSynchronization(encoder->timHandle, &sMasterConfig) != HAL_OK) {
        return false;
    }
    return true;
}


void Encoder_Start(Encoder const *const encoder) {
    assert(encoder != NULL);

    HAL_TIM_Encoder_Start(encoder->timHandle, TIM_CHANNEL_ALL);
}


void Encoder_Stop(Encoder const *const encoder) {
    assert(encoder != NULL);

    HAL_TIM_Encoder_Stop(encoder->timHandle, TIM_CHANNEL_ALL);
}


uint16_t Encoder_GetMaxCount(Encoder const *const encoder) {
    assert(encoder != NULL);

    return encoder->timHandle->Init.Period;
}


int16_t Encoder_GetCount(Encoder const *const encoder) {
    assert(encoder != NULL);

    return __HAL_TIM_GET_COUNTER(encoder->timHandle);
}


void Encoder_SetCount(Encoder const *const encoder, int16_t count) {
    assert(encoder != NULL);

    uint16_t unsignedCount = (uint16_t)count;
    uint16_t maxCount = Encoder_GetMaxCount(encoder);
    if (unsignedCount > maxCount) {
        unsignedCount = unsignedCount % maxCount;
    }
    __HAL_TIM_SET_COUNTER(encoder->timHandle, unsignedCount);
}


void Encoder_ResetCount(Encoder const *const encoder) {
    assert(encoder != NULL);

    Encoder_SetCount(encoder, 0);
}

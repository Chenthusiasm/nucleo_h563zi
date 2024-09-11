//------------------------------------------------------------------------------------------------//
/** @file       PWM.c
 *  @brief      PWM API
 *  @details    See PwmDemo() for testing API demonstration
 *
 *  @author     Justin Reina, Firmware Engineer, Whisker
 *  @created    7/12/24
 *  @last rev   9/6/24 'r2.3'
 *
 *  @section    Opens
 *      PwmStop()
 *      PwmDisable()
 *      PwmClose()
 *      PwmUpdate()
 *      Description for DEMO_PERIOD use of '-1' operation
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


/* Internal functions ----------------------------------------------------------------------------*/


/* External function prototypes ------------------------------------------------------------------*/


/* External functions ----------------------------------------------------------------------------*/

Encoder Encoder_ctor(TIM_HandleTypeDef *const timHandle, TIM_TypeDef *const timPtr) {
    assert(timHandle != NULL);
    assert(timPtr != NULL);

    Encoder encoder = {
        .timHandle = timHandle,
    };
    encoder.timHandle->Instance = timPtr;

    return encoder;
}


bool Encoder_Init(Encoder *const encoder, uint16_t maxCount) {
    assert(encoder != NULL);
    assert(maxCount != 0u);

    encoder->timHandle->Init.Prescaler = 0u;
    encoder->timHandle->Init.CounterMode = TIM_COUNTERMODE_UP;
    encoder->timHandle->Init.Period = maxCount;
    encoder->timHandle->Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
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
    if (HAL_TIM_Encoder_Init(encoder->timHandle, &sConfig) != HAL_OK) {
        return false;
    }

    TIM_MasterConfigTypeDef sMasterConfig = {
        .MasterOutputTrigger = TIM_TRGO_RESET,
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

/**
 * @file    Encoder.c
 * @brief   Implementation of the quadrature encoder driver.
 * @note    The driver is reentrant from the defined structures; the driver may not be reentrant
 *          if different instances of the structures are constructed using the same HW peripherals,
 *          specifically timers (TIM). To ensure reentrancy, the Mutex module must be configured
 *          to use an RTOS mutex.
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

/* Default timeout value in milliseconds to acquire the HW TIM mutex */
#define TIM_MUTEX_TIMEOUT_MS            (5u)


/* Internal macro --------------------------------------------------------------------------------*/


/* Internal variables ----------------------------------------------------------------------------*/


/* Internal constants ----------------------------------------------------------------------------*/


/* Internal function prototypes ------------------------------------------------------------------*/


/* Internal functions ----------------------------------------------------------------------------*/


/* External function prototypes ------------------------------------------------------------------*/


/* External functions ----------------------------------------------------------------------------*/

/**
 * @brief   Constructor that initializes the parameters of the Encoder struct.
 * @param[in]   timHandle           Handle to the TIM handle used to control the encoder function.
 * @param[in]   timPtr              Pointer to the specific HW TIM registers.
 * @param[in]   timMutexPtr         Pointer to the timer (TIM) Mutex; can be NULL to indicate no
 *                                  mutex to protect the TIM resource.
 * @return  The new Encoder struct (to be copied upon assignment).
 */
Encoder Encoder_ctor(TIM_HandleTypeDef *const timHandle, TIM_TypeDef *const timPtr,
                     Mutex *const timMutexPtr) {
    assert(timHandle != NULL);
    assert(timPtr != NULL);

    Encoder self = {
        .timHandle = timHandle,
        .timMutexPtr = timMutexPtr
    };
    self.timHandle->Instance = timPtr;

    return self;
}


/**
 * @brief   Initializes the encoder instance.
 * @param[in]   self        Pointer to the Encoder struct that represents the encoder instance.
 * @param[in]   maxCount    Maximum count value the TIM counts up to (1 - 65535; 0 is invalid).
 * @param[in]   filter      The encoder filter value (0 - 15).
 * @note    The _Init function is empty; the current implementation takes advantage of the
 *          STM32CubeIDE/MX generated source from the *.ioc configuration.
 * @note    The maxCount and filter parameters are UNUSED for now but they would be encoder settings
 *          that can be configurable when the _Init function is fully implemented.
 * @return  If the encoder is successfully initialized, true; otherwise, false.
 */
bool Encoder_Init(Encoder const *const self, uint16_t maxCount, uint8_t filter) {
    UNUSED(maxCount);
    UNUSED(filter);
    return true;
}


/**
 * @brief   Starts the encoder counting.
 * @param[in]   self    Pointer to the Encoder struct that represents the encoder instance.
 */
void Encoder_Start(Encoder const *const self) {
    assert(self != NULL);

    if (Mutex_Acquire(self->timMutexPtr, TIM_MUTEX_TIMEOUT_MS) == false) {
        return;
    }
    HAL_TIM_Encoder_Start(self->timHandle, TIM_CHANNEL_ALL);
    Mutex_Release(self->timMutexPtr);
}


/**
 * @brief   Stops the encoder counting.
 * @param[in]   self    Pointer to the Encoder struct that represents the encoder instance.
 */
void Encoder_Stop(Encoder const *const self) {
    assert(self != NULL);

    if (Mutex_Acquire(self->timMutexPtr, TIM_MUTEX_TIMEOUT_MS) == false) {
        return;
    }
    HAL_TIM_Encoder_Stop(self->timHandle, TIM_CHANNEL_ALL);
    Mutex_Release(self->timMutexPtr);
}


/**
 * @brief   Accessor to get the encoder TIM's maximum count.
 * @param[in]   self    Pointer to the Encoder struct that represents the encoder instance.
 * @return  The encoder's maximum count.
 */
uint16_t Encoder_GetMaxCount(Encoder const *const self) {
    assert(self != NULL);

    if (Mutex_Acquire(self->timMutexPtr, TIM_MUTEX_TIMEOUT_MS) == false) {
        return 0u;
    }
    return self->timHandle->Init.Period;
    Mutex_Release(self->timMutexPtr);
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

    // do not acquire the mutex since we're only reading the register
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

    if (Mutex_Acquire(self->timMutexPtr, TIM_MUTEX_TIMEOUT_MS) == false) {
        return;
    }
    __HAL_TIM_SET_COUNTER(self->timHandle, unsignedCount);
    Mutex_Release(self->timMutexPtr);
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

    // Mutex is not acquired or released; Encoder_SetCounter will handle the Mutex
    Encoder_SetCounter(self, 0);
}

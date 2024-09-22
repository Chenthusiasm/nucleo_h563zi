/**
 * @file    Timer.c
 * @brief   Interface file for the Timer driver.
 * @note    The driver is reentrant from the defined structures; the driver may not be reentrant
 *          if different instances of the structures are constructed using the same HW peripherals,
 *          specifically timers (TIM). To ensure reentrancy, the Mutex module must be configured
 *          to use an RTOS mutex.
 * @note    This driver is not fully platform since it's possible for multiple instances to be
 *          constructed with the same TIM and channel leading to multiple instances being able to
 *          configure the same hardware independent of each other.
 * @note    The timer driver only supports the following modes: basic PWM mode and encoder mode
 *          (both channels 1 and 2).
 * @section Legal Disclaimer
 *      ©2024 Whisker, All rights reserved. All contents of this source file and/or any other
 *      related source files are the explicit property of Whisker. Do not distribute. Do not copy.
 */

/* Includes --------------------------------------------------------------------------------------*/

#include <assert.h>

#include "Timer.h"


/* Internal typedef ------------------------------------------------------------------------------*/

/**
 * @enum    Channel_Mode_t
 * @brief   Enumeration of the different possible modes for a timer channel.
 */
typedef enum {
    CHANNEL_MODE_RESET = 0u,    /*!< Reset mode; the mode has not set */
    CHANNEL_MODE_PWM,           /*!< PWM output generation mode */
    CHANNEL_MODE_ENCODER,       /*!< Encoder (channel 1 and channel 2) mode */
} Channel_Mode_t;


/* Internal define -------------------------------------------------------------------------------*/

/* Default timeout value in milliseconds to acquire the HW TIM mutex. */
#define TIMER_MUTEX_TIMEOUT_MS          (5u)


/* Internal macro --------------------------------------------------------------------------------*/


/* Internal variables ----------------------------------------------------------------------------*/


/* Internal constants ----------------------------------------------------------------------------*/


/* Internal function prototypes ------------------------------------------------------------------*/


/* Internal functions ----------------------------------------------------------------------------*/

/**
 * @brief   General verification if the timer channel is a valid value.
 * @param[in]   channel The channel to check.
 * @return  If the channel is capable of PWM functionality, true; otherwise, false. Note that this
 *          is a general verification: the STM32 TIM peripheral can have up to 6 channels, but only
 *          channels 1 - 4 support PWM functionality and not all TIM support 6 channels.
 */
static bool isChannelValid(Timer_Channel_t channel) {
    if ((channel != TIMER_CHANNEL_1) ||
        (channel != TIMER_CHANNEL_2) ||
        (channel != TIMER_CHANNEL_3) ||
        (channel != TIMER_CHANNEL_4) ||
        (channel != TIMER_CHANNEL_5) ||
        (channel != TIMER_CHANNEL_6)) {
        return false;
    }
    return true;
}


/**
 * @brief   General verification if the timer channel for PWM output generation.
 * @param[in]   channel The channel to check.
 * @return  If the channel is capable of PWM functionality, true; otherwise, false. Note that this
 *          is a general verification: the STM32 TIM peripheral can have up to 6 channels, but only
 *          channels 1 - 4 support PWM functionality and not all TIM support 6 channels.
 */
static bool isPWMChannelValid(Timer_Channel_t channel) {
    if ((channel != TIMER_CHANNEL_1) ||
        (channel != TIMER_CHANNEL_2) ||
        (channel != TIMER_CHANNEL_3) ||
        (channel != TIMER_CHANNEL_4)) {
        return false;
    }
    return true;
}


/**
 * @brief   Verifies that the specific TIM peripheral and channel combination is capable of PWM
 *          output generation functionality.
 * @param[in]   timPtr  The TIM peripheral to check.
 * @param[in]   channel The channel to check.
 * @note    Since this is a static function, pointer parameters are assumed to be non-NULL because
 *          the file-scope calls ensures that the pointers are not NULL.
 * @return  If the TIM and channel combination is capable of PWM output generation functionality,
 *          true; otherwise, false.
 */
static bool validatePWMChannel(TIM_TypeDef const *const timPtr, Timer_Channel_t channel) {
    // verify that the channel is valid; even though Timer_Channel_t is an enumeration, enumerations
    // are not strongly typed in C
    if (isPWMChannelValid(channel) == false) {
        return false;
    }
    // TIM6 and TIM7 are basic timers and don't support PWM
    if ((timPtr == TIM6) || (timPtr == TIM7)) {
        return false;
    }
    // TIM12 and TIM 15 both have 2 channels that support PWM
    if ((timPtr == TIM12) || (timPtr == TIM15)) {
        if ((channel != TIMER_CHANNEL_1) || (channel != TIMER_CHANNEL_2)) {
            return false;
        }
        return true;
    }
    // TIM13, TIM14, TIM16, and TIM17 only have 1 channel that supports PWM
    if ((timPtr == TIM13) || (timPtr == TIM14) || (timPtr == TIM16) || (timPtr == TIM17)) {
        if (channel != TIMER_CHANNEL_1) {
            return false;
        }
    }
    return true;
}


/**
 * @brief   Verifies that the specific TIM peripheral is capable of encoder functionality.
 * @param[in]   timPtr  The TIM peripheral to check.
 * @note    Since this is a static function, pointer parameters are assumed to be non-NULL because
 *          the file-scope calls ensures that the pointers are not NULL.
 * @return  If the TIM is capable of encoder functionality, true; otherwise, false.
 */
static bool validateEncoderTimer(TIM_TypeDef const *const timPtr) {
    // TIM6 and TIM7 are basic timers and don't support encoder counting
    if ((timPtr == TIM6) || (timPtr == TIM7)) {
        return false;
    }
    // TIM12 and TIM 15 both have 2 channels but don't support encoder counting
    if ((timPtr == TIM12) || (timPtr == TIM15)) {
        return false;
    }
    // TIM13, TIM14, TIM16, and TIM17 only have 1 channel; encoder requires channel 1 and 2
    if ((timPtr == TIM13) || (timPtr == TIM14) || (timPtr == TIM16) || (timPtr == TIM17)) {
        return false;
    }
    return true;
}


/**
 * @brief   Accessor to get the TIM register.
 * @param[in]   self    Pointer to the Timer struct that represents the timer instance.
 * @note    Since this is a static function, pointer parameters are assumed to be non-NULL because
 *          the file-scope calls ensures that the pointers are not NULL.
 * @return  Pointer to the TIM register.
 */
TIM_TypeDef *const getTIMRegister(Timer const *const self) {
    return self->timHandle->Instance;
}


/* External function prototypes ------------------------------------------------------------------*/


/* External functions ----------------------------------------------------------------------------*/

/**
 * @brief   Constructor that initializes the parameters of the Timer struct.
 * @param[in]   timHandle   Handle to the TIM handle.
 * @param[in]   timPtr      Pointer to the specific HW TIM register.
 * @param[in]   timMutexPtr Pointer to the timer (TIM) Mutex.
 * @note    All Timer driver struct instances must be constructed using this constructor since it
 *          verifies that the Timer struct parameters are valid.
 * @return  The new Timer struct (to be copied upon assignment).
 */
Timer Timer_ctor(TIM_HandleTypeDef *const timHandle, TIM_TypeDef *const timPtr,
             Mutex *const timMutexPtr) {
    assert(timHandle != NULL);
    assert(timPtr != NULL);

    Timer self = {
        .timHandle = timHandle,
        .timMutexPtr = timMutexPtr,
    };
    self.timHandle->Instance = timPtr;

    return self;
}


/**
 * @brief   Set the Timer driver to PWM output generation mode on specific channels.
 * @param[in]   self    Pointer to the Timer struct that represents the timer instance.
 * @param[in]   channel The timer channel to set to the specific mode.
 * @return  The specific Timer_Err_t which indicates the specific error code if the function
 *          couldn't be executed successfully. If the function executes successfully,
 *          TIMER_ERR_NONE.
 */
Timer_Err_t Timer_SetModePWM(Timer *const self, Timer_Channel_t channel) {
    assert(self != NULL);
    if (isChannelValid(channel) == false) {
        return TIMER_ERR_INVALID_PARAM;
    }
    if (validatePWMChannel(getTIMRegister(self), channel) == false) {
        return TIMER_ERR_MODE_INVALID;
    }
    if (Timer_IsModeEncoder(self) == true) {
        return TIMER_ERR_MODE_CONFLICT;
    }
    if (self->channelMode[channel] != CHANNEL_MODE_RESET) {
        return TIMER_ERR_MODE_CONFLICT;
    }
    self->channelMode[channel] = CHANNEL_MODE_PWM;
    return TIMER_ERR_NONE;
}


/**
 * @brief   Set the Timer driver to encoder mode.
 * @param[in]   self    Pointer to the Timer struct that represents the timer instance.
 * @return  The specific PWM_Err_t which indicates the specific error code if the function couldn't
 *          be executed successfully. If the function executes successfully, PWM_ERR_NONE.
 */
Timer_Err_t Timer_SetModeEncoder(Timer *const self) {
    assert(self != NULL);

    if (validateEncoderTimer(getTIMRegister(self)) == false) {
        return TIMER_ERR_MODE_INVALID;
    }
    if ((self->channelMode[TIMER_CHANNEL_1] != CHANNEL_MODE_RESET) &&
        (self->channelMode[TIMER_CHANNEL_2] != CHANNEL_MODE_RESET)) {
        return TIMER_ERR_MODE_CONFLICT;
    }
    self->channelMode[TIMER_CHANNEL_1] = CHANNEL_MODE_ENCODER;
    self->channelMode[TIMER_CHANNEL_2] = CHANNEL_MODE_ENCODER;
    return TIMER_ERR_NONE;
}


/**
 * @brief   Accessor to get the TIM handle associated with the Timer.
 * @param[in]   self    Pointer to the Timer struct that represents the timer instance.
 * @return  TIM handle associated with the Timer.
 */
TIM_HandleTypeDef *const Timer_GetTIMHandle(Timer const *const self) {
    assert (self != NULL);

    return self->timHandle;
}


/**
 * @brief   Accessor to get the frequency of the source clock for the specific TIM instance.
 * @param[in]   self    Pointer to the Timer struct that represents the timer instance.
 * @return  Frequency of the source clock in Hz.
 */
uint32_t Timer_GetClockFrequency_hz(Timer const *const self) {
    assert(self != NULL);

    // TIM1 and TIM8 use PCLK2 as the source clock
    if ((self->timHandle->Instance == TIM1) || (self->timHandle->Instance == TIM8)) {
        return HAL_RCC_GetPCLK2Freq();
    }
    // other TIMx use PCLK1 as the source clock
    return HAL_RCC_GetPCLK1Freq();
}


/**
 * @brief   Accessor to get the TIM peripheral prescaler value (TIM.PSC + 1).
 * @param[in]   self    Pointer to the Timer struct that represents the timer instance.
 * @note    The STM32H5xx HAL SDK doesn't have a HAL function to get the TIM prescaler value so this
 *          function was implemented.
 * @return  The TIM peripheral prescaler value (TIM.PSC + 1).
 */
uint32_t Timer_GetPrescaler(Timer const *const self) {
    assert(self != NULL);

    return (self->timHandle->Instance->PSC + 1u);
}


/**
 * @brief   Converts the Timer channel into the HAL-usable channel mask.
 * @param[in]   channel The timer channel to get the mask of.
 * @return  The HAL channel mask of the specified channel.
 */
uint32_t Timer_GetChannelMask(Timer_Channel_t channel) {
    assert(isChannelValid(channel) == true);

    static const uint32_t ChannelMask[] = {
        [0] = TIM_CHANNEL_1,
        [1] = TIM_CHANNEL_2,
        [2] = TIM_CHANNEL_3,
        [3] = TIM_CHANNEL_4,
        [4] = TIM_CHANNEL_5,
        [5] = TIM_CHANNEL_6,
    };
    return ChannelMask[channel];
}


/**
 * @brief   Gets the HAL-usable channel mask for all Timer channels.
 * @return  The HAL channel mask of all the timer channels.
 */
uint32_t Timer_GetChannelMaskAll(void) {
    return TIM_CHANNEL_ALL;
}


/**
 * @brief   Check if the Timer is configured for encoder mode.
 * @param[in]   self    Pointer to the Timer struct that represents the timer instance.
 * @return  If the timer is configure for encoder mode, true; otherwise, false.
 */
bool Timer_IsModeEncoder(Timer const *const self) {
    assert(self != NULL);

    return ((self->channelMode[TIMER_CHANNEL_1] == CHANNEL_MODE_ENCODER) ||
            (self->channelMode[TIMER_CHANNEL_2] == CHANNEL_MODE_ENCODER));
}


/**
 * @brief   Check if the Timer channel is configured for PWM mode.
 * @param[in]   self    Pointer to the Timer struct that represents the timer instance.
 * @return  If the timer channel is configure for PWM mode, true; otherwise, false.
 */
bool Timer_IsModePWM(Timer const *const self, Timer_Channel_t channel) {
    assert(self != NULL);

    if (isChannelValid(channel) == false) {
        return false;
    }
    return (self->channelMode[channel] == CHANNEL_MODE_PWM);
}


/**
 * @brief   Acquire the timer mutex to protect the TIM hardware resource.
 * @param[in]   self        Pointer to the Mutex struct that represents the mutex instance.
 * @param[in]   timeout_ms  The timeout period in milliseconds to acquire the mutex. If 0, then no
 *                          timeout.
 * @return  If the mutex was successfully acquired, true; otherwise, false. If there's no RTOS mutex
 *          support, return true.
 */
bool Timer_AcquireMutex(Timer const *const self, uint32_t timeout_ms) {
    assert(self != NULL);

    return Mutex_Acquire(self->timMutexPtr, timeout_ms);
}


/**
 * @brief   Release the timer mutex so that the TIM hardware resource is available.
 * @param[in]   self    Pointer to the Mutex struct that represents the mutex instance.
 * @note    It's possible that two different Mutex instances consist of the same RTOS mutex handle.
 *          In such cases, it's possible that acquiring one mutex but releasing the other mutex will
 *          lead to the mutex being successfully released since they both use the same mutex handle.
 * @return  If the mutex was successfully released, true; otherwise, false. If there's no mutex,
 *          return true.
 */
bool Timer_ReleaseMutex(Timer const *const self) {
    assert(self != NULL);

    return Mutex_Release(self->timMutexPtr);
}

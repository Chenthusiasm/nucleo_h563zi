/**
 * @file    Timer.h
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
#ifndef TIMER_H_
#define TIMER_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------------------------- */

#include <stdbool.h>
#include <stdint.h>

#include "Mutex.h"
#include "stm32h5xx_hal.h"


/* External typedef ------------------------------------------------------------------------------*/

/**
 * @brief   Type definition for the Timer channel.
 * @note    0u = channel 1, 1u = channel 2, 2u = channel 3 ... 5u = channel 6.
 */
typedef uint8_t Timer_Channel_t;


/**
 * @brief   Type definition for the Timer channel mask used by the HAL layer.
 * @note    See TIM_CHANNEL_x (where x = 0 - 6, ALL).
 */
typedef uint8_t Timer_ChanelMask_t;


/**
 * @enum    Timer_Err_t
 * @brief   Enumeration of the different PWM function return error codes.
 */
typedef enum {
    TIMER_ERR_NONE = 0u,        /*!< No error */
    TIMER_ERR_NULL_PARAM,       /*!< An input parameter is NULL which is invalid for that parameter;
                                     assert checks should catch these (possibly UNUSED) */
    TIMER_ERR_INVALID_PARAM,    /*!< An input parameter had an invalid value */
    TIMER_ERR_MODE_RESET,       /*!< The timer channel mode has not been set yet (reset) */
    TIMER_ERR_MODE_INVALID,     /*!< The mode is invalid for the timer or the timer and channel */
    TIMER_ERR_MODE_CONFLICT,    /*!< The timer channel mode is being set to something that conflicts
                                     with an already set mode or the overall timer mod config */
} Timer_Err_t;


/* The maximum number of TIM channels; defined for use in the Timer struct */
#define TIMER_MAX_NUM_CHANNELS          (6u)

/**
 * @struct  Timer
 * @brief   Type definition of a structure that aggregates key components needed to manage the
 *          MCU TIM hardware peripheral and its different modes.
 * @var Timer::timHandle    Handle of the MCU timer (TIM) peripheral.
 * @var Timer::timMutexPtr  Pointer to the timer (TIM) Mutex.
 * @var Timer::channelMode  Array of modes for all the possible timer channels.
 */
typedef struct {
    TIM_HandleTypeDef *timHandle;
    Mutex *timMutexPtr;
    uint8_t channelMode[TIMER_MAX_NUM_CHANNELS];
} Timer;


/* Defines ---------------------------------------------------------------------------------------*/


/* External macro --------------------------------------------------------------------------------*/


/* External variables ----------------------------------------------------------------------------*/


/* External function prototypes ------------------------------------------------------------------*/


/* External functions ----------------------------------------------------------------------------*/

Timer Timer_ctor(TIM_HandleTypeDef *const timHandle, TIM_TypeDef *const timPtr,
             Mutex *const timMutexPtr);
Timer_Err_t Timer_SetModePWM(Timer *const self, Timer_Channel_t channel);
Timer_Err_t Timer_SetModeEncoder(Timer *const self);
TIM_HandleTypeDef *const Timer_GetTIMHandle(Timer const *const self);
uint32_t Timer_GetClockFrequency_hz(Timer const *const self);
uint32_t Timer_GetPrescaler(Timer const *const self);
Timer_ChanelMask_t Timer_GetChannelMask(Timer_Channel_t channel);
Timer_ChanelMask_t Timer_GetChannelMaskAll(void);
bool Timer_IsModeEncoder(Timer const *const self);
bool Timer_IsModePWM(Timer const *const self, Timer_Channel_t channel);
bool Timer_AcquireMutex(Timer const *const self, uint32_t timeout_ms);
bool Timer_ReleaseMutex(Timer const *const self);


#ifdef __cplusplus
}
#endif

#endif /* TIMER_H_ */

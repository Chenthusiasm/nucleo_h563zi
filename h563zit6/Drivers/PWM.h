/**
 * @file    PWM.h
 * @brief   Interface file for the PWM driver.
 * @note    The driver is reentrant from the defined structures; the driver may not be reentrant
 *          if different instances of the structures are constructed using the same HW peripherals,
 *          specifically timers (TIM). To ensure reentrancy, the Mutex module must be configured
 *          to use an RTOS mutex.
 * @note    This driver is not fully platform since it's possible for multiple instances to be
 *          constructed with the same TIM and channel leading to multiple instances being able to
 *          configure the same hardware independent of each other.
 * @section Legal Disclaimer
 *      ©2024 Whisker, All rights reserved. All contents of this source file and/or any other
 *      related source files are the explicit property of Whisker. Do not distribute. Do not copy.
 */
#ifndef PWM_H_
#define PWM_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------------------------- */

#include <stdbool.h>
#include <stdint.h>

#include "Mutex.h"
#include "stm32h5xx_hal.h"
#include "Timer.h"


/* External typedef ------------------------------------------------------------------------------*/

/**
 * @enum    PWM_Err_t
 * @brief   Enumeration of the different PWM function return error codes.
 */
typedef enum {
    PWM_ERR_NONE = 0u,          /*!< No error */
    PWM_ERR_NULL_PARAM,         /*!< An input parameter is NULL which is invalid for that parameter;
                                     assert checks should catch these (possibly UNUSED) */
    PWM_ERR_INVALID_PARAM,      /*!< An input parameter had an invalid value */
    PWM_ERR_RESOURCE_BLOCKED,   /*!< The HW resource is currently blocked */
    PWM_ERR_UNINITIALIZED,      /*!< The PWM is not initialized */
    PWM_ERR_HAL,                /*!< HAL level PWM error */
    PWM_ERR_STARTED,            /*!< The PWM has already started */
    PWM_ERR_STOPPED,            /*!< The PWM is stopped or has already stopped */
} PWM_Err_t;


/**
 * @struct  PWM
 * @brief   Type definition of a structure that aggregates key components needed for the PWM signal
 *          generation to a pin.
 * @var PWM::timerPtr       Pointer to the Timer driver.
 * @var PWM::channelMask    TIM channel mask.
 * @var PWM::state          The current state of the PWM driver.
 */
typedef struct {
    Timer *timerPtr;
    uint32_t channelMask;
    uint8_t state;
} PWM;


/* Defines ---------------------------------------------------------------------------------------*/


/* External macro --------------------------------------------------------------------------------*/


/* External variables ----------------------------------------------------------------------------*/


/* External function prototypes ------------------------------------------------------------------*/


/* External functions ----------------------------------------------------------------------------*/

PWM PWM_ctor(Timer *const timerPtr, Timer_Channel_t channel);
PWM_Err_t PWM_Init(PWM *const self, uint32_t switchingFrequency_hz, uint16_t dutyCycle_tenthPct);
PWM_Err_t PWM_Start(PWM *const self);
PWM_Err_t PWM_Stop(PWM *const self);
uint32_t PWM_GetSwitchingFrequency_hz(PWM const *const self);
uint16_t PWM_GetDutyCycle_tenthPct(PWM const *const self);
PWM_Err_t PWM_SetDutyCycle(PWM const *const self, uint16_t dutyCycle_tenthPct);
PWM_Err_t PWM_SetHigh(PWM const *const self);
PWM_Err_t PWM_SetLow(PWM const *const self);


#ifdef __cplusplus
}
#endif

#endif /* PWM_H_ */

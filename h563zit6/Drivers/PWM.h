/**
 * @file    PWM.h
 * @brief   Interface file for the PWM driver.
 * @note    The driver is reentrant from the defined structures; the driver may not be reentrant
 *          if different instances of the structures are constructed using the same HW peripherals,
 *          specifically timers (TIM). To ensure reentrancy, the Mutex module must be configured
 *          to use an RTOS mutex.
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


/* External typedef ------------------------------------------------------------------------------*/

/**
 * @brief   Enumeration of the different channels associated with PWM output generation using the
 *          MCU timer (TIM) peripheral.
 */
typedef enum {
    PWM_CHANNEL_1 = TIM_CHANNEL_1,
    PWM_CHANNEL_2 = TIM_CHANNEL_2,
    PWM_CHANNEL_3 = TIM_CHANNEL_3,
    PWM_CHANNEL_4 = TIM_CHANNEL_4
    // TIM1 and TIM8 have 6 channels, but PWM output is not available on channels 5 or 6 so they
    // are not defined
} PWM_Channel_t;


/**
 * @struct  PWM
 * @brief   Type definition of a structure that aggregates key components needed for the PWM signal
 *          generation to a pin.
 * @var PWM.timHandle           Handle of the MCU timer (TIM) peripheral.
 * @var PWM.timMutexPtr         Pointer to the timer (TIM) Mutex.
 * @var PWM.channel             TIM channel number the generated PWM is output to.
 */
typedef struct {
    TIM_HandleTypeDef *timHandle;
    Mutex* timMutexPtr;
    PWM_Channel_t channel;
} PWM;


/* Defines ---------------------------------------------------------------------------------------*/


/* External macro --------------------------------------------------------------------------------*/


/* External variables ----------------------------------------------------------------------------*/


/* External function prototypes ------------------------------------------------------------------*/


/* External functions ----------------------------------------------------------------------------*/

PWM PWM_ctor(TIM_HandleTypeDef *const timHandle, TIM_TypeDef *const timPtr,
             Mutex *const timMutexPtr,
             PWM_Channel_t channel);
bool PWM_Init(PWM const *const self, uint32_t switchingFrequency_hz, uint16_t dutyCycle_tenthPct);
void PWM_Start(PWM const *const self);
void PWM_Stop(PWM const *const self);
uint32_t PWM_GetSwitchingFrequency_hz(PWM const *const self);
uint16_t PWM_GetDutyCycle_tenthPct(PWM const *const self);
void PWM_SetDutyCycle(PWM const *const self, uint16_t dutyCycle_tenthPct);
void PWM_SetHigh(PWM const *const self);
void PWM_SetLow(PWM const *const self);


#ifdef __cplusplus
}
#endif

#endif /* PWM_H_ */

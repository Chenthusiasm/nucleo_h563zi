/**
 * @file    DRV8870.h
 * @brief   Interface file for the DRV8870 motor driver.
 * @section Legal Disclaimer
 *      ©2024 Whisker, All rights reserved. All contents of this source file and/or any other
 *      related source files are the explicit property of Whisker. Do not distribute. Do not copy.
 */
#ifndef DRV8870_H_
#define DRV8870_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------------------------- */

#include <stdbool.h>
#include <stdint.h>

#include "PWM.h"


/* External typedef ------------------------------------------------------------------------------*/

typedef enum {
    DRV8870_ROTATION_CLOCKWISE = 0u,
    DRV8870_ROTATION_COUNTERCLOCKWISE
} DRV8870_Rotation_t;


/**
 * @struct  DRV8870
 * @brief   Type definition of a structure that aggregates key components needed for the DRV8870
 *          motor driver.
 * @var PWM.timHandle           Handle of the MCU timer (TIM) peripheral.
 * @var PWM.timMutexPtr         Pointer to the timer (TIM) Mutex.
 * @var PWM.channel             TIM channel number the generated PWM is output to.
 */
typedef struct {
    PWM *pwmPtr0;
    PWM *pwmPtr1;
} DRV8870;


/* Defines ---------------------------------------------------------------------------------------*/


/* External macro --------------------------------------------------------------------------------*/


/* External variables ----------------------------------------------------------------------------*/


/* External function prototypes ------------------------------------------------------------------*/


/* External functions ----------------------------------------------------------------------------*/

DRV8870 DRV8870_ctor(PWM *const pwmPtr0, PWM *const pwmPtr1);
bool DRV8870_Init(DRV8870 const *const self);
void DRV8870_SetMotor(DRV8870 const *const self, DRV8870_Rotation_t rotation,
                      uint16_t speed_tenthPct);
void DRV8870_StopMotor(DRV8870 const *const self);
void DRV8870_StopMotorCoast(DRV8870 const *const self);


#ifdef __cplusplus
}
#endif

#endif /* DRV8870_H_ */

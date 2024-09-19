/**
 * @file    DRV8870.h
 * @brief   Interface file for the DRV8870 motor driver.
 * @note    Refer to https://www.ti.com/product/DRV8870.
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
    DRV8870_DIRECTION_FORWARD = 0u,
    DRV8870_DIRECTION_REVERSE
} DRV8870_Direction_t;


/**
 * @struct  DRV8870
 * @brief   Type definition of a structure that aggregates key components needed for the DRV8870
 *          motor driver.
 * @var DRV8870.pwmPtr0 Pointer to the PWM struct representing the IN0 line.
 * @var DRV8870.pwmPtr1 Pointer to the PWM struct representing the IN1 line.
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
void DRV8870_Drive(DRV8870 const *const self, DRV8870_Direction_t direction,
                   uint16_t strength_tenthPct);
void DRV8870_Brake(DRV8870 const *const self);
void DRV8870_Coast(DRV8870 const *const self);


#ifdef __cplusplus
}
#endif

#endif /* DRV8870_H_ */

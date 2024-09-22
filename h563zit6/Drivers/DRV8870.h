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

/**
 * @enum    DRV8870_Err_t
 * @brief   Enumeration of the different DRV8870 motor driver function return error codes.
 */
typedef enum {
    DRV8870_ERR_NONE = 0u,          /*!< No error */
    DRV8870_ERR_NULL_PARAM,         /*!< An input parameter is NULL which is invalid for that
                                         parameter; assert checks should catch these (possibly
                                         UNUSED) */
    DRV8870_ERR_INVALID_PARAM,      /*!< An input parameter had an invalid value */
    DRV8870_ERR_RESOURCE_BLOCKED,   /*!< The HW resource is currently blocked */
    DRV8870_ERR_UNINITIALIZED,      /*!< The DRV8870 motor is not initialized */
    DRV8870_ERR_PWM_INIT,           /*!< The PWM initialization failed */
    DRV8870_ERR_PWM_STATE,          /*!< The PWM state is invalid */
} DRV8870_Err_t;

/**
 * @enum    DRV8870_Direction_t
 * @brief   Enumeration of the different driver directions for the DRV8870 motor driver.
 */
typedef enum {
    DRV8870_DIRECTION_STOPPED = 0u, /*<! Drive is stopped */
    DRV8870_DIRECTION_COAST,        /*<! Coasting stop */
    DRV8870_DIRECTION_FORWARD,      /*<! Forward drive direction */
    DRV8870_DIRECTION_REVERSE,      /*<! Reverse drive direction */
} DRV8870_Direction_t;


/**
 * @struct  DRV8870
 * @brief   Type definition of a structure that aggregates key components needed for the DRV8870
 *          motor driver to function.
 * @var DRV8870.pwmIN0  PWM driver struct representing the IN0 line.
 * @var DRV8870.pwmIN1  PWM driver struct representing the IN1 line.
 * @var DRV8870.state   The current state of the motor driver.
 */
typedef struct {
    PWM pwmIN0;
    PWM pwmIN1;
    uint8_t state;
} DRV8870;


/* Defines ---------------------------------------------------------------------------------------*/


/* External macro --------------------------------------------------------------------------------*/


/* External variables ----------------------------------------------------------------------------*/


/* External function prototypes ------------------------------------------------------------------*/


/* External functions ----------------------------------------------------------------------------*/

DRV8870 DRV8870_ctor(Timer *const timerPtr, Timer_Channel_t channelIN0, Timer_Channel_t channelIN1);
DRV8870_Err_t DRV8870_Init(DRV8870 *const self, uint32_t pwmFrequency_hz);
DRV8870_Err_t DRV8870_Drive(DRV8870 *const self, DRV8870_Direction_t direction,
                   uint16_t strength_tenthPct);
DRV8870_Err_t DRV8870_Brake(DRV8870 *const self);
DRV8870_Err_t DRV8870_Coast(DRV8870 *const self);


#ifdef __cplusplus
}
#endif

#endif /* DRV8870_H_ */

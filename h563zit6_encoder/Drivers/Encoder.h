/**
 * @file    Encoder.h
 * @brief   Interface file for the quadrature encoder driver.
 * @note    The driver is reentrant from the defined structures; the driver may not be reentrant
 *          if different instances of the structures are constructed using the same HW peripherals,
 *          specifically timers (TIM). To ensure reentrancy, the Mutex module must be configured
 *          to use an RTOS mutex.
 * @section Legal Disclaimer
 *      ©2024 Whisker, All rights reserved. All contents of this source file and/or any other
 *      related source files are the explicit property of Whisker. Do not distribute. Do not copy.
 */
#ifndef ENCODER_H_
#define ENCODER_H_

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
 * @struct Encoder
 * @brief Type definition of a structure that aggregates key components needed for the Encoder to
 *        operate.
 * The Encoder struct consists of the key hardware peripherals that are controlled in order to
 * produce an incremental encoder functionality. These components include the a hardware timer and
 * two GPIO pins.
 * @var Encoder.timHandle           Handle of the MCU timer (TIM) peripheral.
 * @var Encoder.timMutexPtr         Pointer to the timer (TIM) Mutex.
 */
typedef struct {
    TIM_HandleTypeDef *timHandle;
    Mutex* timMutexPtr;
} Encoder;


/* Defines ---------------------------------------------------------------------------------------*/



/* External macro --------------------------------------------------------------------------------*/


/* External variables ----------------------------------------------------------------------------*/


/* External function prototypes ------------------------------------------------------------------*/


/* External functions ----------------------------------------------------------------------------*/

Encoder Encoder_ctor(TIM_HandleTypeDef *const timHandle, TIM_TypeDef *const timPtr,
                     Mutex *const timMutexPtr);
bool Encoder_Init(Encoder const *const self, uint16_t maxCount, uint8_t filter);
void Encoder_Start(Encoder const *const self);
void Encoder_Stop(Encoder const *const self);
uint16_t Encoder_GetMaxCount(Encoder const *const self);
int16_t Encoder_GetCounter(Encoder const *const self);
void Encoder_SetCounter(Encoder const *const self, int16_t count);
void Encoder_ResetCounter(Encoder const *const self);


#ifdef __cplusplus
}
#endif

#endif /* ENCODER_H_ */

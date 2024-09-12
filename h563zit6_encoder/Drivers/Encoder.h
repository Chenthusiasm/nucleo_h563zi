/**
 *	@file		Encoder.h
 *  @brief		Interface file for the quadrature encoder driver.
 */
#ifndef ENCODER_H_
#define ENCODER_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------------------------- */

#include <stdbool.h>
#include <stdint.h>

#include "stm32h5xx_hal.h"


/* External typedef ------------------------------------------------------------------------------*/

typedef struct {
    TIM_HandleTypeDef *timHandle;
    struct {
        GPIO_TypeDef *portHandle;
        uint16_t pin;
    } gpio[2];
} Encoder;


/* Defines ---------------------------------------------------------------------------------------*/



/* External macro --------------------------------------------------------------------------------*/


/* External variables ----------------------------------------------------------------------------*/


/* External function prototypes ------------------------------------------------------------------*/


/* External functions ----------------------------------------------------------------------------*/

Encoder Encoder_ctor(TIM_HandleTypeDef *const timHandle, TIM_TypeDef *const timPtr,
                     GPIO_TypeDef *const gpioPortHandleCh1, uint16_t gpioPinCh1,
                     GPIO_TypeDef *const gpioPortHandleCh2, uint16_t gpioPinCh2);
bool Encoder_Init(Encoder const *const encoder, uint16_t maxCount);
void Encoder_Start(Encoder const *const encoder);
void Encoder_Stop(Encoder const *const encoder);
uint16_t Encoder_GetMaxCount(Encoder const *const encoder);
int16_t Encoder_GetCount(Encoder const *const encoder);
void Encoder_SetCount(Encoder const *const encoder, int16_t count);
void Encoder_ResetCount(Encoder const *const encoder);


#ifdef __cplusplus
}
#endif

#endif /* ENCODER_H_ */

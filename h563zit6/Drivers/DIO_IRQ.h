/**
 * @file    DIO_IRQ.h
 * @brief   Interface file for the DIO (digital input/output) driver (AKA GPIO, general purpose
 *          input/output).
 * @section Legal Disclaimer
 *      ©2024 Whisker, All rights reserved. All contents of this source file and/or any other
 *      related source files are the explicit property of Whisker. Do not distribute. Do not copy.
 */
#ifndef DIO_IRQ_H_
#define DIO_IRQ_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------------------------- */

#include <stdbool.h>
#include <stdint.h>

#include "DIO.h"
#include "stm32h5xx_hal.h"


/* External typedef ------------------------------------------------------------------------------*/

/**
 * @enum    DIO_IRQ_Err_t
 * @brief   Enumeration of the different DIO IRQ driver function return error codes.
 */
typedef enum {
    DIO_IRQ_ERR_NONE = 0u,          /*!< No error */
    DIO_IRQ_ERR_NULL_PARAM,         /*!< An input parameter is NULL which is invalid for that
                                         parameter; assert checks should catch these (possibly
                                         UNUSED) */
    DIO_IRQ_ERR_INVALID_PARAM,      /*!< An input parameter had an invalid value */
    DIO_IRQ_ERR_RESOURCE_BLOCKED,   /*!< The HW resource is currently blocked */
    DIO_IRQ_ERR_UNINITIALIZED,      /*!< The DIO_IRQ is not initialized */
    DIO_IRQ_ERR_REGISTERED,         /*!< The specific IRQ is already registered */
    DIO_IRQ_ERR_UNREGISTERED,       /*!< The specific IRQ is not registerd */
} DIO_IRQ_Err_t;


/* Defines ---------------------------------------------------------------------------------------*/


/* External macro --------------------------------------------------------------------------------*/


/* External variables ----------------------------------------------------------------------------*/


/* External function prototypes ------------------------------------------------------------------*/


/* External functions ----------------------------------------------------------------------------*/

DIO_IRQ_Err_t DIO_IRQ_Init(void);
DIO_IRQ_Err_t DIO_IRQ_Register(DIO_Pin_t pin, DIO_EXTICallback_t callback);
DIO_IRQ_Err_t DIO_IRQ_Enable(DIO_Pin_t pin, bool enable);
bool DIO_IRQ_IsEnabled(DIO_Pin_t pin);


#ifdef __cplusplus
}
#endif

#endif /* DIO_IRQ_H_ */

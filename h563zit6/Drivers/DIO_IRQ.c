/**
 * @file    DIO_IRQ.c
 * @brief   Interface file for the DIO (digital input/output) IRQ (interrupt request) driver. This
 *          is implemented as a singleton.
 * @section Legal Disclaimer
 *      ©2024 Whisker, All rights reserved. All contents of this source file and/or any other
 *      related source files are the explicit property of Whisker. Do not distribute. Do not copy.
 */

/* Includes --------------------------------------------------------------------------------------*/

#include <assert.h>

#include "DIO_IRQ.h"


/* Internal typedef ------------------------------------------------------------------------------*/

/**
 * @struct  EXTIConfig_t
 * @brief   Configuration of the EXTI.
 * @var EXTIConfig_t::callback  The callback function associated with the EXTI.
 * @var EXTIConfig_t::enable    Flag indicating if the callback is enabled.
 */
typedef struct {
    DIO_EXTICallback_t callback;
    GPIO_TypeDef *portHandle;
    bool enable;
} EXTIConfig_t;


/* The number of external interrupts/events. */
#define NUM_EXTI                        (16u)


/**
 * @struct  DIO_IRQ
 * @brief   Type definition of a structure that aggregates key components needed for the DIO
 *          interrupt driver to function.
 * @var DIO_IRQ::configs[]  Array of EXTIConfig_t configuration for each unique EXTI.
 */
typedef struct {
    EXTIConfig_t configs[NUM_EXTI];
} DIO_IRQ;


/* Internal define -------------------------------------------------------------------------------*/


/* Internal macro --------------------------------------------------------------------------------*/


/* Internal variables ----------------------------------------------------------------------------*/

/* Instance of the DIO_IRQ driver. Initialized to 0: callbacks are NULL, GPIO port handles are
 * NULL, and all callbacks are disabled. */
static DIO_IRQ self = { 0 };


/* Internal constants ----------------------------------------------------------------------------*/


/* Internal function prototypes ------------------------------------------------------------------*/


/* Internal functions ----------------------------------------------------------------------------*/


/* External function prototypes ------------------------------------------------------------------*/


/* External functions ----------------------------------------------------------------------------*/

/**
 * @brief   Initializes the DIO_IRQ singleton.
 * @return  The specific DIO_IRQ_Err_t which indicates the specific error code if the function
 *          couldn't be executed successfully. If the function executes successfully,
 *          DIO_IRQ_ERR_NONE.
 */
DIO_IRQ_Err_t DIO_IRQ_Init(void) {
    return DIO_IRQ_ERR_NONE;
}


/**
 * @brief   Register a DIO driver instance with the interrupt.
 * @param[in]   self    Pointer to the DIO struct that represents the digital I/O instance.
 * @return  The specific DIO_IRQ_Err_t which indicates the specific error code if the function
 *          couldn't be executed successfully. If the function executes successfully,
 *          DIO_IRQ_ERR_NONE.
 */
DIO_IRQ_Err_t DIO_IRQ_RegisterDIO(DIO const *const dioPtr) {
    assert(self != NULL);

    return DIO_IRQ_ERR_NONE;
}

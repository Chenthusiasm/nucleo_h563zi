/**
 * @file    RTOSHelper.c
 * @brief   Implementation of the RTOS helper functions.
 * @section Legal Disclaimer
 *      ©2024 Whisker, All rights reserved. All contents of this source file and/or any other
 *      related source files are the explicit property of Whisker. Do not distribute. Do not copy.
 */

/* Includes --------------------------------------------------------------------------------------*/

#include <stdint.h>

#include "stm32h5xx_hal.h"
#if defined(__has_include)
    #if __has_include("FreeRTOS.h")
        #include "FreeRTOS.h"
    #endif /* __has_include("FreeRTOS.h") */
#endif /* defined(__has_include) */


/* Internal typedef ------------------------------------------------------------------------------*/


/* Internal define -------------------------------------------------------------------------------*/

/* Number of milliseconds in 1 second */
#define MSEC_IN_SEC                     (1000u)


/* Internal macro --------------------------------------------------------------------------------*/


/* Internal variables ----------------------------------------------------------------------------*/


/* Internal constants ----------------------------------------------------------------------------*/


/* Internal function prototypes ------------------------------------------------------------------*/


/* Internal functions ----------------------------------------------------------------------------*/


/* External function prototypes ------------------------------------------------------------------*/


/* External functions ----------------------------------------------------------------------------*/

/**
 * @brief   Converts RTOS ticks to milliseconds.
 * @param[in]   ticks   The number of RTOS ticks.
 * @return  The equivalent milliseconds based on the number of RTOS ticks.
 */
uint32_t RTOS_ConvertTicksToMS(uint32_t ticks) {
    uint32_t var = ticks;
#if defined(configTICK_RATE_HZ)
    var = var * MSEC_IN_SEC / configTICK_RATE_HZ;
#endif /* defined(configTICK_RATE_HZ) */
    return var;
}


/**
 * @brief   Converts milliseconds to RTOS ticks.
 * @param[in]   ms  The number of milliseconds.
 * @return  The equivalent RTOS ticks based on milliseconds.
 */
uint32_t RTOS_ConvertMSToTicks(uint32_t ms) {
    uint32_t var = ms;
#if defined(configTICK_RATE_HZ)
    var = var * configTICK_RATE_HZ / MSEC_IN_SEC;
#endif /* defined(configTICK_RATE_HZ) */
    return var;
}

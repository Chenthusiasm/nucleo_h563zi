/**
 * @file    RTOSHelpler.h
 * @brief   Interface file for the RTOS helper functions.
 * @section Legal Disclaimer
 *      ©2024 Whisker, All rights reserved. All contents of this source file and/or any other
 *      related source files are the explicit property of Whisker. Do not distribute. Do not copy.
 */
#ifndef RTOS_HELPER_H_
#define RTOS_HELPER_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------------------------- */

#include <stdint.h>


/* External typedef ------------------------------------------------------------------------------*/


/* Defines ---------------------------------------------------------------------------------------*/


/* External macro --------------------------------------------------------------------------------*/


/* External variables ----------------------------------------------------------------------------*/


/* External function prototypes ------------------------------------------------------------------*/


/* External functions ----------------------------------------------------------------------------*/

uint32_t RTOSHelper_ConvertTicksToMS(uint32_t ticks);
uint32_t RTOSHelper_ConvertMSToTicks(uint32_t ms);


#ifdef __cplusplus
}
#endif

#endif /* RTOS_HELPER_H_ */

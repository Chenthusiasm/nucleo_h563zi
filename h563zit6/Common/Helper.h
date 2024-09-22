/**
 * @file    Helper.h
 * @brief   Interface file for the general helper and utility functions.
 * @section Legal Disclaimer
 *      ©2024 Whisker, All rights reserved. All contents of this source file and/or any other
 *      related source files are the explicit property of Whisker. Do not distribute. Do not copy.
 */
#ifndef HELPER_H_
#define HELPER_H_

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

uint32_t UIntRoundingDivide(uint32_t dividend, uint32_t divisor);
uint32_t UIntCeilingDivide(uint32_t dividend, uint32_t divisor);


#ifdef __cplusplus
}
#endif

#endif /* HELPER_H_ */

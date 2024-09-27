/**
 * @file    ICACHE_Support.h
 * @brief   Interface file for the ICACHE (instruction cache) support.
 * @note    The driver is reentrant from the defined structures; the driver may not be reentrant
 *          if different instances of the structures are constructed using the same HW peripherals,
 *          specifically the ICACHE peripheral.
 * @note    The driver is implemented as a singleton.
 * @note    The driver's purpose is to address issues where STM32H5 MCUs can hard fault when
 *          accessing certain RO (read-only) and OTP (one-time-programmable) areas of memory.
 *          https://community.st.com/t5/stm32-mcus/how-to-avoid-a-hardfault-when-icache-is-enabled-on-the-stm32h5/ta-p/630085
 * @note    In order to use the ICACHE_Support driver, disable the STM32CubeMX auto-generation of
 *          the ICACHE code.
 * @section Legal Disclaimer
 *      ©2024 Whisker, All rights reserved. All contents of this source file and/or any other
 *      related source files are the explicit property of Whisker. Do not distribute. Do not copy.
 */
#ifndef ICACHE_SUPPORT_H_
#define ICACHE_SUPPORT_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------------------------- */

#include <stdbool.h>


/* External typedef ------------------------------------------------------------------------------*/


/* Defines ---------------------------------------------------------------------------------------*/


/* External macro --------------------------------------------------------------------------------*/


/* External variables ----------------------------------------------------------------------------*/


/* External function prototypes ------------------------------------------------------------------*/


/* External functions ----------------------------------------------------------------------------*/

bool ICACHE_Init(void);


#ifdef __cplusplus
}
#endif

#endif /* ICACHE_SUPPORT_H_ */

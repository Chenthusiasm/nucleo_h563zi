/**
 * @file    Mutex.h
 * @brief   Interface file for the RTOS mutex (mutual exclusion).
 * @section Legal Disclaimer
 *      ©2024 Whisker, All rights reserved. All contents of this source file and/or any other
 *      related source files are the explicit property of Whisker. Do not distribute. Do not copy.
 */
#ifndef MUTEX_H_
#define MUTEX_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------------------------- */

#include <stdbool.h>
#include <stdint.h>

#include "stm32h5xx_hal.h"
#if defined(__has_include)
    #if __has_include("cmsis_os2.h")
        #define MUTEX_H_INCLUDE_CMSIS_OS2
        #include "cmsis_os2.h"
    #endif
#endif


/* External typedef ------------------------------------------------------------------------------*/

/**
 * @struct Mutex
 * @brief   Type definition of a structure that aggregates key components needed for the mutex to
 *          operate.
 * @var Mutex.acquired  Flag indicating if the mutex is acquired. This flag is only used if no
 *                      RTOS is enabled so there's no RTOS mutex available for use. This is a
 *                      required parameter because standard C doesn't allow for empty struct
 *                      definitions.
 * @var Mutex.mutexID   The ID of the mutex. Only available if CMSIS RTOS2 is enabled.
 */
typedef struct {
    bool acquired;
#if defined(MUTEX_H_INCLUDE_CMSIS_OS2)
    osMutexId_t mutexID;
#endif /* defined(MUTEX_H_INCLUDE_CMSIS_OS2) */
} Mutex;


/* Defines ---------------------------------------------------------------------------------------*/



/* External macro --------------------------------------------------------------------------------*/


/* External variables ----------------------------------------------------------------------------*/


/* External function prototypes ------------------------------------------------------------------*/


/* External functions ----------------------------------------------------------------------------*/

Mutex Mutex_ctor(
#if defined(MUTEX_H_INCLUDE_CMSIS_OS2)
                   osMutexId_t mutexID
#else
                   void
#endif /* defined(MUTEX_H_INCLUDE_CMSIS_OS2) */
);
bool Mutex_Init(Mutex *const self);
bool Mutex_Acquire(Mutex *const self, uint32_t timeout_ms);
bool Mutex_Release(Mutex *const self);


#ifdef __cplusplus
}
#endif

#endif /* MUTEX_H_ */

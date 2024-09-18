/**
 * @file    Mutex.c
 * @brief   Implementation of the RTOS mutex (mutual exclusion).
 * @section Legal Disclaimer
 *      ©2024 Whisker, All rights reserved. All contents of this source file and/or any other
 *      related source files are the explicit property of Whisker. Do not distribute. Do not copy.
 */

/* Includes --------------------------------------------------------------------------------------*/

#include <assert.h>

#include "Mutex.h"
#include "RTOSHelper.h"


/* Internal typedef ------------------------------------------------------------------------------*/


/* Internal define -------------------------------------------------------------------------------*/


/* Internal macro --------------------------------------------------------------------------------*/


/* Internal variables ----------------------------------------------------------------------------*/


/* Internal constants ----------------------------------------------------------------------------*/


/* Internal function prototypes ------------------------------------------------------------------*/


/* Internal functions ----------------------------------------------------------------------------*/


/* External function prototypes ------------------------------------------------------------------*/


/* External functions ----------------------------------------------------------------------------*/

/**
 * @brief   Constructor that initializes the parameters of the Mutex struct.
 * @param[in]   mutexId ID of the mutex; can be NULL to indicate no mutex to protect the
 *                      resource.
 * @return  The new Mutex struct (to be copied upon assignment).
 */
Mutex Mutex_ctor(
#if defined(MUTEX_H_INCLUDE_CMSIS_OS2)
                   osMutexId_t mutexID
#else
                   void
#endif /* defined(MUTEX_H_INCLUDE_CMSIS_OS2) */
) {
    Mutex self = {
        .acquired = false
#if defined(MUTEX_H_INCLUDE_CMSIS_OS2)
        ,
        .mutexID = mutexID
#endif /* defined(MUTEX_H_INCLUDE_CMSIS_OS2) */
    };

    return self;
}


/**
 * @brief   Initializes the mutex instance.
 * @param[in]   self        Pointer to the Mutex struct that represents the mutex instance.
 * @return  If the mutex was successfully initialized, true; otherwise, false.
 */
bool Mutex_Init(Mutex *const self) {
    assert(self != NULL);
    self->acquired = false;
    return true;
}


/**
 * @brief   Acquire the mutex to protect the resource it's associated with.
 * @param[in]   self        Pointer to the Mutex struct that represents the mutex instance.
 * @param[in]   timeout_ms  The timeout period in milliseconds to acquire the mutex. If 0, then no
 *                          timeout.
 * @return  If the mutex was successfully acquired, true; otherwise, false. If there's no RTOS mutex
 *          support, return true.
 */
bool Mutex_Acquire(Mutex *const self, uint32_t timeout_ms) {
    assert(self != NULL);

#if defined(MUTEX_H_INCLUDE_CMSIS_OS2)
    if (self->mutexID == NULL) {
        return true;
    }
    osStatus_t status = osMutexAcquire(self->mutexID, RTOSHelper_ConvertMSToTicks(timeout_ms));
    if (status != osOK) {
        return false;
    }
#else
    if (self->acquired == true) {
        return false;
    }
    self->acquired = true;
#endif /* defined(MUTEX_H_INCLUDE_CMSIS_OS2) */
    return true;
}


/**
 * @brief   Release the mutex so that the resource it's protecting is available..
 * @param[in]   self    Pointer to the Mutex struct that represents the mutex instance.
 * @note    It's possible that two different Mutex instances consist of the same RTOS mutex handle.
 *          In such cases, it's possible that acquiring one mutex but releasing the other mutex will
 *          lead to the mutex being successfully released since they both use the same mutex handle.
 * @return  If the mutex was successfully released, true; otherwise, false. If there's no mutex,
 *          return true.
 */
bool Mutex_Release(Mutex *const self) {
    assert(self != NULL);

#if defined(MUTEX_H_INCLUDE_CMSIS_OS2)
    if (self->mutexID == NULL) {
        return true;
    }
    osStatus_t status = osMutexRelease(self->mutexID);
    if (status != osOK) {
        return false;
    }
#else
    if (self->acquired == false) {
        return false;
    }
    self->acquired = false;
#endif /* defined(MUTEX_H_INCLUDE_CMSIS_OS2) */
    return true;
}

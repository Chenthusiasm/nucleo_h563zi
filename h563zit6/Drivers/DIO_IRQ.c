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
#include "Mutex.h"
#if defined(__has_include)
    #if __has_include("FreeRTOS.h")
        #define INCLUDE_FREE_RTOS
        #include "FreeRTOS.h"
    #endif
#endif


/* Internal typedef ------------------------------------------------------------------------------*/

/**
 * @struct  EXTIConfig_t
 * @brief   Configuration of the EXTI.
 * @var EXTIConfig_t::callback  The callback function associated with the EXTI.
 * @var EXTIConfig_t::enable    Flag indicating if the callback is enabled.
 */
typedef struct {
    DIO_EXTICallback_t callback;
    bool enable;
} EXTIConfig_t;


/* The number of external interrupts/events. */
#define NUM_EXTI                        (16u)


/**
 * @struct  DIO_IRQ
 * @brief   Type definition of a structure that aggregates key components needed for the DIO
 *          interrupt driver to function.
 * @var DIO_IRQ::configs[]      Array of EXTIConfig_t configuration for each unique EXTI.
 * @var DIO_IRQ::mutex          Mutex to protect access to the instance.
 * @var DIO_IRQ::initialized    Flag indicating if the instance has been initialized.
 */
typedef struct {
    EXTIConfig_t configs[NUM_EXTI];
    Mutex mutex;
    bool initialized;
} DIO_IRQ;


/* Internal define -------------------------------------------------------------------------------*/

#if (defined(MUTEX_H_INCLUDE_CMSIS_OS2) && defined(INCLUDE_FREE_RTOS))
#define DIO_IRQ_MUTEX
#endif /* defined(MUTEX_H_INCLUDE_CMSIS_OS2) && defined(INCLUDE_FREE_RTOS) */

/* Default timeout value in milliseconds to acquire the mutex. */
#define MUTEX_TIMEOUT_MS                (5u)


/* Internal macro --------------------------------------------------------------------------------*/


/* Internal variables ----------------------------------------------------------------------------*/

/* Instance of the DIO_IRQ driver. Initialized to 0: callbacks are NULL, GPIO port handles are
 * NULL, callbacks are disabled, empty Mutex, and not initialized. */
static DIO_IRQ self = { 0 };

#if defined(DIO_IRQ_MUTEX)
/* Mutex handle. */
static osMutexId_t mutexHandle;

/* Semaphore control block (the mutex is based off of a semaphore) */
static StaticSemaphore_t mutexCB;
#endif /* defined(DIO_IRQ_MUTEX) */


/* Internal constants ----------------------------------------------------------------------------*/

#if defined(DIO_IRQ_MUTEX)
static const osMutexAttr_t mutex_attributes = {
  .name = "DIO_IRQ_mutex",
  .cb_mem = &mutexCB,
  .cb_size = sizeof(mutexCB),
};
#endif /* defined(DIO_IRQ_MUTEX) */


/* Internal function prototypes ------------------------------------------------------------------*/


/* Internal functions ----------------------------------------------------------------------------*/

/**
 * @brief   Handle the the EXTI callback.
 * @param[in]   self        Pointer to the DIO_IRQ struct that represents the DIO IRQ instance.
 * @param[in]   pinMask     Bit mask of the GPIO pin number (where 0x00 = pin 0)
 * @param[in]   transition  The DIO_Transition_t transition.
 */
static void handleEXTICallback(DIO_IRQ const *const self, uint16_t pinMask,
                               DIO_Transition_t transition) {
    uint8_t pin = 0u;
    switch (pinMask) {
    case GPIO_PIN_0 : pin =  0u; break;
    case GPIO_PIN_1 : pin =  1u; break;
    case GPIO_PIN_2 : pin =  2u; break;
    case GPIO_PIN_3 : pin =  3u; break;
    case GPIO_PIN_4 : pin =  4u; break;
    case GPIO_PIN_5 : pin =  5u; break;
    case GPIO_PIN_6 : pin =  6u; break;
    case GPIO_PIN_7 : pin =  7u; break;
    case GPIO_PIN_8 : pin =  8u; break;
    case GPIO_PIN_9 : pin =  9u; break;
    case GPIO_PIN_10: pin = 10u; break;
    case GPIO_PIN_11: pin = 11u; break;
    case GPIO_PIN_12: pin = 12u; break;
    case GPIO_PIN_13: pin = 13u; break;
    case GPIO_PIN_14: pin = 14u; break;
    case GPIO_PIN_15: pin = 15u; break;
    default:
        return;
    }
    if ((self->configs[pin].enable == true) && (self->configs[pin].callback != NULL)) {
        self->configs[pin].callback(transition);
    }
}


/* External function prototypes ------------------------------------------------------------------*/


/* External functions ----------------------------------------------------------------------------*/

/**
 * @brief   Initializes the DIO_IRQ singleton.
 * @note    Recommended to invoke this function before the different tasks/threads and task
 *          scheduler starts.
 * @return  The specific DIO_IRQ_Err_t which indicates the specific error code if the function
 *          couldn't be executed successfully. If the function executes successfully,
 *          DIO_IRQ_ERR_NONE.
 */
DIO_IRQ_Err_t DIO_IRQ_Init(void) {
    if (self.initialized == true) {
        return DIO_IRQ_ERR_NONE;
    }
#if defined(DIO_IRQ_MUTEX)
    mutexHandle = osMutexNew(&mutex_attributes);
    self.mutex = Mutex_ctor(mutexHandle);
#else
    self.mutex = Mutex_ctor();
#endif /* defined(DIO_IRQ_MUTEX) */
    Mutex_Init(&self.mutex);
    self.initialized = true;
    return DIO_IRQ_ERR_NONE;
}


/**
 * @brief   Register a DIO driver instance with the interrupt.
 * @param[in]   pin         The pin number (not the GPIO pin mask defined by the HAL).
 * @param[in]   callback    The external interrupt/event callback function to invoke when the
 *                          configured transition is externally triggered.
 * @return  The specific DIO_IRQ_Err_t which indicates the specific error code if the function
 *          couldn't be executed successfully. If the function executes successfully,
 *          DIO_IRQ_ERR_NONE.
 */
DIO_IRQ_Err_t DIO_IRQ_Register(uint8_t pin, DIO_EXTICallback_t callback) {
    if (self.initialized == false) {
        return DIO_IRQ_ERR_UNINITIALIZED;
    }
    if (pin >= NUM_EXTI) {
        return DIO_IRQ_ERR_INVALID_PARAM;
    }
    if (Mutex_Acquire(&self.mutex, MUTEX_TIMEOUT_MS) == false) {
        return DIO_IRQ_ERR_RESOURCE_BLOCKED;
    }
    if (self.configs[pin].callback != NULL) {
        Mutex_Release(&self.mutex);
        return DIO_IRQ_ERR_REGISTERED;
    }
    self.configs[pin] = (EXTIConfig_t){
        .callback = callback,
        .enable = true,
    };
    Mutex_Release(&self.mutex);
    return DIO_IRQ_ERR_NONE;
}


/**
 * @brief   Enable/disable the interrupt for the specific pin.
 * @param[in]   pin     The pin number (not the GPIO pin mask defined by the HAL).
 * @param[in]   enable  Flag; true to enable, false to disable.
 * @return  The specific DIO_IRQ_Err_t which indicates the specific error code if the function
 *          couldn't be executed successfully. If the function executes successfully,
 *          DIO_IRQ_ERR_NONE.
 */
DIO_IRQ_Err_t DIO_IRQ_Enable(uint8_t pin, bool enable) {
    if (pin >= NUM_EXTI) {
        return DIO_IRQ_ERR_INVALID_PARAM;
    }
    if (Mutex_Acquire(&self.mutex, MUTEX_TIMEOUT_MS) == false) {
        return DIO_IRQ_ERR_RESOURCE_BLOCKED;
    }
    if (self.configs[pin].callback == NULL) {
        Mutex_Release(&self.mutex);
        return DIO_IRQ_ERR_UNREGISTERED;
    }
    self.configs[pin].enable = enable;
    Mutex_Release(&self.mutex);
    return DIO_IRQ_ERR_NONE;
}


/**
 * @brief   Check if the interrupt is enabled for the specified pin.
 * @param[in]   pin     The pin number (not the GPIO pin mask defined by the HAL).
 * @return  If the interrupt is enabled for the specified pin, true; otherwise false.
 */
bool DIO_IRQ_IsEnabled(uint8_t pin) {
    if (pin >= NUM_EXTI) {
        return false;
    }
    if (Mutex_Acquire(&self.mutex, MUTEX_TIMEOUT_MS) == false) {
        return DIO_IRQ_ERR_RESOURCE_BLOCKED;
    }
    bool enabled = self.configs[pin].enable;
    Mutex_Release(&self.mutex);
    return enabled;
}


/**
 * @brief EXTI rising edge callback function.
 * The EXTI rising edge callback function. The STM32 HAL provides a weak implementation so this
 * implementation handles an extended interrupt based on the GPIO pin.
 * @note    STM32CubeMX will auto-generate the corresponding EXTIx_IRQHandler functions in
 *          "Core/Src/stm32h5xx_it.c". EXTIx_IRQHandler invokes the GPIO HAL function
 *          HAL_GPIO_EXTI_IRQHandler with the corresponding pin mask. HAL_GPIO_EXTI_IRQHandler
 *          invokes this function, HAL_GPIO_EXTI_Rising_Callback when the corresponding pin
 *          transitions from low to high (rising).
 * @param   GPIO_Pin    Bit mask of the GPIO pin number (where 0x00 = pin 0)
 */
void HAL_GPIO_EXTI_Rising_Callback(uint16_t GPIO_Pin) {
    handleEXTICallback(&self, GPIO_Pin, DIO_TRANSITION_RISING_EDGE);
}


/**
 * @brief EXTI falling edge callback function.
 * The EXTI falling edge callback function. The STM32 HAL provides a weak implementation so this
 * implementation handles an extended interrupt based on the GPIO pin.
 * @note    STM32CubeMX will auto-generate the corresponding EXTIx_IRQHandler functions in
 *          "Core/Src/stm32h5xx_it.c". EXTIx_IRQHandler invokes the GPIO HAL function
 *          HAL_GPIO_EXTI_IRQHandler with the corresponding pin mask. HAL_GPIO_EXTI_IRQHandler
 *          invokes this function, HAL_GPIO_EXTI_Falling_Callback when the corresponding pin
 *          transitions from high to low (falling).
 * @param   GPIO_Pin    Bit mask of the GPIO pin number (where 0x00 = pin 0)
 */
void HAL_GPIO_EXTI_Falling_Callback(uint16_t GPIO_Pin) {
    handleEXTICallback(&self, GPIO_Pin, DIO_TRANSITION_FALLING_EDGE);
}

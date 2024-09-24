/**
 * @file    PWM.c
 * @brief   Interface file for the PWM driver.
 * @note    The driver is reentrant from the defined structures; the driver may not be reentrant
 *          if different instances of the structures are constructed using the same HW peripherals,
 *          specifically timers (TIM). To ensure reentrancy, the Mutex module must be configured
 *          to use an RTOS mutex.
 * @section Legal Disclaimer
 *      ©2024 Whisker, All rights reserved. All contents of this source file and/or any other
 *      related source files are the explicit property of Whisker. Do not distribute. Do not copy.
 */

/* Includes --------------------------------------------------------------------------------------*/

#include <assert.h>
#include <stdint.h>

#include "Helper.h"
#include "PWM.h"


/* Internal typedef ------------------------------------------------------------------------------*/

/**
 * @enum    State_t
 * @brief   Different states of the PWM driver.
 */
typedef enum {
    STATE_UNINITIALIZED = 0u,   /*!< The PWM driver hasn't been initialized */
    STATE_STOPPED,              /*!< The PWM driver has stopped and is not generating PWM output */
    STATE_STARTED               /*!< The PWM driver has started and is generating PWM output */
} State_t;


/**
 * @struct  FrequencyRegisters_t
 * @brief   Aggregates the TIM register values needed to configure the PWM switching frequency.
 * @var FrequencyRegisters_t::timRegARR The value of the TIM.ARR (auto reload register).
 * @var FrequencyRegisters_t::timRegPSC The value of the TIM.PSC (prescaler register).
 */
typedef struct {
    uint16_t timRegARR;
    uint16_t timRegPSC;
} FrequencyRegisters_t;


/* Internal define -------------------------------------------------------------------------------*/

/* Default timeout value in milliseconds to acquire the HW TIM mutex. */
#define TIM_MUTEX_TIMEOUT_MS            (5u)

/* The minimum value for the duty cycle in units of tenth of a percent (0.1%). */
#define DUTY_CYCLE_MIN_TENTH_PCT        (0u)

/* The maximum value for the duty cycle in units of tenth of a percent (0.1%). */
#define DUTY_CYCLE_MAX_TENTH_PCT        (1000u)

/* The maximum value for the TIM.ARR register. Some timers may use a 32-bit counter allowing for a
 * greater max value, but a 16-bit max value is used since the smallest counter is 16-bit.
 */
#define TIM_REG_ARR_MAX                 (UINT16_MAX)


/* Internal macro --------------------------------------------------------------------------------*/


/* Internal variables ----------------------------------------------------------------------------*/


/* Internal constants ----------------------------------------------------------------------------*/


/* Internal function prototypes ------------------------------------------------------------------*/


/* Internal functions ----------------------------------------------------------------------------*/

/**
 * @brief   Checks the duty cycle and limits it to the range of 0.0 - 100.0%.
 * @param[in]   dutyCycle_tenthPct  The duty cycle to limit; in units of tenth of a percent (0.1%).
 * @return  The limited duty cycle that falls in the range of 0.0 - 100.0%.
 */
static uint16_t limitDutyCycle_tenthPct(uint16_t dutyCycle_tenthPct) {
    if (dutyCycle_tenthPct > DUTY_CYCLE_MAX_TENTH_PCT) {
        dutyCycle_tenthPct = DUTY_CYCLE_MAX_TENTH_PCT;
    }
    return dutyCycle_tenthPct;
}


/**
 * @brief   Calculates the TIM.PSC and TIM.ARR register values to achieve a specific PWM switching
 *          frequency.
 * Given a source frequency this function calculates the TIM.PSC (prescaler) and TIM.ARR (auto
 * reload) register values to divide the source clock to achieve the given PWM switching frequency.
 * @note    This algorithm assumes the max value of TIM.ARR is 16 bits.
 * @note    Refer to: https://electronics.stackexchange.com/questions/176922/setting-the-frequency-of-a-pwm-on-a-stm32
 * @param[out]  registers               The FrequencyRegisters_t (TIM.ARR and TIM.PSC) return.
 * @param[in]   switchingFrequency_hz   The target PWM switching frequency in Hz.
 * @param[in]   sourceFrequency_hz      The source clock frequency in Hz.
 * @param[in]   maxOverflow             Max overflow value (number of ticks the TIM counter counts
 *                                      in a single PWM cycle).
 * @return  The specific PWM_Err_t which indicates the specific error code if the function couldn't
 *          be executed successfully. If the function executes successfully, PWM_ERR_NONE.
 */
static PWM_Err_t calculateFrequencyRegisters(FrequencyRegisters_t *const registers,
                                             uint32_t switchingFrequency_hz,
                                             uint32_t sourceFrequency_hz,
                                             uint16_t maxOverflow) {
    if (registers == NULL) {
        return PWM_ERR_NULL_PARAM;
    }
    if ((switchingFrequency_hz == 0u) ||
        (sourceFrequency_hz == 0u) ||
        (maxOverflow == 0u)) {
        return PWM_ERR_INVALID_PARAM;
    }
    uint32_t cycles = sourceFrequency_hz / switchingFrequency_hz;
    uint16_t overflow = maxOverflow;
    if (cycles < overflow) {
        overflow = (uint16_t)cycles;
    }
    if (overflow < DUTY_CYCLE_MAX_TENTH_PCT) {
        return PWM_ERR_INVALID_PARAM;
    }
    uint16_t prescaler = (uint16_t)UIntCeilingDivide(cycles, overflow);
    overflow = (uint16_t)UIntRoundingDivide(cycles, prescaler);
    registers->timRegPSC = prescaler - 1u;
    registers->timRegARR = overflow;
    return PWM_ERR_NONE;
}


/**
 * @brief   Calculates the TIM.CCRx register value to achieve the specific duty cycle.
 * @param[in]   dutyCycle_tenthPct  The target duty cycle; in units of tenth of a percent (0.1%).
 * @param[in]   timRegARR           The value of the TIM.ARR (auto reload register).
 * @note    Assumed that dutyCycle_tenthPct has been limited to the max value prior to being passed
 *          into this function so no error checking is done on its value.
 * @note    Assumed that (timRegARR <= UINT16_MAX); otherwise overflows can occur.
 * @return  The value for the TIM.CCRx register to achieve a specific duty cycle.
 */
static uint32_t calculateCCRx(uint16_t dutyCycle_tenthPct, uint32_t timRegARR) {
    if (timRegARR == 0u) {
        return timRegARR;
    }
    return UIntRoundingDivide(dutyCycle_tenthPct * timRegARR, DUTY_CYCLE_MAX_TENTH_PCT);
}


/**
 * @brief   Calculates the duty cycle in units of tenth of a percent (0.1%) given the values of
 *          the TIM.ARR and TIM.CCRx registers.
 * @param[in]   timRegARR   The value of the TIM.ARR (auto reload register).
 * @param[in]   timRegCCRx  The value of the TIM.CCRx (capture compare register channel x).
 * @note    Assumed that (timRegARR <= UINT16_MAX) and (timRegCCRx <= timRegARR); otherwise
 *          overflows can occur.
 * @return  The duty cycle in units of tenth of a percent (0.1%).
 */
static uint16_t calculateDutyCycle_tenthPct(uint32_t timRegARR, uint32_t timRegCCRx) {
    if (timRegARR == 0u) {
        return timRegARR;
    }
    return UIntRoundingDivide(timRegCCRx * DUTY_CYCLE_MAX_TENTH_PCT, timRegARR);
}


/**
 * @brief   Calculates the PWM switching frequency in Hz given the source clock frequency and the
 *          timer prescaler and overflow values.
 * @param[in]   sourceFrequency_hz  The source clock frequency in Hz.
 * @param[in]   prescaler           The timer prescaler value (TIM.PSC + 1).
 * @param[in]   overflow            The timer overflow value (TIM.ARR).
 * @return  The PWM switching frequency in Hz.
 */
static uint32_t calculateSwitchingFrequency_hz(uint32_t sourceFrequency_hz, uint32_t prescaler,
                                               uint16_t overflow) {
    return UIntRoundingDivide(sourceFrequency_hz, prescaler * overflow);
}


/* External function prototypes ------------------------------------------------------------------*/


/* External functions ----------------------------------------------------------------------------*/

/**
 * @brief   Constructor that initializes the parameters of the PWM struct.
 * @param[in]   timerPtr    Pointer to the Timer driver.
 * @param[in]   channel     Timer channel number the generated PWM is output.
 * @note    All PWM driver struct instances must be constructed using this constructor since it
 *          verifies that the PWM struct parameters are valid.
 * @return  The new PWM struct (to be copied upon assignment).
 */
PWM PWM_ctor(Timer *const timerPtr, Timer_Channel_t channel) {
    assert(timerPtr != NULL);
    // do not set the mode in an assert; this must always be called
    Timer_Err_t err = Timer_SetModePWM(timerPtr, channel);
    assert(err == TIMER_ERR_NONE);

    PWM self = {
        .timerPtr = timerPtr,
        .channelMask = Timer_GetChannelMask(channel),
        .state = STATE_UNINITIALIZED
    };

    return self;
}


/**
 * @brief   Initializes the PWM instance.
 * @param[in]   self        Pointer to the PWM struct that represents the PWM instance.
 * @param[in]   switchingFrequency_hz   The PWM switching frequency (the frequency of a single PWM
 *                                      cycle) in units of Hz.
 * @param[in]   dutyCycle_tenthPct      The PWM duty cycle in units of a tenth of a percent (0.1%).
 *                                      The valid range is 0 - 1000 (0.0 - 100.0%); any value
 *                                      greater than 1000 will automatically rail to 1000 (100.0%).
 * @return  The specific PWM_Err_t which indicates the specific error code if the function couldn't
 *          be executed successfully. If the function executes successfully, PWM_ERR_NONE.
 */
PWM_Err_t PWM_Init(PWM *const self, uint32_t switchingFrequency_hz, uint16_t dutyCycle_tenthPct) {
    assert(self != NULL);

    // preliminary calculations
    FrequencyRegisters_t registers;
    PWM_Err_t err = calculateFrequencyRegisters(&registers, switchingFrequency_hz,
                                                Timer_GetClockFrequency_hz(self->timerPtr),
                                                TIM_REG_ARR_MAX);
    if (err != PWM_ERR_NONE) {
        return err;
    }
    dutyCycle_tenthPct = limitDutyCycle_tenthPct(dutyCycle_tenthPct);
    uint32_t timRegCCRx = calculateCCRx(dutyCycle_tenthPct, registers.timRegARR);
    TIM_HandleTypeDef *const timHandle = Timer_GetTIMHandle(self->timerPtr);

    // modify the TIM peripheral registers
    if (Timer_AcquireMutex(self->timerPtr, TIM_MUTEX_TIMEOUT_MS) == false) {
        return PWM_ERR_RESOURCE_BLOCKED;
    }
    __HAL_TIM_SET_PRESCALER(timHandle, registers.timRegPSC);
    __HAL_TIM_SET_AUTORELOAD(timHandle, registers.timRegARR);
    __HAL_TIM_SET_COMPARE(timHandle, self->channelMask, timRegCCRx);
    Timer_ReleaseMutex(self->timerPtr);

    if (self->state == STATE_UNINITIALIZED) {
        self->state = STATE_STOPPED;
    }
    return PWM_ERR_NONE;
}


/**
 * @brief   Starts the PWM output generation.
 * @param[in]   self    Pointer to the PWM struct that represents the PWM instance.
 * @return  The specific PWM_Err_t which indicates the specific error code if the function couldn't
 *          be executed successfully. If the function executes successfully, PWM_ERR_NONE.
 */
PWM_Err_t PWM_Start(PWM *const self) {
    assert(self != NULL);

    if (self->state == STATE_UNINITIALIZED) {
        return PWM_ERR_UNINITIALIZED;
    }
    if (self->state == STATE_STARTED) {
        return PWM_ERR_STARTED;
    }
    if (Timer_AcquireMutex(self->timerPtr, TIM_MUTEX_TIMEOUT_MS) == false) {
        return PWM_ERR_RESOURCE_BLOCKED;
    }
    if (HAL_TIM_PWM_Start(Timer_GetTIMHandle(self->timerPtr), self->channelMask) != HAL_OK) {
        Timer_ReleaseMutex(self->timerPtr);
        return PWM_ERR_HAL;
    }
    Timer_ReleaseMutex(self->timerPtr);
    self->state = STATE_STARTED;
    return PWM_ERR_NONE;
}


/**
 * @brief   Stops the PWM output generation.
 * @param[in]   self    Pointer to the PWM struct that represents the PWM instance.
 * @return  The specific PWM_Err_t which indicates the specific error code if the function couldn't
 *          be executed successfully. If the function executes successfully, PWM_ERR_NONE.
 */
PWM_Err_t PWM_Stop(PWM *const self) {
    assert(self != NULL);

    if (self->state == STATE_UNINITIALIZED) {
        return PWM_ERR_UNINITIALIZED;
    }
    if (self->state == STATE_STARTED) {
        return PWM_ERR_STOPPED;
    }
    if (Timer_AcquireMutex(self->timerPtr, TIM_MUTEX_TIMEOUT_MS) == false) {
        return PWM_ERR_RESOURCE_BLOCKED;
    }
    if (HAL_TIM_PWM_Stop(Timer_GetTIMHandle(self->timerPtr), self->channelMask) != HAL_OK) {
        Timer_ReleaseMutex(self->timerPtr);
        return PWM_ERR_HAL;
    }
    Timer_ReleaseMutex(self->timerPtr);
    self->state = STATE_STOPPED;
    return PWM_ERR_NONE;
}


/**
 * @brief   Accessor to get the calculated PWM switching frequency (the frequency of a single PWM
 *          cycle).
 * @param[in]   self    Pointer to the PWM struct that represents the PWM instance.
 * @return  The calculated PWM switching frequency.
 */
uint32_t PWM_GetSwitchingFrequency_hz(PWM const *const self) {
    assert(self != NULL);

    if (self->state == STATE_UNINITIALIZED) {
        return 0u;
    }
    TIM_HandleTypeDef *const timHandle = Timer_GetTIMHandle(self->timerPtr);
    if (Timer_AcquireMutex(self->timerPtr, TIM_MUTEX_TIMEOUT_MS) == false) {
        return 0u;
    }
    uint32_t prescaler = Timer_GetPrescaler(self->timerPtr);
    uint32_t overflow = __HAL_TIM_GET_AUTORELOAD(timHandle);
    Timer_ReleaseMutex(self->timerPtr);
    return calculateSwitchingFrequency_hz(Timer_GetClockFrequency_hz(self->timerPtr), prescaler,
                                          overflow);
}


/**
 * @brief   Accessor to get the calculated PWM duty cycle in units of tenth of a percent (0.1%).
 * @param[in]   self    Pointer to the PWM struct that represents the PWM instance.
 * @return  The calculated PWM duty cycle in tenth of a percent (0.1%).
 */
uint16_t PWM_GetDutyCycle_tenthPct(PWM const *const self) {
    assert(self != NULL);

    if (self->state == STATE_UNINITIALIZED) {
        return 0u;
    }
    TIM_HandleTypeDef *const timHandle = Timer_GetTIMHandle(self->timerPtr);
    if (Timer_AcquireMutex(self->timerPtr, TIM_MUTEX_TIMEOUT_MS) == false) {
        return 0u;
    }
    uint32_t timRegARR = __HAL_TIM_GET_AUTORELOAD(timHandle);
    uint32_t timRegCCRx = __HAL_TIM_GET_COMPARE(timHandle, self->channelMask);
    Timer_ReleaseMutex(self->timerPtr);
    return calculateDutyCycle_tenthPct(timRegARR, timRegCCRx);
}


/**
 * @brief   Accessor to set the PWM duty cycle in units of tenth of a percent (0.1%).
 * @param[in]   self                Pointer to the PWM struct that represents the PWM instance.
 * @param[in]   dutyCycle_tenthPct  The duty cycle to set in units of tenth of a percent (0.1%).
 * @note    The function will attempt to set the duty cycle as close as possible to the desired
 *          value, but this is highly dependent on the TIM peripheral input clock and the PWM
 *          switching frequency.
 * @return  The specific PWM_Err_t which indicates the specific error code if the function couldn't
 *          be executed successfully. If the function executes successfully, PWM_ERR_NONE.
 */
PWM_Err_t PWM_SetDutyCycle(PWM const *const self, uint16_t dutyCycle_tenthPct) {
    assert(self != NULL);

    if (self->state == STATE_UNINITIALIZED) {
        return PWM_ERR_UNINITIALIZED;
    }

    dutyCycle_tenthPct = limitDutyCycle_tenthPct(dutyCycle_tenthPct);
    uint32_t timRegCCR = 0u;
    if (Timer_AcquireMutex(self->timerPtr, TIM_MUTEX_TIMEOUT_MS) == false) {
        return PWM_ERR_RESOURCE_BLOCKED;
    }
    if (dutyCycle_tenthPct > DUTY_CYCLE_MIN_TENTH_PCT) {
        uint32_t timRegARR = __HAL_TIM_GET_AUTORELOAD(self->timerPtr->timHandle);
        timRegCCR = timRegARR;
        if (dutyCycle_tenthPct < DUTY_CYCLE_MAX_TENTH_PCT) {
            timRegCCR = calculateCCRx(dutyCycle_tenthPct, timRegARR);
        }
    }
    __HAL_TIM_SET_COMPARE(Timer_GetTIMHandle(self->timerPtr), self->channelMask, timRegCCR);
    Timer_ReleaseMutex(self->timerPtr);
    return PWM_ERR_NONE;
}


/**
 * @brief   Accessor to set the generated PWM output to high, so 100.0% duty cycle.
 * @param[in]   self    Pointer to the PWM struct that represents the PWM instance.
 * @return  The specific PWM_Err_t which indicates the specific error code if the function couldn't
 *          be executed successfully. If the function executes successfully, PWM_ERR_NONE.
 */
PWM_Err_t PWM_SetHigh(PWM const *const self) {
    assert(self != NULL);

    // Mutex is not acquired or released; PWM_SetDutyCycle will handle the Mutex
    return PWM_SetDutyCycle(self, DUTY_CYCLE_MAX_TENTH_PCT);
}


/**
 * @brief   Accessor to set the generated PWM output to low, so 0.0% duty cycle.
 * @param[in]   self    Pointer to the PWM struct that represents the PWM instance.
 * @return  The specific PWM_Err_t which indicates the specific error code if the function couldn't
 *          be executed successfully. If the function executes successfully, PWM_ERR_NONE.
 */
PWM_Err_t PWM_SetLow(PWM const *const self) {
    assert(self != NULL);

    // Mutex is not acquired or released; PWM_SetDutyCycle will handle the Mutex
    return PWM_SetDutyCycle(self, DUTY_CYCLE_MIN_TENTH_PCT);
}

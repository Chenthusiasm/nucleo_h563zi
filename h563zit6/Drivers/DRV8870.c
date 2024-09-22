/**
 * @file    DRV8870.c
 * @brief   Implementation of the DRV8870 motor driver.
 * @section Legal Disclaimer
 *      ©2024 Whisker, All rights reserved. All contents of this source file and/or any other
 *      related source files are the explicit property of Whisker. Do not distribute. Do not copy.
 */

/* Includes --------------------------------------------------------------------------------------*/

#include <assert.h>
#include <stdint.h>

#include "DRV8870.h"
#include "PWM.h"


/* Internal typedef ------------------------------------------------------------------------------*/

/**
 * @enum    State_t
 * @brief   Different states of the PWM driver.
 */
typedef enum {
    STATE_UNINITIALIZED = 0u,   /*!< The motor driver hasn't been initialized */
    STATE_BRAKED,               /*!< The motor driver has stopped (braked) */
    STATE_DRIVING,              /*!< The motor driver is running (driving)*/
} State_t;


typedef struct {
    uint16_t in0_tenthPct;
    uint16_t in1_tenthPct;
} DutyCycles_t;


/* Internal define -------------------------------------------------------------------------------*/

/* The minimum value for the motor drive strength in units of tenth of a percent (0.1%). */
#define DRIVE_STRENGTH_MIN_TENTH_PCT    (0u)

/* The maximum value for the motor drive strength in units of tenth of a percent (0.1%). */
#define DRIVE_STRENGTH_MAX_TENTH_PCT    (1000u)

/* The PWM duty cycle for a stopped (braked) motor */
#define DUTY_CYCLE_STOPPED_TENTH_PCT    (1000u)


/* Internal macro --------------------------------------------------------------------------------*/


/* Internal variables ----------------------------------------------------------------------------*/


/* Internal constants ----------------------------------------------------------------------------*/


/* Internal function prototypes ------------------------------------------------------------------*/


/* Internal functions ----------------------------------------------------------------------------*/

/**
 * @brief   Checks the drive strength and limits it to the range of 0.0 - 100.0%.
 * @param[in]   driveStrength_tenthPct  The drive strength to limit; in units of tenth of a percent
 *              (0.1%).
 * @return  The limited drive strength that falls in the range of 0.0 - 100.0%.
 */
static uint16_t limitStrength_tenthPct(uint16_t strength_tenthPct) {
    if (strength_tenthPct > DRIVE_STRENGTH_MAX_TENTH_PCT) {
        strength_tenthPct = DRIVE_STRENGTH_MAX_TENTH_PCT;
    }
    return strength_tenthPct;
}


/**
 * @brief   Converts the drive strength to the PWM duty cycle needed to drive the motor at the
 *          specified drive strength.
 * @note    The drive strength and PWM duty cycle is inversely proportional.
 * @param[in]   strength_tenthPct
 */
static uint16_t convertStrengthToDutyCycle(uint16_t strength_tenthPct) {
    strength_tenthPct = limitStrength(strengthPct);
    return (DRIVE_STRENGTH_MAX_TENTH_PCT - strength_tenthPCT);
}


/**
 * @brief   Calculates the duty cycles for the IN0 and IN1 lines in order to drive the DRV8870 motor
 *          driver at the specified direction and strength.
 * @param[in]   direction           The direction to drive the motor in.
 * @param[in]   strength_tenthPct   The strength of the motor drive in units of tenth of a percent
 *                                  (0.1%). 0 = stopped, 1000 = full speed.
 * @return  The duty cycles as a DutyCycles_t struct.
 */
static DutyCycles_t calculateDutyCycles(DRV8870_Direction_t direction, uint16_t strength_tenthPct) {
    // stopped (braked)
    if ((direction == DRV8870_DIRECTION_STOPPED) ||
        (strength_tenthPct == DRIVE_STRENGTH_MIN_TENTH_PCT)) {
        DutyCycles_t dutyCycles = {
            .in0_tenthPct = DUTY_CYCLE_STOPPED_TENTH_PCT,
            .in1_tenthPct = DUTY_CYCLE_STOPPED_TENTH_PCT,
        };
        return dutyCycles;
    }
    // forward drive direction
    uint16_t dutyCycle = convertStrengthToDutyCycle(strength_tenthPct);
    if (direction == DRV8870_DIRECTION_FORWARD) {
        DutyCycles_t dutyCycles = {
            .in0_tenthPct = DUTY_CYCLE_STOPPED_TENTH_PCT,
            .in1_tenthPct = dutyCycle,
        };
        return dutyCycles;
    }
    // reverse drive direction
    DutyCycles_t dutyCycles = {
        .in0_tenthPct = dutyCycle,
        .in1_tenthPct = DUTY_CYCLE_STOPPED_TENTH_PCT,
    };
    return dutyCycles;
}


/* External function prototypes ------------------------------------------------------------------*/


/* External functions ----------------------------------------------------------------------------*/

/**
 * @brief   Constructor that initializes the parameters of the DRV8870 motor driver struct.
 * @param[in]   timerPtr    Pointer to the Timer driver used by the PWM channels.
 * @param[in]   channelIN0  Timer channel number for the PWM output for IN0.
 * @param[in]   channelIN1  Timer channel number for the PWM output for IN1.
 * @return  The new DRV8870 motor driver struct (to be copied upon assignment).
 */
DRV8870 DRV8870_ctor(Timer *const timerPtr,
                     Timer_Channel_t channelIN0, Timer_Channel_t channelIN1) {
    assert(timerPtr != NULL);

    DRV8870 self = {
        .pwmIN0 = PWM_ctor(timerPtr, channelIN0),
        .pwmIN1 = PWM_ctor(timerPtr, channelIN1),
        .state = STATE_UNINITIALIZED,
    };
    return self;
}


/**
 * @brief   Initializes the DRV8870 motor driver instance and puts it in the brake stop (motors not
 *          being driven).
 * @param[in]   self            Pointer to the DRV8870 motor driver struct that represents the motor
 *                              driver instance.
 * @param[in]   pwmFrequency_hz The PWM frequency in Hz.
 * @return  The specific DRV8870_Err_t which indicates the specific error code if the function
 *          couldn't be executed successfully. If the function executes successfully,
 *          DRV8870_ERR_NONE.
 */
DRV8870_Err_t DRV8870_Init(DRV8870 *const self, uint32_t pwmFrequency_hz) {
    assert(self != NULL);

    PWM_ERR_t err = PWM_Init(&self.pwmIN0, pwmFrequency_hz, 1000u)

    return PWM_ERR_NONE;
}


/**
 * @brief   Puts the DRV8870 motor driver in the drive state so that the connected motor will be
 *          driven in the specified direction at the specified drive strength.
 * @param[in]   self                Pointer to the DRV8870 motor driver struct that represents the
 *                                  motor driver instance.
 * @param[in]   direction           The DRV8870_Direction_t direction to drive the motor.
 * @param[in]   strength_tenthPct   The strength to drive the motor, in units of tenth of a percent
 *                                  (0.1%).
 * @return  The specific DRV8870_Err_t which indicates the specific error code if the function
 *          couldn't be executed successfully. If the function executes successfully,
 *          DRV8870_ERR_NONE.
 */
DRV8870_Err_t DRV8870_Drive(DRV8870 const *const self, DRV8870_Direction_t direction,
                   uint16_t strength_tenthPct) {
    assert(self != NULL);

    return PWM_ERR_NONE;
}


/**
 * @brief   Puts the DRV8870 motor driver in the brake state so that the connected motor will no
 *          longer be driven.
 * @param[in]   self    Pointer to the DRV8870 motor driver struct that represents the motor driver
 *                      instance.
 * @return  The specific DRV8870_Err_t which indicates the specific error code if the function
 *          couldn't be executed successfully. If the function executes successfully,
 *          DRV8870_ERR_NONE.
 */
DRV8870_Err_t DRV8870_Brake(DRV8870 const *const self) {
    assert(self != NULL);

    return PWM_ERR_NONE;
}


/**
 * @brief   Puts the DRV8870 motor driver in the coast state so that the connected motor will no
 *          longer be driven and also goes to sleep.
 * @param[in]   self    Pointer to the DRV8870 motor driver struct that represents the motor driver
 *                      instance.
 * @return  The specific DRV8870_Err_t which indicates the specific error code if the function
 *          couldn't be executed successfully. If the function executes successfully,
 *          DRV8870_ERR_NONE.
 */
DRV8870_Err_t DRV8870_Coast(DRV8870 const *const self) {
    assert(self != NULL);

    return PWM_ERR_NONE;
}

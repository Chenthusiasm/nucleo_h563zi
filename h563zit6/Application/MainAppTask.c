/* Includes --------------------------------------------------------------------------------------*/

#include <stdbool.h>

#include "app_freertos.h"
#include "DRV8870.h"
#include "FreeRTOS.h"
#include "Mutex.h"
#include "PWM.h"
#include "RTOS.h"
#include "tim.h"
#include "Timer.h"
#include "sys_command_line.h"


/* Internal typedef ------------------------------------------------------------------------------*/

typedef struct {
    uint16_t dutyCycle;
    uint16_t const dutyCycleIncrement;
    DRV8870_Direction_t direction;
} DriveConfig_t;


/* Internal define -------------------------------------------------------------------------------*/

#define EN_COAST                        (false)

#define EN_MOTOR1                       (false)

#define LOOP_DELAY_MS                   (200u)

#define TIM1_CHANNELS                   (4u)

#define TIM2_CHANNELS                   (4u)

#define PWM1_SWITCHING_FREQUENCY_HZ     (5000u)

#define PWM2_SWITCHING_FREQUENCY_HZ     (10000u)

#define PWM1_DUTY_CYCLE_INCREMENT       (10u)

#define PWM2_DUTY_CYCLE_INCREMENT       (50u)


/* Internal macro --------------------------------------------------------------------------------*/


/* Internal variables ----------------------------------------------------------------------------*/

static osMutexId_t tim1MutexHandle;
static osMutexId_t tim2MutexHandle;

static StaticSemaphore_t tim1MutexCB;
static StaticSemaphore_t tim2MutexCB;

static Mutex tim1Mutex;
static Mutex tim2Mutex;

static Timer tim1;
static Timer tim2;

static PWM pwmTim1[TIM1_CHANNELS];
static PWM pwmTim2[TIM2_CHANNELS];

static uint16_t dutyCycle1 = 0u;
static uint16_t dutyCycle2 = 0u;

static DRV8870 motorDriver0;
static DRV8870 motorDriver1;

static DriveConfig_t config0 = {
    .dutyCycle = 0u,
    .dutyCycleIncrement = PWM1_DUTY_CYCLE_INCREMENT,
    .direction = DRV8870_DIRECTION_FORWARD,
};
static DriveConfig_t config1 = {
    .dutyCycle = 0u,
    .dutyCycleIncrement = PWM2_DUTY_CYCLE_INCREMENT,
    .direction = DRV8870_DIRECTION_REVERSE,
};


/* Internal constants ----------------------------------------------------------------------------*/

static const osMutexAttr_t Tim1MutexAttributes = {
  .name = "tim1_mutex",
  .cb_mem = &tim1MutexCB,
  .cb_size = sizeof(tim1MutexCB),
};

static const osMutexAttr_t Tim2MutexAttributes = {
  .name = "tim2_mutex",
  .cb_mem = &tim2MutexCB,
  .cb_size = sizeof(tim2MutexCB),
};


/* Internal function prototypes ------------------------------------------------------------------*/


/* Internal functions ----------------------------------------------------------------------------*/

static void init(void) {
    printf("init()\n");

    // construct mutexes
    tim1MutexHandle = osMutexNew(&Tim1MutexAttributes);
    tim1Mutex = Mutex_ctor(tim1MutexHandle);
    tim2MutexHandle = osMutexNew(&Tim2MutexAttributes);
    tim2Mutex = Mutex_ctor(tim2MutexHandle);

    // construct timers
    tim1 = Timer_ctor(&htim1, TIM1, &tim1Mutex);
    tim2 = Timer_ctor(&htim2, TIM2, &tim2Mutex);

    // construct and init PWM
    for (uint8_t channel = 0u; channel < TIM1_CHANNELS; ++channel) {
        pwmTim1[channel] = PWM_ctor(&tim1, channel);
        PWM *pwmPtr = &pwmTim1[channel];
        uint32_t swFreq = PWM1_SWITCHING_FREQUENCY_HZ;
        uint16_t dutyCycle = dutyCycle1;
        //printf("TIM1 ch%u\n", channel);
        PWM_Err_t err = PWM_Init(pwmPtr, swFreq, dutyCycle);
        //printf("    PWM_Init(self, %lu, %u) = %u\n", swFreq, dutyCycle, err);
        swFreq = PWM_GetSwitchingFrequency_hz(pwmPtr);
        //printf("    PWM_GetSwitchingFrequency_hz(self) = %lu\n", swFreq);
        dutyCycle = PWM_GetDutyCycle_tenthPct(pwmPtr);
        //printf("    PWM_GetDutyCycle_tenthPct(self) = %u\n", dutyCycle);
        err = PWM_Start(pwmPtr);
        //printf("    PWM_Start(self) = %u\n", err);
    }
    for (uint8_t channel = 0u; channel < TIM2_CHANNELS; ++channel) {
        pwmTim2[channel] = PWM_ctor(&tim2, channel);
        PWM *pwmPtr = &pwmTim2[channel];
        uint32_t swFreq = PWM2_SWITCHING_FREQUENCY_HZ;
        uint16_t dutyCycle = dutyCycle2;
        //printf("TIM2 ch%u\n", channel);
        PWM_Err_t err = PWM_Init(pwmPtr, swFreq, dutyCycle);
        //printf("    PWM_Init(self, %lu, %u) = %u\n", swFreq, dutyCycle, err);
        swFreq = PWM_GetSwitchingFrequency_hz(pwmPtr);
        //printf("    PWM_GetSwitchingFrequency_hz(self) = %lu\n", swFreq);
        dutyCycle = PWM_GetDutyCycle_tenthPct(pwmPtr);
        //printf("    PWM_GetDutyCycle_tenthPct(self) = %u\n", dutyCycle);
        err = PWM_Start(pwmPtr);
        //printf("    PWM_Start(self) = %u\n", err);
    }
}

static uint16_t incrementDutyCycle(uint16_t dutyCycle, uint16_t increment) {
    dutyCycle += increment;
    if (dutyCycle > 1000u) {
        dutyCycle = 0u;
    }
    return dutyCycle;
}

static void processCycle(void) {
    dutyCycle1 = incrementDutyCycle(dutyCycle1, PWM1_DUTY_CYCLE_INCREMENT);
    for (uint8_t channel = 0u; channel < TIM1_CHANNELS; ++channel) {
        PWM *pwmPtr = &pwmTim1[channel];
        PWM_Err_t err = PWM_SetDutyCycle(pwmPtr, dutyCycle1);
        if (err != PWM_ERR_NONE) {
            printf("    PWM_SetDutyCycle(self, %u) = %u\n", dutyCycle1, err);
        }
        uint16_t dutyCycle = PWM_GetDutyCycle_tenthPct(pwmPtr);
        if (dutyCycle != dutyCycle1) {
            printf("!ERR: TIM1 ch%u; dutyCycle %u != %u\n", channel, dutyCycle, dutyCycle1);
        }
    }
#if EN_MOTOR1
    dutyCycle2 = incrementDutyCycle(dutyCycle2, PWM2_DUTY_CYCLE_INCREMENT);
    for (uint8_t channel = 0u; channel < TIM2_CHANNELS; ++channel) {
        PWM *pwmPtr = &pwmTim2[channel];
        PWM_Err_t err = PWM_SetDutyCycle(pwmPtr, dutyCycle2);
        if (err != PWM_ERR_NONE) {
            printf("    PWM_SetDutyCycle(self, %u) = %u\n", dutyCycle2, err);
        }
        uint16_t dutyCycle = PWM_GetDutyCycle_tenthPct(pwmPtr);
        if (dutyCycle != dutyCycle2) {
            printf("!ERR: TIM1 ch%u; dutyCycle %u != %u\n", channel, dutyCycle, dutyCycle2);
        }
    }
#endif /* EN_MOTOR1 */
}

static void initDRV8870(void) {
    printf("init()\n");

    // construct timers and mutexes
    tim1MutexHandle = osMutexNew(&Tim1MutexAttributes);
    tim1Mutex = Mutex_ctor(tim1MutexHandle);
    tim1 = Timer_ctor(&htim1, TIM1, &tim1Mutex);

#if EN_MOTOR1
    tim2MutexHandle = osMutexNew(&Tim2MutexAttributes);
    tim2Mutex = Mutex_ctor(tim2MutexHandle);
    tim2 = Timer_ctor(&htim2, TIM2, &tim2Mutex);
#endif

    // construct motors
    motorDriver0 = DRV8870_ctor(&tim1, 0u, 1u);
    DRV8870_Err_t err = DRV8870_Init(&motorDriver0, PWM1_SWITCHING_FREQUENCY_HZ);
    printf("    DRV8870_Init(%u, %u)=%u\n", 0, PWM1_SWITCHING_FREQUENCY_HZ, err);
#if EN_MOTOR1
    motorDriver1 = DRV8870_ctor(&tim2, 0u, 1u);
    err = DRV8870_Init(&motorDriver1, PWM2_SWITCHING_FREQUENCY_HZ);
    printf("    DRV8870_Init(%u, %u)=%u\n", 1, PWM2_SWITCHING_FREQUENCY_HZ, err);
#endif /* EN_MOTOR1 */
}

static void updateDriveConfig(DriveConfig_t *const configPtr) {
    if (configPtr == NULL) {
        return;
    }
    configPtr->dutyCycle += configPtr->dutyCycleIncrement;
    if (configPtr->dutyCycle > 1000u) {
        configPtr->dutyCycle = 0u;
        if (configPtr->direction == DRV8870_DIRECTION_FORWARD) {
            configPtr->direction = DRV8870_DIRECTION_REVERSE;
            return;
        }
        configPtr->direction = DRV8870_DIRECTION_FORWARD;
    }
}

static void logStatus(DRV8870 *const motorPtr) {
    bool isStopped = DRV8870_IsStopped(motorPtr);
    DRV8870_Direction_t direction = DRV8870_GetDirection(motorPtr);
    uint16_t strength = DRV8870_GetStrength_tenthPct(motorPtr);
    printf("    stopped=%u; direction=%u; strength=%u\n", isStopped, direction, strength);
}

static void updateDrive(uint8_t id, DRV8870 *const motorPtr, DriveConfig_t const *const configPtr) {
    if (motorPtr == NULL) {
        return;
    }
    if (configPtr == NULL) {
        return;
    }
    if (configPtr->dutyCycle == 0u) {
#if EN_COAST
        DRV8870_Err_t err = DRV8870_Coast(motorPtr);
        printf("DRV8870_Coast(%u)=%u\n", id, err);
#else
        DRV8870_Err_t err = DRV8870_Brake(motorPtr);
        printf("DRV8870_Brake(%u)=%u\n", id, err);
#endif /* EN_COAST */
        logStatus(motorPtr);
        return;
    }
    DRV8870_Err_t err = DRV8870_Drive(motorPtr, configPtr->direction, configPtr->dutyCycle);
    printf("DRV8870_Drive(%u,%u,%u)=%u\n", id, configPtr->direction, configPtr->dutyCycle, err);
    logStatus(motorPtr);
}

static void processCycleDRV8870(void) {
    updateDriveConfig(&config0);
    updateDrive(0u, &motorDriver0, &config0);
#if EN_MOTOR1
    updateDriveConfig(&config1);
    updateDrive(1u, &motorDriver1, &config1);
#endif /* EN_MOTOR1 */
}


/* External functions ----------------------------------------------------------------------------*/

void MainAppTask_Start(void *argument) {
    initDRV8870();
    osDelay(RTOS_ConvertMSToTicks(LOOP_DELAY_MS));
    for (;;) {
        processCycleDRV8870();
        osDelay(RTOS_ConvertMSToTicks(LOOP_DELAY_MS));
    }
}

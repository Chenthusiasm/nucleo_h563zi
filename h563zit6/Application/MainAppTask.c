/* Includes --------------------------------------------------------------------------------------*/

#include <stdbool.h>

#include "app_freertos.h"
#include "FreeRTOS.h"
#include "Mutex.h"
#include "PWM.h"
#include "RTOS.h"
#include "tim.h"
#include "Timer.h"
#include "sys_command_line.h"


/* Internal typedef ------------------------------------------------------------------------------*/


/* Internal define -------------------------------------------------------------------------------*/

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


/* External functions ----------------------------------------------------------------------------*/

void MainAppTask_Start(void *argument) {
    init();
    osDelay(RTOS_ConvertMSToTicks(LOOP_DELAY_MS));
    for (;;) {
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
        osDelay(RTOS_ConvertMSToTicks(LOOP_DELAY_MS));
    }
}

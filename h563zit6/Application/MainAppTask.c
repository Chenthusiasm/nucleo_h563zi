/* Includes --------------------------------------------------------------------------------------*/

#include <stdbool.h>

#include "app_freertos.h"
#include "Mutex.h"
#include "sys_command_line.h"
#include "tim.h"
#include "RTOSHelper.h"


/* Internal typedef ------------------------------------------------------------------------------*/

typedef struct {
    TIM_HandleTypeDef *htim;
    uint32_t channel;
} PWM_Channel_t;


/* Internal define -------------------------------------------------------------------------------*/


/* Internal macro --------------------------------------------------------------------------------*/


/* Internal variables ----------------------------------------------------------------------------*/

static PWM_Channel_t channels[] = {
    [ 0] = { .htim = &htim1, .channel = TIM_CHANNEL_1 },
    [ 1] = { .htim = &htim1, .channel = TIM_CHANNEL_2 },
    [ 2] = { .htim = &htim1, .channel = TIM_CHANNEL_3 },
    [ 3] = { .htim = &htim1, .channel = TIM_CHANNEL_4 },

    [ 4] = { .htim = &htim2, .channel = TIM_CHANNEL_1 },
    [ 5] = { .htim = &htim2, .channel = TIM_CHANNEL_2 },
    [ 6] = { .htim = &htim2, .channel = TIM_CHANNEL_3 },
    [ 7] = { .htim = &htim2, .channel = TIM_CHANNEL_4 },

    [ 8] = { .htim = &htim3, .channel = TIM_CHANNEL_1 },
    [ 9] = { .htim = &htim3, .channel = TIM_CHANNEL_2 },
    [10] = { .htim = &htim3, .channel = TIM_CHANNEL_3 },
    [11] = { .htim = &htim3, .channel = TIM_CHANNEL_4 },

    [12] = { .htim = &htim4, .channel = TIM_CHANNEL_1 },
    [13] = { .htim = &htim4, .channel = TIM_CHANNEL_2 },
    [14] = { .htim = &htim4, .channel = TIM_CHANNEL_3 },
    [15] = { .htim = &htim4, .channel = TIM_CHANNEL_4 },

    [16] = { .htim = &htim5, .channel = TIM_CHANNEL_1 },
    [17] = { .htim = &htim5, .channel = TIM_CHANNEL_2 },
    [18] = { .htim = &htim5, .channel = TIM_CHANNEL_3 },
    [19] = { .htim = &htim5, .channel = TIM_CHANNEL_4 },

    [20] = { .htim = &htim8, .channel = TIM_CHANNEL_1 },

    [21] = { .htim = &htim12, .channel = TIM_CHANNEL_1 },
    [22] = { .htim = &htim12, .channel = TIM_CHANNEL_2 },

    [23] = { .htim = &htim13, .channel = TIM_CHANNEL_1 },

    [23] = { .htim = &htim14, .channel = TIM_CHANNEL_1 },

    [24] = { .htim = &htim15, .channel = TIM_CHANNEL_1 },
    [25] = { .htim = &htim15, .channel = TIM_CHANNEL_2 },

    [26] = { .htim = &htim16, .channel = TIM_CHANNEL_1 },

    [27] = { .htim = &htim17, .channel = TIM_CHANNEL_1 },
};

static uint16_t numChannels = sizeof(channels) / sizeof(channels[0]);


/* Internal function prototypes ------------------------------------------------------------------*/


/* Internal functions ----------------------------------------------------------------------------*/

static void startAll(void) {
    for (uint16_t channel = 0; channel < numChannels; ++channel) {
        HAL_TIM_PWM_Start(channels[channel].htim, channels[channel].channel);
    }
}


static void addAll(void) {
    for (uint16_t channel = 0; channel < numChannels; ++channel) {
        uint32_t compare = __HAL_TIM_GET_COMPARE(channels[channel].htim, channels[channel].channel);
        compare += 100u;
        __HAL_TIM_SET_COMPARE(channels[channel].htim, channels[channel].channel, compare);
    }
}

static void setAllLow(void) {
    for (uint16_t channel = 0; channel < numChannels; ++channel) {
        __HAL_TIM_SET_COMPARE(channels[channel].htim, channels[channel].channel, 0);
    }
}


static void setAllHigh(void) {
    for (uint16_t channel = 0; channel < numChannels; ++channel) {
        uint32_t autoreload = __HAL_TIM_GET_AUTORELOAD(channels[channel].htim);
        __HAL_TIM_SET_COMPARE(channels[channel].htim, channels[channel].channel, autoreload);
    }
}

/* External functions ----------------------------------------------------------------------------*/

void MainAppTask_Start(void *argument) {
    startAll();

    for (;;) {
        setAllLow();
        osDelay(1000u);
        setAllHigh();
        osDelay(1000u);
    }
}

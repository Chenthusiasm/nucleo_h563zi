/* Includes --------------------------------------------------------------------------------------*/

#include <stdbool.h>

#include "app_freertos.h"
#include "DIO.h"
#include "DIO_IRQ.h"
#include "RTOS.h"
#include "sys_command_line.h"


/* Internal typedef ------------------------------------------------------------------------------*/


/* Internal define -------------------------------------------------------------------------------*/

#define LOOP_DELAY_MS                   (1u)
#define LED_TOGGLE_DELAY_MS             (500u)


/* Internal macro --------------------------------------------------------------------------------*/


/* Internal variables ----------------------------------------------------------------------------*/

static DIO ld1;
static DIO ld2;
static DIO ld3;
static DIO userButton;
static DIO inputPC0;
static DIO inputPC1;
static DIO inputPC2;
static DIO inputPC3;
static DIO inputPC4;
static DIO inputPC5;
static DIO inputPC6;
static DIO inputPC7;
static DIO inputPC8;
static DIO inputPC9;
static DIO inputPC10;
static DIO inputPC11;
static DIO inputPC12;


/* Internal function prototypes ------------------------------------------------------------------*/


/* Internal functions ----------------------------------------------------------------------------*/

void userButtonCallback(uint8_t pin, DIO_Transition_t transition) {
    if (transition == DIO_TRANSITION_FALLING_EDGE) {
        DIO_SetLow(&ld3);
    } else if (transition == DIO_TRANSITION_RISING_EDGE) {
        DIO_SetHigh(&ld3);
    }
}


void generalIOCallback(uint8_t pin, DIO_Transition_t transition) {
    printf("callback; pin=%u; transition=%u\n", pin, transition);
}


static void init(void) {
    printf("init()\n");

    // construct DIOs
    ld1 = DIO_ctor(LD1_GPIO_Port, DIO_GetPin(LD1_Pin), NULL);
    ld2 = DIO_ctor(LD2_GPIO_Port, DIO_GetPin(LD2_Pin), NULL);
    ld3 = DIO_ctor(LD3_GPIO_Port, DIO_GetPin(LD3_Pin), NULL);
    userButton = DIO_ctor(USER_BUTTON_GPIO_Port, DIO_GetPin(USER_BUTTON_Pin), userButtonCallback);
    inputPC0  = DIO_ctor(GPIOC, DIO_GetPin(GPIO_PIN_0) , generalIOCallback);
    inputPC1  = DIO_ctor(GPIOC, DIO_GetPin(GPIO_PIN_1) , generalIOCallback);
    inputPC2  = DIO_ctor(GPIOC, DIO_GetPin(GPIO_PIN_2) , generalIOCallback);
    inputPC3  = DIO_ctor(GPIOC, DIO_GetPin(GPIO_PIN_3) , generalIOCallback);
    inputPC4  = DIO_ctor(GPIOC, DIO_GetPin(GPIO_PIN_4) , generalIOCallback);
    inputPC5  = DIO_ctor(GPIOC, DIO_GetPin(GPIO_PIN_5) , generalIOCallback);
    inputPC6  = DIO_ctor(GPIOC, DIO_GetPin(GPIO_PIN_6) , generalIOCallback);
    inputPC7  = DIO_ctor(GPIOC, DIO_GetPin(GPIO_PIN_7) , generalIOCallback);
    inputPC8  = DIO_ctor(GPIOC, DIO_GetPin(GPIO_PIN_8) , generalIOCallback);
    inputPC9  = DIO_ctor(GPIOC, DIO_GetPin(GPIO_PIN_9) , generalIOCallback);
    inputPC10 = DIO_ctor(GPIOC, DIO_GetPin(GPIO_PIN_10), generalIOCallback);
    inputPC11 = DIO_ctor(GPIOC, DIO_GetPin(GPIO_PIN_11), generalIOCallback);
    inputPC12 = DIO_ctor(GPIOC, DIO_GetPin(GPIO_PIN_12), generalIOCallback);

    // init DIO_IRQ
    DIO_IRQ_Err_t irqErr = DIO_IRQ_Init();
    printf("    DIO_IRQ_Init()=%u\n", irqErr);

    // init DIO
    DIO_Err_t err = DIO_Init(&ld1);
    printf("    DIO_Init(&ld1)=%u\n", err);
    err = DIO_SetHigh(&ld1);
    printf("    DIO_SetHigh(&ld1)=%u\n", err);
    err = DIO_Init(&ld2);
    printf("    DIO_Init(&ld2)=%u\n", err);
    err = DIO_SetHigh(&ld2);
    printf("    DIO_SetHigh(&ld2)=%u\n", err);
    err = DIO_Init(&ld3);
    printf("    DIO_Init(&ld3)=%u\n", err);
    err = DIO_SetLow(&ld3);
    printf("    DIO_SetLow(&ld3)=%u\n", err);
    err = DIO_Init(&userButton);
    printf("    DIO_Init(&userButton)=%u\n", err);
}


/* External functions ----------------------------------------------------------------------------*/

void MainAppTask_Start(void *argument) {
    init();
    for (;;) {
        DIO_Err_t err = DIO_Toggle(&ld1);
        printf("    DIO_Toggle(&ld1)=%u\n", err);
        err = DIO_Toggle(&ld2);
        printf("    DIO_Toggle(&ld2)=%u\n", err);
        osDelay(RTOS_ConvertMSToTicks(LED_TOGGLE_DELAY_MS));
    }
}

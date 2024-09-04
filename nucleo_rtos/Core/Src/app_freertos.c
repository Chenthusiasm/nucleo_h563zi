/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : app_freertos.c
  * Description        : FreeRTOS applicative file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "app_freertos.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
typedef StaticTask_t osStaticThreadDef_t;
typedef StaticQueue_t osStaticMessageQDef_t;
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for dynamicTask */
osThreadId_t dynamicTaskHandle;
const osThreadAttr_t dynamicTask_attributes = {
  .name = "dynamicTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 128 * 4
};
/* Definitions for staticTask */
osThreadId_t staticTaskHandle;
uint32_t staticTaskBuffer[ 128 ];
osStaticThreadDef_t staticTaskTCB;
const osThreadAttr_t staticTask_attributes = {
  .name = "staticTask",
  .stack_mem = &staticTaskBuffer[0],
  .stack_size = sizeof(staticTaskBuffer),
  .cb_mem = &staticTaskTCB,
  .cb_size = sizeof(staticTaskTCB),
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for dynamicQueue */
osMessageQueueId_t dynamicQueueHandle;
const osMessageQueueAttr_t dynamicQueue_attributes = {
  .name = "dynamicQueue"
};
/* Definitions for staticQueue */
osMessageQueueId_t staticQueueHandle;
uint8_t staticQueueBuffer[ 8 * sizeof( uint32_t ) ];
osStaticMessageQDef_t staticQueueTCB;
const osMessageQueueAttr_t staticQueue_attributes = {
  .name = "staticQueue",
  .cb_mem = &staticQueueTCB,
  .cb_size = sizeof(staticQueueTCB),
  .mq_mem = &staticQueueBuffer,
  .mq_size = sizeof(staticQueueBuffer)
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */
  /* creation of dynamicQueue */
  dynamicQueueHandle = osMessageQueueNew (16, sizeof(uint16_t), &dynamicQueue_attributes);
  /* creation of staticQueue */
  staticQueueHandle = osMessageQueueNew (8, sizeof(uint32_t), &staticQueue_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */
  /* creation of dynamicTask */
  dynamicTaskHandle = osThreadNew(startDynamicTask, NULL, &dynamicTask_attributes);

  /* creation of staticTask */
  staticTaskHandle = osThreadNew(startStaticTask, NULL, &staticTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}
/* USER CODE BEGIN Header_startDynamicTask */
/**
* @brief Function implementing the dynamicTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_startDynamicTask */
__weak void startDynamicTask(void *argument)
{
  /* USER CODE BEGIN dynamicTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END dynamicTask */
}

/* USER CODE BEGIN Header_startStaticTask */
/**
* @brief Function implementing the staticTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_startStaticTask */
__weak void startStaticTask(void *argument)
{
  /* USER CODE BEGIN staticTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END staticTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */


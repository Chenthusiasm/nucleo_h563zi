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
typedef StaticSemaphore_t osStaticMutexDef_t;
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
/* Definitions for MainAppTask */
osThreadId_t MainAppTaskHandle;
uint32_t MainAppTaskBuffer[ 128 ];
osStaticThreadDef_t MainAppTaskCB;
const osThreadAttr_t MainAppTask_attributes = {
  .name = "MainAppTask",
  .stack_mem = &MainAppTaskBuffer[0],
  .stack_size = sizeof(MainAppTaskBuffer),
  .cb_mem = &MainAppTaskCB,
  .cb_size = sizeof(MainAppTaskCB),
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for DiagnosticsTask */
osThreadId_t DiagnosticsTaskHandle;
uint32_t DiagnosticsTaskBuffer[ 1024 ];
osStaticThreadDef_t DiagnosticsTaskCB;
const osThreadAttr_t DiagnosticsTask_attributes = {
  .name = "DiagnosticsTask",
  .stack_mem = &DiagnosticsTaskBuffer[0],
  .stack_size = sizeof(DiagnosticsTaskBuffer),
  .cb_mem = &DiagnosticsTaskCB,
  .cb_size = sizeof(DiagnosticsTaskCB),
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for TestMutex */
osMutexId_t TestMutexHandle;
osStaticMutexDef_t TestMutexCB;
const osMutexAttr_t TestMutex_attributes = {
  .name = "TestMutex",
  .cb_mem = &TestMutexCB,
  .cb_size = sizeof(TestMutexCB),
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
  /* creation of TestMutex */
  TestMutexHandle = osMutexNew(&TestMutex_attributes);

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */
  /* creation of MainAppTask */
  MainAppTaskHandle = osThreadNew(MainAppTask_Start, NULL, &MainAppTask_attributes);

  /* creation of DiagnosticsTask */
  DiagnosticsTaskHandle = osThreadNew(DiagnosticsTask_Start, NULL, &DiagnosticsTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}
/* USER CODE BEGIN Header_MainAppTask_Start */
/**
* @brief Function implementing the MainAppTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_MainAppTask_Start */
__weak void MainAppTask_Start(void *argument)
{
  /* USER CODE BEGIN MainAppTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END MainAppTask */
}

/* USER CODE BEGIN Header_DiagnosticsTask_Start */
/**
* @brief Function implementing the DiagnosticsTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_DiagnosticsTask_Start */
__weak void DiagnosticsTask_Start(void *argument)
{
  /* USER CODE BEGIN DiagnosticsTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END DiagnosticsTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */


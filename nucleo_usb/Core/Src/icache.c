/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    icache.c
  * @brief   This file provides code for the configuration
  *          of the ICACHE instances.
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
#include "icache.h"

/* USER CODE BEGIN 0 */
/* MPU attribute setting */
#define  DEVICE_nGnRnE          0x0U  /* Device, noGather, noReorder, noEarly acknowledge. */
#define  DEVICE_nGnRE           0x4U  /* Device, noGather, noReorder, Early acknowledge.   */
#define  DEVICE_nGRE            0x8U  /* Device, noGather, Reorder, Early acknowledge.     */
#define  DEVICE_GRE             0xCU  /* Device, Gather, Reorder, Early acknowledge.       */

#define  WRITE_THROUGH          0x0U  /* Normal memory, write-through. */
#define  NOT_CACHEABLE          0x4U  /* Normal memory, non-cacheable. */
#define  WRITE_BACK             0x4U  /* Normal memory, write-back.    */

#define  TRANSIENT              0x0U  /* Normal memory, transient.     */
#define  NON_TRANSIENT          0x8U  /* Normal memory, non-transient. */

#define  NO_ALLOCATE            0x0U  /* Normal memory, no allocate.         */
#define  W_ALLOCATE             0x1U  /* Normal memory, write allocate.      */
#define  R_ALLOCATE             0x2U  /* Normal memory, read allocate.       */
#define  RW_ALLOCATE            0x3U  /* Normal memory, read/write allocate. */

#define OUTER(__ATTR__)        ((__ATTR__) << 4U)
#define INNER_OUTER(__ATTR__)  ((__ATTR__) | ((__ATTR__) << 4U))

/**
  * @brief  Configure the MPU attributes.
  * @note   The Base Address is RO area
  * https://community.st.com/t5/stm32-mcus/how-to-avoid-a-hardfault-when-icache-is-enabled-on-the-stm32h5/ta-p/630085
  *   None
  * @retval None
  */
static void MPU_Config(void)
{
  MPU_Attributes_InitTypeDef   attr;
  MPU_Region_InitTypeDef       region;

  /* Disable MPU before perloading and config update */
  HAL_MPU_Disable();

  /* Define cacheable memory via MPU */
  attr.Number             = MPU_ATTRIBUTES_NUMBER0;
  attr.Attributes         = INNER_OUTER(NOT_CACHEABLE);
  HAL_MPU_ConfigMemoryAttributes(&attr);

  /* BaseAddress-LimitAddress configuration */
  region.Enable           = MPU_REGION_ENABLE;
  region.Number           = MPU_REGION_NUMBER0;
  region.AttributesIndex  = MPU_ATTRIBUTES_NUMBER0;
  region.BaseAddress      = 0x08FFF800;
  region.LimitAddress     = 0x08FFFFFF;
  region.AccessPermission = MPU_REGION_ALL_RW;
  region.DisableExec      = MPU_INSTRUCTION_ACCESS_DISABLE;
  region.IsShareable      = MPU_ACCESS_NOT_SHAREABLE;
  HAL_MPU_ConfigRegion(&region);

  /* Enable the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}
/* USER CODE END 0 */

/* ICACHE init function */
void MX_ICACHE_Init(void)
{

  /* USER CODE BEGIN ICACHE_Init 0 */
  MPU_Config();
  /* USER CODE END ICACHE_Init 0 */

  /* USER CODE BEGIN ICACHE_Init 1 */

  /* USER CODE END ICACHE_Init 1 */

  /** Enable instruction cache (default 2-ways set associative cache)
  */
  if (HAL_ICACHE_Enable() != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ICACHE_Init 2 */

  /* USER CODE END ICACHE_Init 2 */

}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

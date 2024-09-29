/**
 * @file    ICACHE.h
 * @brief   Implementation for the ICACHE (instruction cache) support.
 * @note    The driver is reentrant from the defined structures; the driver may not be reentrant
 *          if different instances of the structures are constructed using the same HW peripherals,
 *          specifically the ICACHE peripheral.
 * @note    The driver is implemented as a singleton.
 * @note    The driver's purpose is to address issues where STM32H5 MCUs can hard fault when
 *          accessing certain RO (read-only) and OTP (one-time-programmable) areas of memory.
 *          https://community.st.com/t5/stm32-mcus/how-to-avoid-a-hardfault-when-icache-is-enabled-on-the-stm32h5/ta-p/630085
 * @note    In order to use the ICACHE_Support driver, disable the STM32CubeMX auto-generation of
 *          the ICACHE code.
 * @section Legal Disclaimer
 *      ©2024 Whisker, All rights reserved. All contents of this source file and/or any other
 *      related source files are the explicit property of Whisker. Do not distribute. Do not copy.
 */

/* Includes --------------------------------------------------------------------------------------*/

#include <stdbool.h>

#include "stm32h5xx_hal.h"


/* Internal typedef ------------------------------------------------------------------------------*/


/* Internal define -------------------------------------------------------------------------------*/

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


/* Internal macro --------------------------------------------------------------------------------*/

#define OUTER(__ATTR__)        ((__ATTR__) << 4U)
#define INNER_OUTER(__ATTR__)  ((__ATTR__) | ((__ATTR__) << 4U))


/* Internal variables ----------------------------------------------------------------------------*/


/* Internal constants ----------------------------------------------------------------------------*/


/* Internal function prototypes ------------------------------------------------------------------*/


/* Internal functions ----------------------------------------------------------------------------*/

#if defined(ICACHE) && defined (HAL_ICACHE_MODULE_ENABLED)
/**
  * @brief  Configure the MPU attributes.
  * @note   The Base Address is the RO (read-only) area.
  */
static void mpuConfig(void) {
    MPU_Attributes_InitTypeDef   attr;
    MPU_Region_InitTypeDef       region;

    /* Disable MPU before config update */
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


/* External function prototypes ------------------------------------------------------------------*/


/* External functions ----------------------------------------------------------------------------*/

/**
 * @brief   Initializes the ICACHE (instruction cache) component.
 * @return  If the initialization is successful, true; otherwise, false.
 */
bool ICACHE_Init(void) {
    mpuConfig();
    if (HAL_ICACHE_Enable() != HAL_OK) {
        return false;
    }
    return true;
}
#else
bool ICACHE_Init(void) {
    return true;
}
#endif /* defined(ICACHE) && defined (HAL_ICACHE_MODULE_ENABLED) */

/**
 * @file    USB_CDC.c
 * @brief   Implementation for the USB CDC (communications device class) driver.
 * @note    The driver is reentrant from the defined structures; the driver may not be reentrant
 *          if different instances of the structures are constructed using the same HW peripherals,
 *          specifically the USB peripheral.
 * @note    The driver is implemented as a singleton.
 * @note    In order to use the USB_CDC driver, the classic USB device middleware with CDC support
 *          must be added to the project. Also, disable the STM32CubeMX auto-generation of the USB
 *          code.
 * @note    If the ICACHE is enabled, the ICACHE_Support module must be included to deal with
 *          avoiding hard faults when accessing RO and OTP memory.
 * @section Legal Disclaimer
 *      ©2024 Whisker, All rights reserved. All contents of this source file and/or any other
 *      related source files are the explicit property of Whisker. Do not distribute. Do not copy.
 */

/* Includes --------------------------------------------------------------------------------------*/

#include <assert.h>
#include <stdint.h>

#include "ICACHE_Support.h"
#include "usbd_cdc_if.h"
#include "usbd_core.h"
#include "usbd_desc.h"
#include "USB_CDC.h"


/* Internal typedef ------------------------------------------------------------------------------*/

/* Buffer defined before their usage in the USBCDC_Buffers_t definition. Defined early to be used
 * by the Buffers_t struct. */
#define TX_BUFFER_SIZE           (512u)
#define RX_BUFFER_SIZE           (512u)


/**
 * @struct  Buffers_t
 * @brief   Struct that contains the transmit and receive buffers used in the USB CDC library.
 * @var Buffers_t::tx Allocated memory for USB transmits.
 * @var Buffers_t::rx Allocated memory for USB receives.
 */
typedef struct {
    uint8_t txBuffer[TX_BUFFER_SIZE];
    uint8_t rxBuffer[RX_BUFFER_SIZE];
} Buffers_t;


/**
 * @struct  USB_CDC
 * @brief   Type definition of a structure that aggregates key components needed for the USB CDC
 *          function (as VCP).
 * @var USB_CDC.pcdHandle       Handle of the USB PCD (peripheral control driver).
 * @var USB_CDC.usbdHandle      Handle of the USB device class.
 * @var USB_CDC.usbMutexPtr     Pointer to the USB Mutex.
 * @var USB_CDC.receiveCallback Receive callback function.
 */
typedef struct {
    PCD_HandleTypeDef pcdHandle;
    USBD_HandleTypeDef usbdHandle;
    USB_CDC_ReceiveCallback_t receiveCallback;
    Buffers_t *buffers;
    bool initialized;
    USB_CDC_Err_t retainedError;
} USB_CDC;


/* Internal define -------------------------------------------------------------------------------*/


/* Internal macro --------------------------------------------------------------------------------*/


/* Internal variables ----------------------------------------------------------------------------*/

/* Singleton instance of the USB buffers. Initialized to 0. */
static Buffers_t usbBuffers = { 0 };

/* Singleton instance of the USB_CDC driver. All but the buffers are initialized to 0. The buffers
 * are initialized to point to the usbBuffers instance.
 */
static USB_CDC self = {
    .buffers = &usbBuffers,
};


/* Internal constants ----------------------------------------------------------------------------*/

/* HAL_StatusTypeDef map to USB_CDC_Err_t. */
static USB_CDC_Err_t const HALErrMap[] = {
    [HAL_OK]       = USB_CDC_ERR_NONE,
    [HAL_ERROR]    = USB_CDC_ERR_HAL_BUSY,
    [HAL_BUSY]     = USB_CDC_ERR_HAL_MEMORY,
    [HAL_TIMEOUT]  = USB_CDC_ERR_HAL_FAIL,
};


/* USBD_StatusTypeDef map to USB_CDC_Err_t. */
static USB_CDC_Err_t const USBDErrMap[] = {
    [USBD_OK]   = USB_CDC_ERR_NONE,
    [USBD_BUSY] = USB_CDC_ERR_MW_BUSY,
    [USBD_EMEM] = USB_CDC_ERR_MW_MEMORY,
    [USBD_FAIL] = USB_CDC_ERR_MW_FAIL,
};


/* Internal function prototypes ------------------------------------------------------------------*/


/* Internal functions ----------------------------------------------------------------------------*/

/**
 * @brief   Initialize the USB device.
 */
static USB_CDC_Err_t initUSBD(PCD_HandleTypeDef *const pcdHandle,
                              USBD_HandleTypeDef *const usbdHandle) {
    // caller confirms if pcdHandle is not NULL

    pcdHandle->pData = usbdHandle;
    pcdHandle->Instance = USB_DRD_FS;
    pcdHandle->Init.dev_endpoints = 8;
    pcdHandle->Init.speed = USBD_FS_SPEED;
    pcdHandle->Init.phy_itface = PCD_PHY_EMBEDDED;
    pcdHandle->Init.Sof_enable = DISABLE;
    pcdHandle->Init.low_power_enable = DISABLE;
    pcdHandle->Init.lpm_enable = DISABLE;
    pcdHandle->Init.battery_charging_enable = DISABLE;
    pcdHandle->Init.vbus_sensing_enable = DISABLE;
    pcdHandle->Init.bulk_doublebuffer_enable = DISABLE;
    pcdHandle->Init.iso_singlebuffer_enable = DISABLE;
    USB_CDC_Err_t err = HALErrMap[HAL_PCD_Init(pcdHandle)];
    if (err != USB_CDC_ERR_NONE) {
        return err;
    }
    err = USBDErrMap[USBD_Init(usbdHandle, &CDC_Desc, 0)];
    if (err != USB_CDC_ERR_NONE) {
        return err;
    }
    err = USBDErrMap[USBD_RegisterClass(usbdHandle, &USBD_CDC)];
    if (err != USB_CDC_ERR_NONE) {
        return err;
    }
    err = USBDErrMap[USBD_CDC_RegisterInterface(usbdHandle, &USBD_CDC_Template_fops)];
    if (err != USB_CDC_ERR_NONE) {
        return err;
    }
    err = USBDErrMap[USBD_Start(usbdHandle)];
    if (err != USB_CDC_ERR_NONE) {
        return err;
    }
    return USB_CDC_ERR_NONE;
}


/* External function prototypes ------------------------------------------------------------------*/


/* External functions ----------------------------------------------------------------------------*/

/**
 * @brief   Initializes the USB CDC driver singleton.
 * @return  The specific USB_CDC_Err_t which indicates the specific error code if the function
 *          couldn't be executed successfully. If the function executes successfully,
 *          USB_CDC_ERR_NONE.
 */
USB_CDC_Err_t USB_CDC_Init(void) {
    if (self.initialized == true) {
        return USB_CDC_ERR_NONE;
    }
    if (ICACHE_Init() != true) {
        return USB_CDC_ERR_HAL_FAIL;
    }
    USB_CDC_Err_t err = initUSBD(&self.pcdHandle, &self.usbdHandle);
    if (err != USB_CDC_ERR_NONE) {
        return err;
    }
    return USB_CDC_ERR_NONE;
}


/**
 * @brief   Deinitializes the USB CDC driver singleton.
 * @param[in]   pcdHandle   The handle to the USB PCD (peripheral control driver).
 * @param[in]   usbdHandle  The handle to the USB device.
 * @return  The specific USB_CDC_Err_t which indicates the specific error code if the function
 *          couldn't be executed successfully. If the function executes successfully,
 *          USB_CDC_ERR_NONE.
 */
USB_CDC_Err_t USB_CDC_Deinit(void) {
    if (self.initialized == false) {
        return USB_CDC_ERR_NONE;
    }
    USB_CDC_Err_t err = HALErrMap[HAL_PCD_DeInit(&self.pcdHandle)];
    if (err != USB_CDC_ERR_NONE) {
        return err;
    }
    self.initialized = false;
    return USB_CDC_ERR_NONE;
}


/**
 * @brief Transmit the specified buffer over the USB bus.
 * @param[in]   buffer  Buffer of data to transmit.
 * @param[in]   length  Number of bytes to transmit.
 * @return  The specific USB_CDC_Err_t which indicates the specific error code if the function
 *          couldn't be executed successfully. If the function executes successfully,
 *          USB_CDC_ERR_NONE.
 */
USB_CDC_Err_t USB_CDC_Transmit(uint8_t *const buffer, uint16_t length) {
    if (buffer == NULL) {
        return USB_CDC_ERR_NULL_PARAM;
    }
    if (length == 0u) {
        return USB_CDC_ERR_INVALID_PARAM;
    }
    if (self.usbdHandle.pClassData == NULL) {
        // USB has not been connected; do not transmit but return with no error
        return USB_CDC_ERR_NONE;
    }
    USB_CDC_Err_t err = USBD_CDC_Transmit(buffer, length);
    return err;
}


/**
  * @brief  Initializes the USB PCD MSP.
  * @param  pcdHandle   The USB PCD handle.
  * @note   HAL_PCD_MspInit is defined as a weak function in the HAL SDK: see stm32h5xx_hal_pcd.c.
  *         This implementation will be used over that weak implementation.
  * @note   The implementation is based on the STM32CubeIDE auto-generated implementation; the only
  *         addition is the call to HAL_PWREx_EnableVddUSB().
  */
void HAL_PCD_MspInit(PCD_HandleTypeDef* pcdHandle) {
    // caller confirms if pcdHandle is not NULL
    HAL_PWREx_EnableVddUSB();

    // Initializes the peripherals clock
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {
        .PeriphClockSelection = RCC_PERIPHCLK_USB,
        .UsbClockSelection = RCC_USBCLKSOURCE_HSI48,
    };
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
        assert(false);
    }

    // USB_DRD_FS clock enable
    __HAL_RCC_USB_CLK_ENABLE();

    // USB_DRD_FS interrupt init
    HAL_NVIC_SetPriority(USB_DRD_FS_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(USB_DRD_FS_IRQn);
    return;
}


/**
  * @brief  DeInitializes USB PCD MSP.
  * @param  pcdHandle   The USB PCD handle
  * @note   HAL_PCD_MspDeInit is defined as a weak function in the HAL SDK: see stm32h5xx_hal_pcd.c.
  *         This implementation will be used over that weak implementation.
  * @note   The implementation is based on the STM32CubeIDE auto-generated implementation; the only
  *         addition is the call to HAL_PWREx_DisableeVddUSB().
  */
void HAL_PCD_MspDeInit(PCD_HandleTypeDef* pcdHandle) {
    // caller confirms if pcdHandle is not NULL

    HAL_PWREx_DisableVddUSB();
    // peripheral clock disable
    __HAL_RCC_USB_CLK_DISABLE();
    // USB_DRD_FS interrupt deinit
    HAL_NVIC_DisableIRQ(USB_DRD_FS_IRQn);
}

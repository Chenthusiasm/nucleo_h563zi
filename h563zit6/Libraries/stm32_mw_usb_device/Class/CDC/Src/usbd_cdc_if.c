/**
 * @file    usbd_cdc_if.h
 * @brief   Interface file for the USB CDC (communications device class) driver.
 * @note    The driver is reentrant from the defined structures; the driver may not be reentrant
 *          if different instances of the structures are constructed using the same HW peripherals,
 *          specifically the USB peripheral.
 * @note    The driver is implemented as a singleton.
 * @note    In order to use the USB_CDC driver, the classic USB device middleware with CDC support
 *          must be added to the project. Also, disable the STM32CubeMX auto-generation of the USB
 *          code.
 * @note    If the ICACHE is enabled, the ICACHE_Support module must be included to deal with
 *          avoiding hard faults when accessing RO and OTP memory.
 * @note    Originally based on the work from the STM MCD Application Team with modifications based
 *          on the following knowledge base article and the intent to make platform USB CDC drivers.
 *          https://community.st.com/t5/stm32-mcus/how-to-use-stmicroelectronics-classic-usb-device-middleware-with/ta-p/599274
 * @section Legal Disclaimer
 *      ©2024 Whisker, All rights reserved. All contents of this source file and/or any other
 *      related source files are the explicit property of Whisker. Do not distribute. Do not copy.
 */

/* Includes ------------------------------------------------------------------------------------- */

#include <assert.h>

#include "usbd_cdc_if.h"
#include "usbd_core.h"
#include "usbd_desc.h"


/* Internal typedef ------------------------------------------------------------------------------*/

/* Buffer defined before their usage in the USBCDC_Buffers_t definition. Defined early to be used
 * by the Buffers_t struct. */
#define TX_BUFFER_SIZE           (512u)
#define RX_BUFFER_SIZE           (512u)

/**
 * @struct  USB_CDC
 * @brief   Type definition of a structure that aggregates key components needed for the USB CDC
 *          function (as VCP).
 * @var USB_CDC.pcdHandle       Handle of the USB PCD (peripheral control driver).
 * @var USB_CDC.usbdHandle      Handle of the USB device class.
 * @var USB_CDC.receiveCallback Receive callback function.
 * @var USB_CDC::txBuffer       Allocated memory for USB transmits.
 * @var USB_CDC::rxBuffer       Allocated memory for USB receives.
 */
typedef struct {
    PCD_HandleTypeDef pcdHandle;
    USBD_HandleTypeDef usbdHandle;
    USB_CDC_ReceiveCallback_t receiveCallback;
    uint8_t txBuffer[TX_BUFFER_SIZE];
    uint8_t rxBuffer[RX_BUFFER_SIZE];
    bool initialized;
} USB_CDC;


/* Internal define -------------------------------------------------------------------------------*/


/* Internal macro --------------------------------------------------------------------------------*/


/* Internal variables ----------------------------------------------------------------------------*/

/* Singleton instance of the USB_CDC driver. All initialized to 0. */
static USB_CDC self = { 0 };

/* Forward declaration of private function prototypes */
static int8_t USBD_CDC_Init(void);
static int8_t USBD_CDC_DeInit(void);
static int8_t USBD_CDC_Control(uint8_t cmd, uint8_t *buffer, uint16_t length);
static int8_t USBD_CDC_Receive(uint8_t *buffer, uint32_t *lengthPtr);
static int8_t USBD_CDC_TransmitCplt(uint8_t *pbuf, uint32_t *Len, uint8_t epnum);

/* */
USBD_CDC_ItfTypeDef USBD_CDC_Template_fops = {
    .Init           = USBD_CDC_Init,
    .DeInit         = USBD_CDC_DeInit,
    .Control        = USBD_CDC_Control,
    .Receive        = USBD_CDC_Receive,
    .TransmitCplt   = USBD_CDC_TransmitCplt
};

/* */
USBD_CDC_LineCodingTypeDef linecoding = {
    .bitrate    = 115200,   /* baud rate*/
    .format     = 0x00,     /* stop bits-1*/
    .paritytype = 0x00,     /* parity - none*/
    .datatype   = 0x08,     /* nb. of bits 8*/
};


/* Internal constants ----------------------------------------------------------------------------*/

/* HAL_StatusTypeDef map to USBD_StatusTypeDef. */
static USBD_StatusTypeDef const HALStatusMap[] = {
    [HAL_OK]       = USBD_OK,
    [HAL_ERROR]    = USBD_BUSY,
    [HAL_BUSY]     = USBD_EMEM,
    [HAL_TIMEOUT]  = USBD_FAIL,
};


/* External variables ----------------------------------------------------------------------------*/

/* Provide external access to the pointer to the USB PCD (peripheral control driver). */
PCD_HandleTypeDef *hpcd_USB_DRD_FS = &self.pcdHandle;


/* Internal function prototypes ------------------------------------------------------------------*/


/* Internal functions ----------------------------------------------------------------------------*/

/**
 * @brief   Initializes the CDC media low layer.
 * @return  Result of the operation: USBD_OK if all operations are OK else USBD_FAIL.
 */
static int8_t USBD_CDC_Init(void) {
    USBD_CDC_SetTxBuffer(&self.usbdHandle, self.txBuffer, 0);
    USBD_CDC_SetRxBuffer(&self.usbdHandle, self.rxBuffer);
    return (0);
}


/**
 * @brief   DeInitializes the CDC media low layer
 * @return  Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
 */
static int8_t USBD_CDC_DeInit(void) {
    /* Add your deinitialization code here */
    return (0);
}


/**
 * @brief  Manage the CDC class requests.
 * @param[in]   cmd     Command code.
 * @param[in]   buffer  Buffer containing command data (request parameters).
 * @param[in]   length  Number of data to be sent (in bytes).
 * @return  Result of the operation: USBD_OK if all operations are OK else USBD_FAIL.
 */
static int8_t USBD_CDC_Control(uint8_t cmd, uint8_t *buffer, uint16_t length) {
    UNUSED(length);

    switch (cmd)
    {
    case CDC_SEND_ENCAPSULATED_COMMAND:
        /* Add your code here */
        break;
    case CDC_GET_ENCAPSULATED_RESPONSE:
        /* Add your code here */
        break;
    case CDC_SET_COMM_FEATURE:
        /* Add your code here */
        break;
    case CDC_GET_COMM_FEATURE:
        /* Add your code here */
        break;
    case CDC_CLEAR_COMM_FEATURE:
        /* Add your code here */
        break;
    case CDC_SET_LINE_CODING:
        linecoding.bitrate    = (uint32_t)(buffer[0] | (buffer[1] << 8) | \
                                          (buffer[2] << 16) | (buffer[3] << 24));
        linecoding.format     = buffer[4];
        linecoding.paritytype = buffer[5];
        linecoding.datatype   = buffer[6];
        /* Add your code here */
        break;
    case CDC_GET_LINE_CODING:
        buffer[0] = (uint8_t)(linecoding.bitrate);
        buffer[1] = (uint8_t)(linecoding.bitrate >> 8);
        buffer[2] = (uint8_t)(linecoding.bitrate >> 16);
        buffer[3] = (uint8_t)(linecoding.bitrate >> 24);
        buffer[4] = linecoding.format;
        buffer[5] = linecoding.paritytype;
        buffer[6] = linecoding.datatype;
        /* Add your code here */
        break;
    case CDC_SET_CONTROL_LINE_STATE:
        /* Add your code here */
        break;
    case CDC_SEND_BREAK:
        /* Add your code here */
        break;
    default:
        break;
    }
    return USBD_OK;
}


/**
 * @brief   Data received over USB OUT endpoint are sent over CDC interface through this function.
 *
 * @note    This function will issue a NAK packet on any OUT packet received on the USB endpoint
 *          until exiting this function. If you exit this function before transfer is complete on
 *          the CDC interface (ie. using DMA controller) it will result in receiving more data
 *          while previous ones are still not sent.
 * @param[in]   buffer      Buffer of data to be received.
 * @param[in]   lengthPtr   Number of data received (in bytes).
 * @retrun  Result of the operation: USBD_OK if all operations are OK else USBD_FAIL.
 */
static int8_t USBD_CDC_Receive(uint8_t *buffer, uint32_t *lengthPtr) {
    // verify the receiveCallback is valid; also confirm that the input pointers are valid
    if ((self.receiveCallback == NULL) || (buffer == NULL) ||
        (lengthPtr == NULL) || (*lengthPtr == 0)) {
        USBD_CDC_ReceivePacket(&self.usbdHandle);
        return USBD_FAIL;
    }
    self.receiveCallback(buffer, *buffer);
    USBD_CDC_ReceivePacket(&self.usbdHandle);
    return USBD_OK;
}


/**
 * @brief   Data transmitted callback.
 * @note    This function is IN transfer complete callback used to inform user that the submitted
 *          Data is successfully sent over USB.
 * @param[in]   buffer      Buffer of data that was transmitted.
 * @param[in]   lengthPtr   Number of data transmitted (in bytes).
 * @param[in]   epnum       TODO
 * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL.
 */
static int8_t USBD_CDC_TransmitCplt(uint8_t *buffer, uint32_t *lengthPtr, uint8_t epnum) {
    UNUSED(buffer);
    UNUSED(lengthPtr);
    UNUSED(epnum);

    return USBD_OK;
}


/**
 * @brief   Initialize the USB device.
 * @param[in]   pcdHandle   The handle to the USB PCD (peripheral control driver).
 * @param[in]   usbdHandle  The handle to the USB device.
 * @return  The USBD_StatusTypeDef; if successful, USBD_OK.
 */
static USBD_StatusTypeDef initUSBD(PCD_HandleTypeDef *const pcdHandle,
                                   USBD_HandleTypeDef *const usbdHandle) {
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
    USBD_StatusTypeDef status = HALStatusMap[HAL_PCD_Init(pcdHandle)];
    if (status != USBD_OK) {
        return status;
    }
    status = USBD_Init(usbdHandle, &CDC_Desc, 0);
    if (status != USBD_OK) {
        return status;
    }
    status = USBD_RegisterClass(usbdHandle, &USBD_CDC);
    if (status != USBD_OK) {
        return status;
    }
    status = USBD_CDC_RegisterInterface(usbdHandle, &USBD_CDC_Template_fops);
    if (status != USBD_OK) {
        return status;
    }
    status = USBD_Start(usbdHandle);
    if (status != USBD_OK) {
        return status;
    }
    return USBD_OK;
}


/* External function prototypes ------------------------------------------------------------------*/


/* External functions ----------------------------------------------------------------------------*/

/**
 * @brief   Initializes the USB CDC driver singleton.
 * @return  The USBD_StatusTypeDef; if successful, USBD_OK.
 */
USBD_StatusTypeDef USB_CDC_Init(void) {
    if (self.initialized == true) {
        return USBD_OK;
    }
    USBD_StatusTypeDef status = initUSBD(&self.pcdHandle, &self.usbdHandle);
    if (status != USBD_OK) {
        return status;
    }
    self.initialized = true;
    return USBD_OK;
}


/**
 * @brief   Deinitializes the USB CDC driver singleton.
 * @return  The USBD_StatusTypeDef; if successful, USBD_OK.
 */
USBD_StatusTypeDef USB_CDC_Deinit(void) {
    if (self.initialized == false) {
        return USBD_OK;
    }
    USBD_StatusTypeDef status = HALStatusMap[HAL_PCD_DeInit(&self.pcdHandle)];
    if (status != USBD_OK) {
        return status;
    }
    self.receiveCallback = NULL;
    self.initialized = false;
    return USBD_OK;
}


/**
 * @brief   Function to transmit data over the USB VCP.
 * @param[in]   buffer  Buffer of data to transmit.
 * @param[in]   length  Number of bytes to transmit.
 * @return  The USBD_StatusTypeDef; if successful, USBD_OK.
 */
USBD_StatusTypeDef USB_CDC_Transmit(uint8_t* const buffer, uint16_t length) {
    USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef*)self.usbdHandle.pClassData;
    if (hcdc->TxState != 0) {
        return USBD_BUSY;
    }
    USBD_CDC_SetTxBuffer(&self.usbdHandle, buffer, length);
    return USBD_CDC_TransmitPacket(&self.usbdHandle);
}


/**
 * @brief   Register a receive callback to handle data received over the USB VCP.
 * @note    Be sure the receive callback is a non-blocking function.
 * @param[in]   callback    The receive callback function to register.
 * @return  The USBD_StatusTypeDef; if successful, USBD_OK.
 */
USBD_StatusTypeDef USB_CDC_RegisterReceiveCallback(USB_CDC_ReceiveCallback_t const callback) {
    if (callback == NULL) {
        return USBD_EMEM;
    }
    if (self.receiveCallback != NULL) {
        return USBD_FAIL;
    }
    self.receiveCallback = callback;
    return USBD_OK;
}


/**
 * @brief   Deregisters the receive callback.
 * @return  The USBD_StatusTypeDef; if successful, USBD_OK.
 */
USBD_StatusTypeDef USB_CDC_DeregisterReceiveCallback(void) {
    if (self.receiveCallback == NULL) {
        return USBD_FAIL;
    }
    self.receiveCallback = NULL;
    return USBD_OK;
}


/**
  * @brief  Initializes the USB PCD MSP.
  * @param[in]  pcdHandle   The USB PCD handle.
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
  * @param[in]  pcdHandle   The USB PCD handle
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


/**
 * @brief   ISR (interrupt service routine) for the USB full-speed interface.
  * @note   USB_DRD_FS_IRQHandler is defined as a weak function in the HAL SDK. This implementation
  *         will be used over that weak implementation.
  * @note   The implementation is based on the STM32CubeIDE auto-generated implementation.
 */
void USB_DRD_FS_IRQHandler(void) {
    HAL_PCD_IRQHandler(&self.pcdHandle);
}

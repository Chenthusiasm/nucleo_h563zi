/**
  ******************************************************************************
  * @file    usbd_cdc_if_template.h
  * @author  MCD Application Team
  * @brief   Header for usbd_cdc_if_template.c file.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2015 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USBD_CDC_IF_TEMPLATE_H
#define __USBD_CDC_IF_TEMPLATE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stddef.h>

#include "usbd_cdc.h"

/* Exported types ------------------------------------------------------------*/

/**
 * @brief Type definition of the receive callback function.
 * @param[in]
 */
typedef size_t (*USBD_CDC_ReceiveCallback)(uint8_t* const Buf, size_t Len);


/* Exported constants --------------------------------------------------------*/

extern USBD_CDC_ItfTypeDef  USBD_CDC_Template_fops;

#define APP_RX_DATA_SIZE  512
#define APP_TX_DATA_SIZE  512

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

uint8_t USBD_CDC_Transmit(uint8_t* const Buf, uint16_t Len);
void USBD_CDC_RegisterReceiveCallback(USBD_CDC_ReceiveCallback const callback);

#ifdef __cplusplus
}
#endif

#endif /* __USBD_CDC_IF_TEMPLATE_H */


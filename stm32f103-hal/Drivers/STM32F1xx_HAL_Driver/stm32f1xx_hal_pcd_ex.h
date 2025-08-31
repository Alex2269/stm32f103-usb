// stm32f1xx_hal_pcd_ex.h

#ifndef STM32F1xx_HAL_PCD_EX_H
#define STM32F1xx_HAL_PCD_EX_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal_def.h"

#if defined (USB) || defined (USB_OTG_FS)

#if defined (USB)
HAL_StatusTypeDef  HAL_PCDEx_PMAConfig(PCD_HandleTypeDef *hpcd, uint16_t ep_addr,
                                       uint16_t ep_kind, uint32_t pmaadress);

void HAL_PCDEx_SetConnectionState(PCD_HandleTypeDef *hpcd, uint8_t state);
#endif /* defined (USB) */

void HAL_PCDEx_LPM_Callback(PCD_HandleTypeDef *hpcd, PCD_LPM_MsgTypeDef msg);
void HAL_PCDEx_BCD_Callback(PCD_HandleTypeDef *hpcd, PCD_BCD_MsgTypeDef msg);

#endif /* defined (USB) || defined (USB_OTG_FS) */

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* STM32F1xx_HAL_PCD_EX_H */

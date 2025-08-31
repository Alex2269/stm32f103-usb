// stm32f1xx_hal_pcd_ex.c

#include "stm32f1xx_hal.h"

#ifdef HAL_PCD_MODULE_ENABLED
#if defined (USB) || defined (USB_OTG_FS)

#if defined (USB)
HAL_StatusTypeDef HAL_PCDEx_PMAConfig(PCD_HandleTypeDef *hpcd, uint16_t ep_addr, uint16_t ep_kind, uint32_t pmaadress)
{
  PCD_EPTypeDef *ep;
  if ((0x80U & ep_addr) == 0x80U)
  {
    ep = &hpcd->IN_ep[ep_addr & EP_ADDR_MSK];
  }
  else
  {
    ep = &hpcd->OUT_ep[ep_addr];
  }
  if (ep_kind == PCD_SNG_BUF)
  {
    ep->doublebuffer = 0U;
    ep->pmaadress = (uint16_t)pmaadress;
  }
  #if (USE_USB_DOUBLE_BUFFER == 1U)
  else 
  {
    ep->doublebuffer = 1U;
    ep->pmaaddr0 = (uint16_t)(pmaadress & 0xFFFFU);
    ep->pmaaddr1 = (uint16_t)((pmaadress & 0xFFFF0000U) >> 16);
  }
  #endif 
  return HAL_OK;
}

__weak void HAL_PCDEx_SetConnectionState(PCD_HandleTypeDef *hpcd, uint8_t state)
{
  UNUSED(hpcd);
  UNUSED(state);
}

#endif 
__weak void HAL_PCDEx_LPM_Callback(PCD_HandleTypeDef *hpcd, PCD_LPM_MsgTypeDef msg)
{
  UNUSED(hpcd);
  UNUSED(msg);
}

__weak void HAL_PCDEx_BCD_Callback(PCD_HandleTypeDef *hpcd, PCD_BCD_MsgTypeDef msg)
{
  UNUSED(hpcd);
  UNUSED(msg);
}

#endif 
#endif 

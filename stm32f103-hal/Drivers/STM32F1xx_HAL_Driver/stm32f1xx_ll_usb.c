// stm32f1xx_ll_usb.c

#include "stm32f1xx_hal.h"
#if defined (HAL_PCD_MODULE_ENABLED) || defined (HAL_HCD_MODULE_ENABLED)
#if defined (USB) || defined (USB_OTG_FS)

#if defined (USB)
HAL_StatusTypeDef USB_CoreInit(USB_TypeDef *USBx, USB_CfgTypeDef cfg)
{
  UNUSED(USBx);
  UNUSED(cfg);
  return HAL_OK;
}

HAL_StatusTypeDef USB_EnableGlobalInt(USB_TypeDef *USBx)
{
  uint32_t winterruptmask;
  USBx->ISTR = 0U;
  winterruptmask = USB_CNTR_CTRM | USB_CNTR_WKUPM |
  USB_CNTR_SUSPM | USB_CNTR_ERRM |
  USB_CNTR_SOFM | USB_CNTR_ESOFM |
  USB_CNTR_RESETM;
  USBx->CNTR = (uint16_t)winterruptmask;
  return HAL_OK;
}

HAL_StatusTypeDef USB_DisableGlobalInt(USB_TypeDef *USBx)
{
  uint32_t winterruptmask;
  winterruptmask = USB_CNTR_CTRM | USB_CNTR_WKUPM |
  USB_CNTR_SUSPM | USB_CNTR_ERRM |
  USB_CNTR_SOFM | USB_CNTR_ESOFM |
  USB_CNTR_RESETM;
  USBx->CNTR &= (uint16_t)(~winterruptmask);
  return HAL_OK;
}

HAL_StatusTypeDef USB_SetCurrentMode(USB_TypeDef *USBx, USB_ModeTypeDef mode)
{
  UNUSED(USBx);
  UNUSED(mode);
  return HAL_OK;
}

HAL_StatusTypeDef USB_DevInit(USB_TypeDef *USBx, USB_CfgTypeDef cfg)
{
  UNUSED(cfg);
  USBx->CNTR = (uint16_t)USB_CNTR_FRES;
  USBx->CNTR = 0U;
  USBx->ISTR = 0U;
  USBx->BTABLE = BTABLE_ADDRESS;
  return HAL_OK;
}

HAL_StatusTypeDef USB_FlushTxFifo(USB_TypeDef const *USBx, uint32_t num)
{
  UNUSED(USBx);
  UNUSED(num);
  return HAL_OK;
}

HAL_StatusTypeDef USB_FlushRxFifo(USB_TypeDef const *USBx)
{
  UNUSED(USBx);
  return HAL_OK;
}

#if defined (HAL_PCD_MODULE_ENABLED)
HAL_StatusTypeDef USB_ActivateEndpoint(USB_TypeDef *USBx, USB_EPTypeDef *ep)
{
  HAL_StatusTypeDef ret = HAL_OK;
  uint16_t wEpRegVal;
  wEpRegVal = PCD_GET_ENDPOINT(USBx, ep->num) & USB_EP_T_MASK;
  switch (ep->type)
  {
    case EP_TYPE_CTRL:
    wEpRegVal |= USB_EP_CONTROL;
    break;
    case EP_TYPE_BULK:
    wEpRegVal |= USB_EP_BULK;
    break;
    case EP_TYPE_INTR:
    wEpRegVal |= USB_EP_INTERRUPT;
    break;
    case EP_TYPE_ISOC:
    wEpRegVal |= USB_EP_ISOCHRONOUS;
    break;
    default:
    ret = HAL_ERROR;
    break;
  }
  PCD_SET_ENDPOINT(USBx, ep->num, (wEpRegVal | USB_EP_CTR_RX | USB_EP_CTR_TX));
  PCD_SET_EP_ADDRESS(USBx, ep->num, ep->num);
  if (ep->doublebuffer == 0U)
  {
    if (ep->is_in != 0U)
    {
      PCD_SET_EP_TX_ADDRESS(USBx, ep->num, ep->pmaadress);
      PCD_CLEAR_TX_DTOG(USBx, ep->num);
      if (ep->type != EP_TYPE_ISOC)
      {
        PCD_SET_EP_TX_STATUS(USBx, ep->num, USB_EP_TX_NAK);
      }
      else
      {
        PCD_SET_EP_TX_STATUS(USBx, ep->num, USB_EP_TX_DIS);
      }
    }
    else
    {
      PCD_SET_EP_RX_ADDRESS(USBx, ep->num, ep->pmaadress);
      PCD_SET_EP_RX_CNT(USBx, ep->num, ep->maxpacket);
      PCD_CLEAR_RX_DTOG(USBx, ep->num);
      if (ep->num == 0U)
      {
        PCD_SET_EP_RX_STATUS(USBx, ep->num, USB_EP_RX_VALID);
      }
      else
      {
        PCD_SET_EP_RX_STATUS(USBx, ep->num, USB_EP_RX_NAK);
      }
    }
  }
  return ret;
}

HAL_StatusTypeDef USB_DeactivateEndpoint(USB_TypeDef *USBx, USB_EPTypeDef *ep)
{
  if (ep->doublebuffer == 0U)
  {
    if (ep->is_in != 0U)
    {
      PCD_CLEAR_TX_DTOG(USBx, ep->num);
      PCD_SET_EP_TX_STATUS(USBx, ep->num, USB_EP_TX_DIS);
    }
    else
    {
      PCD_CLEAR_RX_DTOG(USBx, ep->num);
      PCD_SET_EP_RX_STATUS(USBx, ep->num, USB_EP_RX_DIS);
    }
  }
  return HAL_OK;
}

HAL_StatusTypeDef USB_EPStartXfer(USB_TypeDef *USBx, USB_EPTypeDef *ep)
{
  uint32_t len;
  if (ep->is_in == 1U)
  {
    if (ep->xfer_len > ep->maxpacket)
    {
      len = ep->maxpacket;
    }
    else
    {
      len = ep->xfer_len;
    }
    if (ep->doublebuffer == 0U)
    {
      USB_WritePMA(USBx, ep->xfer_buff, ep->pmaadress, (uint16_t)len);
      PCD_SET_EP_TX_CNT(USBx, ep->num, len);
    }
    PCD_SET_EP_TX_STATUS(USBx, ep->num, USB_EP_TX_VALID);
  }
  else
  {
    if (ep->doublebuffer == 0U)
    {
      if (ep->xfer_len > ep->maxpacket)
      {
        len = ep->maxpacket;
        ep->xfer_len -= len;
      }
      else
      {
        len = ep->xfer_len;
        ep->xfer_len = 0U;
      }
      PCD_SET_EP_RX_CNT(USBx, ep->num, len);
    }
    PCD_SET_EP_RX_STATUS(USBx, ep->num, USB_EP_RX_VALID);
  }
  return HAL_OK;
}

HAL_StatusTypeDef USB_EPSetStall(USB_TypeDef *USBx, USB_EPTypeDef *ep)
{
  if (ep->is_in != 0U)
  {
    PCD_SET_EP_TX_STATUS(USBx, ep->num, USB_EP_TX_STALL);
  }
  else
  {
    PCD_SET_EP_RX_STATUS(USBx, ep->num, USB_EP_RX_STALL);
  }
  return HAL_OK;
}

HAL_StatusTypeDef USB_EPClearStall(USB_TypeDef *USBx, USB_EPTypeDef *ep)
{
  if (ep->doublebuffer == 0U)
  {
    if (ep->is_in != 0U)
    {
      PCD_CLEAR_TX_DTOG(USBx, ep->num);
      if (ep->type != EP_TYPE_ISOC)
      {
        PCD_SET_EP_TX_STATUS(USBx, ep->num, USB_EP_TX_NAK);
      }
    }
    else
    {
      PCD_CLEAR_RX_DTOG(USBx, ep->num);
      PCD_SET_EP_RX_STATUS(USBx, ep->num, USB_EP_RX_VALID);
    }
  }
  return HAL_OK;
}

HAL_StatusTypeDef USB_EPStopXfer(USB_TypeDef *USBx, USB_EPTypeDef *ep)
{
  if (ep->is_in == 1U)
  {
    if (ep->doublebuffer == 0U)
    {
      if (ep->type != EP_TYPE_ISOC)
      {
        PCD_SET_EP_TX_STATUS(USBx, ep->num, USB_EP_TX_NAK);
      }
      else
      {
        PCD_SET_EP_TX_STATUS(USBx, ep->num, USB_EP_TX_DIS);
      }
    }
  }
  else
  {
    if (ep->doublebuffer == 0U)
    {
      if (ep->type != EP_TYPE_ISOC)
      {
        PCD_SET_EP_RX_STATUS(USBx, ep->num, USB_EP_RX_NAK);
      }
      else
      {
        PCD_SET_EP_RX_STATUS(USBx, ep->num, USB_EP_RX_DIS);
      }
    }
  }
  return HAL_OK;
}

#endif
HAL_StatusTypeDef USB_StopDevice(USB_TypeDef *USBx)
{
  USBx->CNTR = (uint16_t)USB_CNTR_FRES;
  USBx->ISTR = 0U;
  USBx->CNTR = (uint16_t)(USB_CNTR_FRES | USB_CNTR_PDWN);
  return HAL_OK;
}

HAL_StatusTypeDef USB_SetDevAddress(USB_TypeDef *USBx, uint8_t address)
{
  if (address == 0U)
  {
    USBx->DADDR = (uint16_t)USB_DADDR_EF;
  }
  return HAL_OK;
}

HAL_StatusTypeDef USB_DevConnect(USB_TypeDef *USBx)
{
  UNUSED(USBx);
  return HAL_OK;
}

HAL_StatusTypeDef USB_DevDisconnect(USB_TypeDef *USBx)
{
  UNUSED(USBx);
  return HAL_OK;
}

uint32_t USB_ReadInterrupts(USB_TypeDef const *USBx)
{
  uint32_t tmpreg;
  tmpreg = USBx->ISTR;
  return tmpreg;
}

uint32_t USB_ReadDevAllOutEpInterrupt(USB_TypeDef *USBx)
{
  UNUSED(USBx);
  return (0);
}

uint32_t USB_ReadDevAllInEpInterrupt(USB_TypeDef *USBx)
{
  UNUSED(USBx);
  return (0);
}

uint32_t USB_ReadDevOutEPInterrupt(USB_TypeDef *USBx, uint8_t epnum)
{
  UNUSED(USBx);
  UNUSED(epnum);
  return (0);
}

uint32_t USB_ReadDevInEPInterrupt(USB_TypeDef *USBx, uint8_t epnum)
{
  UNUSED(USBx);
  UNUSED(epnum);
  return (0);
}

void USB_ClearInterrupts(USB_TypeDef *USBx, uint32_t interrupt)
{
  UNUSED(USBx);
  UNUSED(interrupt);
}

HAL_StatusTypeDef USB_EP0_OutStart(USB_TypeDef *USBx, uint8_t *psetup)
{
  UNUSED(USBx);
  UNUSED(psetup);
  return HAL_OK;
}

HAL_StatusTypeDef USB_ActivateRemoteWakeup(USB_TypeDef *USBx)
{
  USBx->CNTR |= (uint16_t)USB_CNTR_RESUME;
  return HAL_OK;
}

HAL_StatusTypeDef USB_DeActivateRemoteWakeup(USB_TypeDef *USBx)
{
  USBx->CNTR &= (uint16_t)(~USB_CNTR_RESUME);
  return HAL_OK;
}

void USB_WritePMA(USB_TypeDef const *USBx, uint8_t *pbUsrBuf, uint16_t wPMABufAddr, uint16_t wNBytes)
{
  uint32_t n = ((uint32_t)wNBytes + 1U) >> 1;
  uint32_t BaseAddr = (uint32_t)USBx;
  uint32_t count;
  uint16_t WrVal;
  __IO uint16_t *pdwVal;
  uint8_t *pBuf = pbUsrBuf;
  pdwVal = (__IO uint16_t *)(BaseAddr + 0x400U + ((uint32_t)wPMABufAddr * PMA_ACCESS));
  for (count = n; count != 0U; count--)
  {
    WrVal = pBuf[0];
    WrVal |= (uint16_t)pBuf[1] << 8;
    *pdwVal = (WrVal & 0xFFFFU);
    pdwVal++;
    #if PMA_ACCESS > 1U
    pdwVal++;
    #endif 
    pBuf++;
    pBuf++;
  }
}

void USB_ReadPMA(USB_TypeDef const *USBx, uint8_t *pbUsrBuf, uint16_t wPMABufAddr, uint16_t wNBytes)
{
  uint32_t n = (uint32_t)wNBytes >> 1;
  uint32_t BaseAddr = (uint32_t)USBx;
  uint32_t count;
  uint32_t RdVal;
  __IO uint16_t *pdwVal;
  uint8_t *pBuf = pbUsrBuf;
  pdwVal = (__IO uint16_t *)(BaseAddr + 0x400U + ((uint32_t)wPMABufAddr * PMA_ACCESS));
  for (count = n; count != 0U; count--)
  {
    RdVal = *(__IO uint16_t *)pdwVal;
    pdwVal++;
    *pBuf = (uint8_t)((RdVal >> 0) & 0xFFU);
    pBuf++;
    *pBuf = (uint8_t)((RdVal >> 8) & 0xFFU);
    pBuf++;
    #if PMA_ACCESS > 1U
    pdwVal++;
    #endif 
  }
  if ((wNBytes % 2U) != 0U)
  {
    RdVal = *pdwVal;
    *pBuf = (uint8_t)((RdVal >> 0) & 0xFFU);
  }
}

#endif 
#endif 
#endif 

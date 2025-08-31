
#include "stm32f1xx_hal.h"
#ifdef HAL_PCD_MODULE_ENABLED
#if defined (USB) || defined (USB_OTG_FS)
#define PCD_MIN(a, b) (((a) < (b)) ? (a) : (b))
#define PCD_MAX(a, b) (((a) > (b)) ? (a) : (b))

#if defined (USB)
static HAL_StatusTypeDef PCD_EP_ISR_Handler(PCD_HandleTypeDef *hpcd);
#endif

HAL_StatusTypeDef HAL_PCD_Init(PCD_HandleTypeDef *hpcd)
{
  uint8_t i;
  if (hpcd == NULL)
  {
    return HAL_ERROR;
  }
  assert_param(IS_PCD_ALL_INSTANCE(hpcd->Instance));
  if (hpcd->State == HAL_PCD_STATE_RESET)
  {
    hpcd->Lock = HAL_UNLOCKED;
    #if (USE_HAL_PCD_REGISTER_CALLBACKS == 1U)
    hpcd->SOFCallback = HAL_PCD_SOFCallback;
    hpcd->SetupStageCallback = HAL_PCD_SetupStageCallback;
    hpcd->ResetCallback = HAL_PCD_ResetCallback;
    hpcd->SuspendCallback = HAL_PCD_SuspendCallback;
    hpcd->ResumeCallback = HAL_PCD_ResumeCallback;
    hpcd->ConnectCallback = HAL_PCD_ConnectCallback;
    hpcd->DisconnectCallback = HAL_PCD_DisconnectCallback;
    hpcd->DataOutStageCallback = HAL_PCD_DataOutStageCallback;
    hpcd->DataInStageCallback = HAL_PCD_DataInStageCallback;
    hpcd->ISOOUTIncompleteCallback = HAL_PCD_ISOOUTIncompleteCallback;
    hpcd->ISOINIncompleteCallback = HAL_PCD_ISOINIncompleteCallback;
    if (hpcd->MspInitCallback == NULL)
    {
      hpcd->MspInitCallback = HAL_PCD_MspInit;
    }
    hpcd->MspInitCallback(hpcd);
    #else
    HAL_PCD_MspInit(hpcd);
    #endif
  }
  hpcd->State = HAL_PCD_STATE_BUSY;
  hpcd->Init.dma_enable = 0U;
  __HAL_PCD_DISABLE(hpcd);
  if (USB_CoreInit(hpcd->Instance, hpcd->Init) != HAL_OK)
  {
    hpcd->State = HAL_PCD_STATE_ERROR;
    return HAL_ERROR;
  }
  if (USB_SetCurrentMode(hpcd->Instance, USB_DEVICE_MODE) != HAL_OK)
  {
    hpcd->State = HAL_PCD_STATE_ERROR;
    return HAL_ERROR;
  }
  for (i = 0U; i < hpcd->Init.dev_endpoints; i++)
  {
    hpcd->IN_ep[i].is_in = 1U;
    hpcd->IN_ep[i].num = i;

    hpcd->IN_ep[i].type = EP_TYPE_CTRL;
    hpcd->IN_ep[i].maxpacket = 0U;
    hpcd->IN_ep[i].xfer_buff = 0U;
    hpcd->IN_ep[i].xfer_len = 0U;
  }
  for (i = 0U; i < hpcd->Init.dev_endpoints; i++)
  {
    hpcd->OUT_ep[i].is_in = 0U;
    hpcd->OUT_ep[i].num = i;
    hpcd->OUT_ep[i].type = EP_TYPE_CTRL;
    hpcd->OUT_ep[i].maxpacket = 0U;
    hpcd->OUT_ep[i].xfer_buff = 0U;
    hpcd->OUT_ep[i].xfer_len = 0U;
  }
  if (USB_DevInit(hpcd->Instance, hpcd->Init) != HAL_OK)
  {
    hpcd->State = HAL_PCD_STATE_ERROR;
    return HAL_ERROR;
  }
  hpcd->USB_Address = 0U;
  hpcd->State = HAL_PCD_STATE_READY;
  (void)USB_DevDisconnect(hpcd->Instance);
  return HAL_OK;
}

HAL_StatusTypeDef HAL_PCD_DeInit(PCD_HandleTypeDef *hpcd)
{
  if (hpcd == NULL)
  {
    return HAL_ERROR;
  }
  hpcd->State = HAL_PCD_STATE_BUSY;
  if (USB_StopDevice(hpcd->Instance) != HAL_OK)
  {
    return HAL_ERROR;
  }
  #if (USE_HAL_PCD_REGISTER_CALLBACKS == 1U)
  if (hpcd->MspDeInitCallback == NULL)
  {
    hpcd->MspDeInitCallback = HAL_PCD_MspDeInit;
  }
  hpcd->MspDeInitCallback(hpcd);
  #else
  HAL_PCD_MspDeInit(hpcd);
  #endif
  hpcd->State = HAL_PCD_STATE_RESET;
  return HAL_OK;
}

__weak void HAL_PCD_MspInit(PCD_HandleTypeDef *hpcd)
{
  UNUSED(hpcd);
}

__weak void HAL_PCD_MspDeInit(PCD_HandleTypeDef *hpcd)
{
  UNUSED(hpcd);
}

#if (USE_HAL_PCD_REGISTER_CALLBACKS == 1U)
HAL_StatusTypeDef HAL_PCD_RegisterCallback(PCD_HandleTypeDef *hpcd, HAL_PCD_CallbackIDTypeDef CallbackID, pPCD_CallbackTypeDef pCallback)
{
  HAL_StatusTypeDef status = HAL_OK;
  if (pCallback == NULL)
  {
    hpcd->ErrorCode |= HAL_PCD_ERROR_INVALID_CALLBACK;
    return HAL_ERROR;
  }
  __HAL_LOCK(hpcd);
  if (hpcd->State == HAL_PCD_STATE_READY)
  {
    switch (CallbackID)
    {
      case HAL_PCD_SOF_CB_ID :
      hpcd->SOFCallback = pCallback;
      break;
      case HAL_PCD_SETUPSTAGE_CB_ID :
      hpcd->SetupStageCallback = pCallback;
      break;
      case HAL_PCD_RESET_CB_ID :
      hpcd->ResetCallback = pCallback;
      break;
      case HAL_PCD_SUSPEND_CB_ID :
      hpcd->SuspendCallback = pCallback;
      break;
      case HAL_PCD_RESUME_CB_ID :
      hpcd->ResumeCallback = pCallback;
      break;
      case HAL_PCD_CONNECT_CB_ID :
      hpcd->ConnectCallback = pCallback;
      break;
      case HAL_PCD_DISCONNECT_CB_ID :
      hpcd->DisconnectCallback = pCallback;
      break;
      case HAL_PCD_MSPINIT_CB_ID :
      hpcd->MspInitCallback = pCallback;
      break;
      case HAL_PCD_MSPDEINIT_CB_ID :
      hpcd->MspDeInitCallback = pCallback;
      break;
      default :
      hpcd->ErrorCode |= HAL_PCD_ERROR_INVALID_CALLBACK;
      status = HAL_ERROR;
      break;
    }
  }
  else if (hpcd->State == HAL_PCD_STATE_RESET)
  {
    switch (CallbackID)
    {
      case HAL_PCD_MSPINIT_CB_ID :
      hpcd->MspInitCallback = pCallback;
      break;
      case HAL_PCD_MSPDEINIT_CB_ID :
      hpcd->MspDeInitCallback = pCallback;
      break;
      default :
      hpcd->ErrorCode |= HAL_PCD_ERROR_INVALID_CALLBACK;
      status = HAL_ERROR;
      break;
    }
  }
  else
  {
    hpcd->ErrorCode |= HAL_PCD_ERROR_INVALID_CALLBACK;
    status = HAL_ERROR;
  }
  __HAL_UNLOCK(hpcd);
  return status;
}

HAL_StatusTypeDef HAL_PCD_UnRegisterCallback(PCD_HandleTypeDef *hpcd, HAL_PCD_CallbackIDTypeDef CallbackID)
{
  HAL_StatusTypeDef status = HAL_OK;
  __HAL_LOCK(hpcd);
  if (hpcd->State == HAL_PCD_STATE_READY)
  {
    switch (CallbackID)
    {
      case HAL_PCD_SOF_CB_ID :
      hpcd->SOFCallback = HAL_PCD_SOFCallback;
      break;
      case HAL_PCD_SETUPSTAGE_CB_ID :
      hpcd->SetupStageCallback = HAL_PCD_SetupStageCallback;
      break;
      case HAL_PCD_RESET_CB_ID :
      hpcd->ResetCallback = HAL_PCD_ResetCallback;
      break;
      case HAL_PCD_SUSPEND_CB_ID :
      hpcd->SuspendCallback = HAL_PCD_SuspendCallback;
      break;
      case HAL_PCD_RESUME_CB_ID :
      hpcd->ResumeCallback = HAL_PCD_ResumeCallback;
      break;
      case HAL_PCD_CONNECT_CB_ID :
      hpcd->ConnectCallback = HAL_PCD_ConnectCallback;
      break;
      case HAL_PCD_DISCONNECT_CB_ID :
      hpcd->DisconnectCallback = HAL_PCD_DisconnectCallback;
      break;
      case HAL_PCD_MSPINIT_CB_ID :
      hpcd->MspInitCallback = HAL_PCD_MspInit;
      break;
      case HAL_PCD_MSPDEINIT_CB_ID :
      hpcd->MspDeInitCallback = HAL_PCD_MspDeInit;
      break;
      default :
      hpcd->ErrorCode |= HAL_PCD_ERROR_INVALID_CALLBACK;
      status = HAL_ERROR;
      break;
    }
  }
  else if (hpcd->State == HAL_PCD_STATE_RESET)
  {
    switch (CallbackID)
    {
      case HAL_PCD_MSPINIT_CB_ID :
      hpcd->MspInitCallback = HAL_PCD_MspInit;
      break;
      case HAL_PCD_MSPDEINIT_CB_ID :
      hpcd->MspDeInitCallback = HAL_PCD_MspDeInit;
      break;
      default :
      hpcd->ErrorCode |= HAL_PCD_ERROR_INVALID_CALLBACK;
      status = HAL_ERROR;
      break;
    }
  }
  else
  {
    hpcd->ErrorCode |= HAL_PCD_ERROR_INVALID_CALLBACK;
    status = HAL_ERROR;
  }
  __HAL_UNLOCK(hpcd);
  return status;
}

HAL_StatusTypeDef HAL_PCD_RegisterDataOutStageCallback(PCD_HandleTypeDef *hpcd, pPCD_DataOutStageCallbackTypeDef pCallback)
{
  HAL_StatusTypeDef status = HAL_OK;
  if (pCallback == NULL)
  {
    hpcd->ErrorCode |= HAL_PCD_ERROR_INVALID_CALLBACK;
    return HAL_ERROR;
  }
  __HAL_LOCK(hpcd);
  if (hpcd->State == HAL_PCD_STATE_READY)
  {
    hpcd->DataOutStageCallback = pCallback;
  }
  else
  {
    hpcd->ErrorCode |= HAL_PCD_ERROR_INVALID_CALLBACK;
    status = HAL_ERROR;
  }
  __HAL_UNLOCK(hpcd);
  return status;
}

HAL_StatusTypeDef HAL_PCD_UnRegisterDataOutStageCallback(PCD_HandleTypeDef *hpcd)
{
  HAL_StatusTypeDef status = HAL_OK;
  __HAL_LOCK(hpcd);
  if (hpcd->State == HAL_PCD_STATE_READY)
  {
    hpcd->DataOutStageCallback = HAL_PCD_DataOutStageCallback;
  }
  else
  {
    hpcd->ErrorCode |= HAL_PCD_ERROR_INVALID_CALLBACK;
    status = HAL_ERROR;
  }
  __HAL_UNLOCK(hpcd);
  return status;
}

HAL_StatusTypeDef HAL_PCD_RegisterDataInStageCallback(PCD_HandleTypeDef *hpcd, pPCD_DataInStageCallbackTypeDef pCallback)
{
  HAL_StatusTypeDef status = HAL_OK;
  if (pCallback == NULL)
  {
    hpcd->ErrorCode |= HAL_PCD_ERROR_INVALID_CALLBACK;
    return HAL_ERROR;
  }
  __HAL_LOCK(hpcd);
  if (hpcd->State == HAL_PCD_STATE_READY)
  {
    hpcd->DataInStageCallback = pCallback;
  }
  else
  {
    hpcd->ErrorCode |= HAL_PCD_ERROR_INVALID_CALLBACK;
    status = HAL_ERROR;
  }
  __HAL_UNLOCK(hpcd);
  return status;
}

HAL_StatusTypeDef HAL_PCD_UnRegisterDataInStageCallback(PCD_HandleTypeDef *hpcd)
{
  HAL_StatusTypeDef status = HAL_OK;
  __HAL_LOCK(hpcd);
  if (hpcd->State == HAL_PCD_STATE_READY)
  {
    hpcd->DataInStageCallback = HAL_PCD_DataInStageCallback;
  }
  else
  {
    hpcd->ErrorCode |= HAL_PCD_ERROR_INVALID_CALLBACK;
    status = HAL_ERROR;
  }
  __HAL_UNLOCK(hpcd);
  return status;
}

HAL_StatusTypeDef HAL_PCD_RegisterIsoOutIncpltCallback(PCD_HandleTypeDef *hpcd, pPCD_IsoOutIncpltCallbackTypeDef pCallback)
{
  HAL_StatusTypeDef status = HAL_OK;
  if (pCallback == NULL)
  {
    hpcd->ErrorCode |= HAL_PCD_ERROR_INVALID_CALLBACK;
    return HAL_ERROR;
  }
  __HAL_LOCK(hpcd);
  if (hpcd->State == HAL_PCD_STATE_READY)
  {
    hpcd->ISOOUTIncompleteCallback = pCallback;
  }
  else
  {
    hpcd->ErrorCode |= HAL_PCD_ERROR_INVALID_CALLBACK;
    status = HAL_ERROR;
  }
  __HAL_UNLOCK(hpcd);
  return status;
}

HAL_StatusTypeDef HAL_PCD_UnRegisterIsoOutIncpltCallback(PCD_HandleTypeDef *hpcd)
{
  HAL_StatusTypeDef status = HAL_OK;
  __HAL_LOCK(hpcd);
  if (hpcd->State == HAL_PCD_STATE_READY)
  {
    hpcd->ISOOUTIncompleteCallback = HAL_PCD_ISOOUTIncompleteCallback;
  }
  else
  {
    hpcd->ErrorCode |= HAL_PCD_ERROR_INVALID_CALLBACK;
    status = HAL_ERROR;
  }
  __HAL_UNLOCK(hpcd);
  return status;
}

HAL_StatusTypeDef HAL_PCD_RegisterIsoInIncpltCallback(PCD_HandleTypeDef *hpcd, pPCD_IsoInIncpltCallbackTypeDef pCallback)
{
  HAL_StatusTypeDef status = HAL_OK;
  if (pCallback == NULL)
  {
    hpcd->ErrorCode |= HAL_PCD_ERROR_INVALID_CALLBACK;
    return HAL_ERROR;
  }
  __HAL_LOCK(hpcd);
  if (hpcd->State == HAL_PCD_STATE_READY)
  {
    hpcd->ISOINIncompleteCallback = pCallback;
  }
  else
  {
    hpcd->ErrorCode |= HAL_PCD_ERROR_INVALID_CALLBACK;
    status = HAL_ERROR;
  }
  __HAL_UNLOCK(hpcd);
  return status;
}

HAL_StatusTypeDef HAL_PCD_UnRegisterIsoInIncpltCallback(PCD_HandleTypeDef *hpcd)
{
  HAL_StatusTypeDef status = HAL_OK;
  __HAL_LOCK(hpcd);
  if (hpcd->State == HAL_PCD_STATE_READY)
  {
    hpcd->ISOINIncompleteCallback = HAL_PCD_ISOINIncompleteCallback;
  }
  else
  {
    hpcd->ErrorCode |= HAL_PCD_ERROR_INVALID_CALLBACK;
    status = HAL_ERROR;
  }
  __HAL_UNLOCK(hpcd);
  return status;
}

#endif
HAL_StatusTypeDef HAL_PCD_Start(PCD_HandleTypeDef *hpcd)
{
  __HAL_LOCK(hpcd);
  __HAL_PCD_ENABLE(hpcd);
  #if defined (USB)
  HAL_PCDEx_SetConnectionState(hpcd, 1U);
  #endif
  (void)USB_DevConnect(hpcd->Instance);
  __HAL_UNLOCK(hpcd);
  return HAL_OK;
}

HAL_StatusTypeDef HAL_PCD_Stop(PCD_HandleTypeDef *hpcd)
{
  __HAL_LOCK(hpcd);
  __HAL_PCD_DISABLE(hpcd);
  #if defined (USB)
  HAL_PCDEx_SetConnectionState(hpcd, 0U);
  #endif
  (void)USB_DevDisconnect(hpcd->Instance);

  __HAL_UNLOCK(hpcd);
  return HAL_OK;
}

#if defined (USB)
void HAL_PCD_IRQHandler(PCD_HandleTypeDef *hpcd)
{
  uint32_t wIstr = USB_ReadInterrupts(hpcd->Instance);
  uint16_t store_ep[8];
  uint8_t i;
  if ((wIstr & USB_ISTR_CTR) == USB_ISTR_CTR)
  {
    (void)PCD_EP_ISR_Handler(hpcd);
    return;
  }
  if ((wIstr & USB_ISTR_RESET) == USB_ISTR_RESET)
  {
    __HAL_PCD_CLEAR_FLAG(hpcd, USB_ISTR_RESET);
    #if (USE_HAL_PCD_REGISTER_CALLBACKS == 1U)
    hpcd->ResetCallback(hpcd);
    #else
    HAL_PCD_ResetCallback(hpcd);
    #endif
    (void)HAL_PCD_SetAddress(hpcd, 0U);
    return;
  }
  if ((wIstr & USB_ISTR_PMAOVR) == USB_ISTR_PMAOVR)
  {
    __HAL_PCD_CLEAR_FLAG(hpcd, USB_ISTR_PMAOVR);
    return;
  }
  if ((wIstr & USB_ISTR_ERR) == USB_ISTR_ERR)
  {
    __HAL_PCD_CLEAR_FLAG(hpcd, USB_ISTR_ERR);
    return;
  }
  if ((wIstr & USB_ISTR_WKUP) == USB_ISTR_WKUP)
  {
    hpcd->Instance->CNTR &= (uint16_t) ~(USB_CNTR_LP_MODE);
    hpcd->Instance->CNTR &= (uint16_t) ~(USB_CNTR_FSUSP);
    #if (USE_HAL_PCD_REGISTER_CALLBACKS == 1U)
    hpcd->ResumeCallback(hpcd);
    #else
    HAL_PCD_ResumeCallback(hpcd);
    #endif
    __HAL_PCD_CLEAR_FLAG(hpcd, USB_ISTR_WKUP);
    return;
  }
  if ((wIstr & USB_ISTR_SUSP) == USB_ISTR_SUSP)
  {
    for (i = 0U; i < 8U; i++)
    {
      store_ep[i] = PCD_GET_ENDPOINT(hpcd->Instance, i);
    }
    hpcd->Instance->CNTR |= (uint16_t)(USB_CNTR_FRES);
    hpcd->Instance->CNTR &= (uint16_t)(~USB_CNTR_FRES);
    while ((hpcd->Instance->ISTR & USB_ISTR_RESET) == 0U)
    {
    }
    __HAL_PCD_CLEAR_FLAG(hpcd, USB_ISTR_RESET);
    for (i = 0U; i < 8U; i++)
    {
      PCD_SET_ENDPOINT(hpcd->Instance, i, store_ep[i]);
    }
    hpcd->Instance->CNTR |= (uint16_t)USB_CNTR_FSUSP;
    __HAL_PCD_CLEAR_FLAG(hpcd, USB_ISTR_SUSP);
    hpcd->Instance->CNTR |= (uint16_t)USB_CNTR_LP_MODE;
    #if (USE_HAL_PCD_REGISTER_CALLBACKS == 1U)
    hpcd->SuspendCallback(hpcd);
    #else
    HAL_PCD_SuspendCallback(hpcd);
    #endif
    return;
  }
  if ((wIstr & USB_ISTR_SOF) == USB_ISTR_SOF)
  {
    __HAL_PCD_CLEAR_FLAG(hpcd, USB_ISTR_SOF);
    #if (USE_HAL_PCD_REGISTER_CALLBACKS == 1U)
    hpcd->SOFCallback(hpcd);
    #else
    HAL_PCD_SOFCallback(hpcd);
    #endif
    return;
  }
  if ((wIstr & USB_ISTR_ESOF) == USB_ISTR_ESOF)
  {
    __HAL_PCD_CLEAR_FLAG(hpcd, USB_ISTR_ESOF);
    return;
  }
}

void HAL_PCD_WKUP_IRQHandler(PCD_HandleTypeDef *hpcd)
{
  __HAL_USB_WAKEUP_EXTI_CLEAR_FLAG();
}

#endif
__weak void HAL_PCD_DataOutStageCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
{
  UNUSED(hpcd);
  UNUSED(epnum);
}

__weak void HAL_PCD_DataInStageCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
{
  UNUSED(hpcd);
  UNUSED(epnum);
}

__weak void HAL_PCD_SetupStageCallback(PCD_HandleTypeDef *hpcd)
{
  UNUSED(hpcd);
}

__weak void HAL_PCD_SOFCallback(PCD_HandleTypeDef *hpcd)
{
  UNUSED(hpcd);
}

__weak void HAL_PCD_ResetCallback(PCD_HandleTypeDef *hpcd)
{
  UNUSED(hpcd);
}

__weak void HAL_PCD_SuspendCallback(PCD_HandleTypeDef *hpcd)
{
  UNUSED(hpcd);
}

__weak void HAL_PCD_ResumeCallback(PCD_HandleTypeDef *hpcd)
{
  UNUSED(hpcd);
}

__weak void HAL_PCD_ISOOUTIncompleteCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
{
  UNUSED(hpcd);
  UNUSED(epnum);
}

__weak void HAL_PCD_ISOINIncompleteCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
{
  UNUSED(hpcd);
  UNUSED(epnum);
}

__weak void HAL_PCD_ConnectCallback(PCD_HandleTypeDef *hpcd)
{
  UNUSED(hpcd);
}

__weak void HAL_PCD_DisconnectCallback(PCD_HandleTypeDef *hpcd)
{
  UNUSED(hpcd);
}

HAL_StatusTypeDef HAL_PCD_DevConnect(PCD_HandleTypeDef *hpcd)
{
  __HAL_LOCK(hpcd);
  #if defined (USB)
  HAL_PCDEx_SetConnectionState(hpcd, 1U);
  #endif
  (void)USB_DevConnect(hpcd->Instance);
  __HAL_UNLOCK(hpcd);
  return HAL_OK;
}

HAL_StatusTypeDef HAL_PCD_DevDisconnect(PCD_HandleTypeDef *hpcd)
{
  __HAL_LOCK(hpcd);
  #if defined (USB)
  HAL_PCDEx_SetConnectionState(hpcd, 0U);
  #endif
  (void)USB_DevDisconnect(hpcd->Instance);
  __HAL_UNLOCK(hpcd);
  return HAL_OK;
}

HAL_StatusTypeDef HAL_PCD_SetAddress(PCD_HandleTypeDef *hpcd, uint8_t address)
{
  __HAL_LOCK(hpcd);
  hpcd->USB_Address = address;
  (void)USB_SetDevAddress(hpcd->Instance, address);
  __HAL_UNLOCK(hpcd);
  return HAL_OK;
}

HAL_StatusTypeDef HAL_PCD_EP_Open(PCD_HandleTypeDef *hpcd, uint8_t ep_addr, uint16_t ep_mps, uint8_t ep_type)
{
  HAL_StatusTypeDef ret = HAL_OK;
  PCD_EPTypeDef *ep;
  if ((ep_addr & 0x80U) == 0x80U)
  {
    ep = &hpcd->IN_ep[ep_addr & EP_ADDR_MSK];
    ep->is_in = 1U;
  }
  else
  {
    ep = &hpcd->OUT_ep[ep_addr & EP_ADDR_MSK];
    ep->is_in = 0U;
  }
  ep->num = ep_addr & EP_ADDR_MSK;
  ep->maxpacket = ep_mps;
  ep->type = ep_type;

  if (ep_type == EP_TYPE_BULK)
  {
    ep->data_pid_start = 0U;
  }
  __HAL_LOCK(hpcd);
  (void)USB_ActivateEndpoint(hpcd->Instance, ep);
  __HAL_UNLOCK(hpcd);
  return ret;
}

HAL_StatusTypeDef HAL_PCD_EP_Close(PCD_HandleTypeDef *hpcd, uint8_t ep_addr)
{
  PCD_EPTypeDef *ep;
  if ((ep_addr & 0x80U) == 0x80U)
  {
    ep = &hpcd->IN_ep[ep_addr & EP_ADDR_MSK];
    ep->is_in = 1U;
  }
  else
  {
    ep = &hpcd->OUT_ep[ep_addr & EP_ADDR_MSK];
    ep->is_in = 0U;
  }
  ep->num = ep_addr & EP_ADDR_MSK;
  __HAL_LOCK(hpcd);
  (void)USB_DeactivateEndpoint(hpcd->Instance, ep);
  __HAL_UNLOCK(hpcd);
  return HAL_OK;
}

HAL_StatusTypeDef HAL_PCD_EP_Receive(PCD_HandleTypeDef *hpcd, uint8_t ep_addr, uint8_t *pBuf, uint32_t len)
{
  PCD_EPTypeDef *ep;
  ep = &hpcd->OUT_ep[ep_addr & EP_ADDR_MSK];
  ep->xfer_buff = pBuf;
  ep->xfer_len = len;
  ep->xfer_count = 0U;
  ep->is_in = 0U;
  ep->num = ep_addr & EP_ADDR_MSK;
  (void)USB_EPStartXfer(hpcd->Instance, ep);
  return HAL_OK;
}

uint32_t HAL_PCD_EP_GetRxCount(PCD_HandleTypeDef const *hpcd, uint8_t ep_addr)
{
  return hpcd->OUT_ep[ep_addr & EP_ADDR_MSK].xfer_count;
}

HAL_StatusTypeDef HAL_PCD_EP_Transmit(PCD_HandleTypeDef *hpcd, uint8_t ep_addr, uint8_t *pBuf, uint32_t len)
{
  PCD_EPTypeDef *ep;
  ep = &hpcd->IN_ep[ep_addr & EP_ADDR_MSK];
  ep->xfer_buff = pBuf;
  ep->xfer_len = len;
  #if defined (USB)
  ep->xfer_fill_db = 1U;
  ep->xfer_len_db = len;
  #endif
  ep->xfer_count = 0U;
  ep->is_in = 1U;
  ep->num = ep_addr & EP_ADDR_MSK;
  (void)USB_EPStartXfer(hpcd->Instance, ep);
  return HAL_OK;
}

HAL_StatusTypeDef HAL_PCD_EP_SetStall(PCD_HandleTypeDef *hpcd, uint8_t ep_addr)
{
  PCD_EPTypeDef *ep;
  if (((uint32_t)ep_addr & EP_ADDR_MSK) > hpcd->Init.dev_endpoints)
  {
    return HAL_ERROR;
  }
  if ((0x80U & ep_addr) == 0x80U)
  {
    ep = &hpcd->IN_ep[ep_addr & EP_ADDR_MSK];
    ep->is_in = 1U;
  }
  else
  {
    ep = &hpcd->OUT_ep[ep_addr];
    ep->is_in = 0U;
  }
  ep->is_stall = 1U;
  ep->num = ep_addr & EP_ADDR_MSK;
  __HAL_LOCK(hpcd);
  (void)USB_EPSetStall(hpcd->Instance, ep);
  if ((ep_addr & EP_ADDR_MSK) == 0U)
  {
    (void)USB_EP0_OutStart(hpcd->Instance, (uint8_t *)hpcd->Setup);
  }
  __HAL_UNLOCK(hpcd);
  return HAL_OK;
}

HAL_StatusTypeDef HAL_PCD_EP_ClrStall(PCD_HandleTypeDef *hpcd, uint8_t ep_addr)
{
  PCD_EPTypeDef *ep;
  if (((uint32_t)ep_addr & 0x0FU) > hpcd->Init.dev_endpoints)
  {
    return HAL_ERROR;
  }
  if ((0x80U & ep_addr) == 0x80U)
  {
    ep = &hpcd->IN_ep[ep_addr & EP_ADDR_MSK];
    ep->is_in = 1U;
  }
  else
  {
    ep = &hpcd->OUT_ep[ep_addr & EP_ADDR_MSK];
    ep->is_in = 0U;
  }
  ep->is_stall = 0U;
  ep->num = ep_addr & EP_ADDR_MSK;
  __HAL_LOCK(hpcd);
  (void)USB_EPClearStall(hpcd->Instance, ep);
  __HAL_UNLOCK(hpcd);
  return HAL_OK;
}

HAL_StatusTypeDef HAL_PCD_EP_Abort(PCD_HandleTypeDef *hpcd, uint8_t ep_addr)
{
  HAL_StatusTypeDef ret;
  PCD_EPTypeDef *ep;
  if ((0x80U & ep_addr) == 0x80U)
  {
    ep = &hpcd->IN_ep[ep_addr & EP_ADDR_MSK];
  }
  else
  {
    ep = &hpcd->OUT_ep[ep_addr & EP_ADDR_MSK];
  }
  ret = USB_EPStopXfer(hpcd->Instance, ep);
  return ret;
}

HAL_StatusTypeDef HAL_PCD_EP_Flush(PCD_HandleTypeDef *hpcd, uint8_t ep_addr)
{
  __HAL_LOCK(hpcd);
  if ((ep_addr & 0x80U) == 0x80U)
  {
    (void)USB_FlushTxFifo(hpcd->Instance, (uint32_t)ep_addr & EP_ADDR_MSK);
  }
  else
  {
    (void)USB_FlushRxFifo(hpcd->Instance);
  }
  __HAL_UNLOCK(hpcd);
  return HAL_OK;
}

HAL_StatusTypeDef HAL_PCD_ActivateRemoteWakeup(PCD_HandleTypeDef *hpcd)
{
  return (USB_ActivateRemoteWakeup(hpcd->Instance));
}

HAL_StatusTypeDef HAL_PCD_DeActivateRemoteWakeup(PCD_HandleTypeDef *hpcd)
{
  return (USB_DeActivateRemoteWakeup(hpcd->Instance));
}

PCD_StateTypeDef HAL_PCD_GetState(PCD_HandleTypeDef const *hpcd)
{
  return hpcd->State;
}

#if defined (USB)
static HAL_StatusTypeDef PCD_EP_ISR_Handler(PCD_HandleTypeDef *hpcd)
{
  PCD_EPTypeDef *ep;
  uint16_t count;
  uint16_t wIstr;
  uint16_t wEPVal;
  uint16_t TxPctSize;
  uint8_t epindex;
  #if (USE_USB_DOUBLE_BUFFER != 1U)
  count = 0U;
  #endif
  while ((hpcd->Instance->ISTR & USB_ISTR_CTR) != 0U)
  {
    wIstr = hpcd->Instance->ISTR;
    epindex = (uint8_t)(wIstr & USB_ISTR_EP_ID);
    if (epindex == 0U)
    {
      if ((wIstr & USB_ISTR_DIR) == 0U)
      {
        PCD_CLEAR_TX_EP_CTR(hpcd->Instance, PCD_ENDP0);
        ep = &hpcd->IN_ep[0];
        ep->xfer_count = PCD_GET_EP_TX_CNT(hpcd->Instance, ep->num);
        ep->xfer_buff += ep->xfer_count;
        #if (USE_HAL_PCD_REGISTER_CALLBACKS == 1U)
        hpcd->DataInStageCallback(hpcd, 0U);
        #else
        HAL_PCD_DataInStageCallback(hpcd, 0U);
        #endif
        if ((hpcd->USB_Address > 0U) && (ep->xfer_len == 0U))
        {
          hpcd->Instance->DADDR = ((uint16_t)hpcd->USB_Address | USB_DADDR_EF);
          hpcd->USB_Address = 0U;
        }
      }
      else
      {
        ep = &hpcd->OUT_ep[0];
        wEPVal = PCD_GET_ENDPOINT(hpcd->Instance, PCD_ENDP0);
        if ((wEPVal & USB_EP_SETUP) != 0U)
        {
          ep->xfer_count = PCD_GET_EP_RX_CNT(hpcd->Instance, ep->num);
          USB_ReadPMA(hpcd->Instance, (uint8_t *)hpcd->Setup, ep->pmaadress, (uint16_t)ep->xfer_count);
          PCD_CLEAR_RX_EP_CTR(hpcd->Instance, PCD_ENDP0);
          #if (USE_HAL_PCD_REGISTER_CALLBACKS == 1U)
          hpcd->SetupStageCallback(hpcd);
          #else
          HAL_PCD_SetupStageCallback(hpcd);
          #endif
        }
        else if ((wEPVal & USB_EP_CTR_RX) != 0U)
        {
          PCD_CLEAR_RX_EP_CTR(hpcd->Instance, PCD_ENDP0);
          ep->xfer_count = PCD_GET_EP_RX_CNT(hpcd->Instance, ep->num);
          if ((ep->xfer_count != 0U) && (ep->xfer_buff != 0U))
          {
            USB_ReadPMA(hpcd->Instance, ep->xfer_buff, ep->pmaadress, (uint16_t)ep->xfer_count);
            ep->xfer_buff += ep->xfer_count;
            #if (USE_HAL_PCD_REGISTER_CALLBACKS == 1U)
            hpcd->DataOutStageCallback(hpcd, 0U);
            #else
            HAL_PCD_DataOutStageCallback(hpcd, 0U);
            #endif
          }
          wEPVal = (uint16_t)PCD_GET_ENDPOINT(hpcd->Instance, PCD_ENDP0);
          if (((wEPVal & USB_EP_SETUP) == 0U) && ((wEPVal & USB_EP_RX_STRX) != USB_EP_RX_VALID))
          {
            PCD_SET_EP_RX_CNT(hpcd->Instance, PCD_ENDP0, ep->maxpacket);
            PCD_SET_EP_RX_STATUS(hpcd->Instance, PCD_ENDP0, USB_EP_RX_VALID);
          }
        }
      }
    }
    else
    {
      wEPVal = PCD_GET_ENDPOINT(hpcd->Instance, epindex);
      if ((wEPVal & USB_EP_CTR_RX) != 0U)
      {
        PCD_CLEAR_RX_EP_CTR(hpcd->Instance, epindex);
        ep = &hpcd->OUT_ep[epindex];
        if (ep->doublebuffer == 0U)
        {
          count = (uint16_t)PCD_GET_EP_RX_CNT(hpcd->Instance, ep->num);
          if (count != 0U)
          {
            USB_ReadPMA(hpcd->Instance, ep->xfer_buff, ep->pmaadress, count);
          }
        }
        ep->xfer_count += count;
        ep->xfer_buff += count;
        if ((ep->xfer_len == 0U) || (count < ep->maxpacket))
        {
          #if (USE_HAL_PCD_REGISTER_CALLBACKS == 1U)
          hpcd->DataOutStageCallback(hpcd, ep->num);
          #else
          HAL_PCD_DataOutStageCallback(hpcd, ep->num);
          #endif
        }
        else
        {
          (void)USB_EPStartXfer(hpcd->Instance, ep);
        }
      }
      if ((wEPVal & USB_EP_CTR_TX) != 0U)
      {
        ep = &hpcd->IN_ep[epindex];
        PCD_CLEAR_TX_EP_CTR(hpcd->Instance, epindex);
        if (ep->type == EP_TYPE_ISOC)
        {
          ep->xfer_len = 0U;
          #if (USE_HAL_PCD_REGISTER_CALLBACKS == 1U)
          hpcd->DataInStageCallback(hpcd, ep->num);
          #else
          HAL_PCD_DataInStageCallback(hpcd, ep->num);
          #endif
        }
        else
        {
          if ((wEPVal & USB_EP_KIND) == 0U)
          {
            TxPctSize = (uint16_t)PCD_GET_EP_TX_CNT(hpcd->Instance, ep->num);
            if (ep->xfer_len > TxPctSize)
            {
              ep->xfer_len -= TxPctSize;
            }
            else
            {
              ep->xfer_len = 0U;
            }
            if (ep->xfer_len == 0U)
            {
              #if (USE_HAL_PCD_REGISTER_CALLBACKS == 1U)
              hpcd->DataInStageCallback(hpcd, ep->num);
              #else
              HAL_PCD_DataInStageCallback(hpcd, ep->num);
              #endif
            }
            else
            {
              ep->xfer_buff += TxPctSize;
              ep->xfer_count += TxPctSize;
              (void)USB_EPStartXfer(hpcd->Instance, ep);
            }
          }
        }
      }
    }
  }
  return HAL_OK;
}

#endif
#endif
#endif

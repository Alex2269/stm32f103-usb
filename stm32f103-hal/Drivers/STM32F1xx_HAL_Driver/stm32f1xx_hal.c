
#include "stm32f1xx_hal.h"

#ifdef HAL_MODULE_ENABLED
#define __STM32F1xx_HAL_VERSION_MAIN (0x01U) 
#define __STM32F1xx_HAL_VERSION_SUB1 (0x01U) 
#define __STM32F1xx_HAL_VERSION_SUB2 (0x0AU) 
#define __STM32F1xx_HAL_VERSION_RC (0x00U) 

#define __STM32F1xx_HAL_VERSION ((__STM32F1xx_HAL_VERSION_MAIN << 24)\
|(__STM32F1xx_HAL_VERSION_SUB1 << 16)\
|(__STM32F1xx_HAL_VERSION_SUB2 << 8 )\
|(__STM32F1xx_HAL_VERSION_RC))

#define IDCODE_DEVID_MASK 0x00000FFFU
__IO uint32_t uwTick;
uint32_t uwTickPrio = (1UL << __NVIC_PRIO_BITS); 
HAL_TickFreqTypeDef uwTickFreq = HAL_TICK_FREQ_DEFAULT;

HAL_StatusTypeDef HAL_Init(void)
{
  #if (PREFETCH_ENABLE != 0)
  #if defined(STM32F101x6) || defined(STM32F101xB) || defined(STM32F101xE) || defined(STM32F101xG) || \
  defined(STM32F102x6) || defined(STM32F102xB) || \
  defined(STM32F103x6) || defined(STM32F103xB) || defined(STM32F103xE) || defined(STM32F103xG) || \
  defined(STM32F105xC) || defined(STM32F107xC)
  // __HAL_FLASH_PREFETCH_BUFFER_ENABLE();
  #endif
  #endif 
  // HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
  HAL_InitTick(TICK_INT_PRIORITY);
  HAL_MspInit();
  return HAL_OK;
}

HAL_StatusTypeDef HAL_DeInit(void)
{
  (RCC->APB1RSTR = 0xFFFFFFFFU); //   __HAL_RCC_APB1_FORCE_RESET();
  (RCC->APB1RSTR = 0x00); //   __HAL_RCC_APB1_RELEASE_RESET();
  (RCC->APB2RSTR = 0xFFFFFFFFU); //__HAL_RCC_APB2_FORCE_RESET();
  (RCC->APB2RSTR = 0x00); // __HAL_RCC_APB2_RELEASE_RESET();

  HAL_MspDeInit();
  return HAL_OK;
}

__weak void HAL_MspInit(void)
{
}

__weak void HAL_MspDeInit(void)
{
}

__weak HAL_StatusTypeDef HAL_InitTick(uint32_t TickPriority)
{
  return HAL_OK;
}

__weak void HAL_IncTick(void)
{
  uwTick += uwTickFreq;
}

__weak uint32_t HAL_GetTick(void)
{
  return uwTick;
}

uint32_t HAL_GetTickPrio(void)
{
  return uwTickPrio;
}

HAL_StatusTypeDef HAL_SetTickFreq(HAL_TickFreqTypeDef Freq)
{
  HAL_StatusTypeDef status = HAL_OK;
  HAL_TickFreqTypeDef prevTickFreq;
  assert_param(IS_TICKFREQ(Freq));
  if (uwTickFreq != Freq)
  {
    prevTickFreq = uwTickFreq;
    uwTickFreq = Freq;
    status = HAL_InitTick(uwTickPrio);
    if (status != HAL_OK)
    {
      uwTickFreq = prevTickFreq;
    }
  }
  return status;
}

HAL_TickFreqTypeDef HAL_GetTickFreq(void)
{
  return uwTickFreq;
}

__weak void HAL_Delay(uint32_t Delay)
{
  uint32_t tickstart = HAL_GetTick();
  uint32_t wait = Delay;
  if (wait < HAL_MAX_DELAY)
  {
    wait += (uint32_t)(uwTickFreq);
  }
  while ((HAL_GetTick() - tickstart) < wait)
  {
  }
}

__weak void HAL_SuspendTick(void)
{
  CLEAR_BIT(SysTick->CTRL, SysTick_CTRL_TICKINT_Msk);
}

__weak void HAL_ResumeTick(void)
{
  SET_BIT(SysTick->CTRL, SysTick_CTRL_TICKINT_Msk);
}

uint32_t HAL_GetHalVersion(void)
{
  return __STM32F1xx_HAL_VERSION;
}

uint32_t HAL_GetREVID(void)
{
  return ((DBGMCU->IDCODE) >> DBGMCU_IDCODE_REV_ID_Pos);
}

uint32_t HAL_GetDEVID(void)
{
  return ((DBGMCU->IDCODE) & IDCODE_DEVID_MASK);
}

uint32_t HAL_GetUIDw0(void)
{
  return(READ_REG(*((uint32_t *)UID_BASE)));
}

uint32_t HAL_GetUIDw1(void)
{
  return(READ_REG(*((uint32_t *)(UID_BASE + 4U))));
}

uint32_t HAL_GetUIDw2(void)
{
  return(READ_REG(*((uint32_t *)(UID_BASE + 8U))));
}

void HAL_DBGMCU_EnableDBGSleepMode(void)
{
  SET_BIT(DBGMCU->CR, DBGMCU_CR_DBG_SLEEP);
}

void HAL_DBGMCU_DisableDBGSleepMode(void)
{
  CLEAR_BIT(DBGMCU->CR, DBGMCU_CR_DBG_SLEEP);
}

void HAL_DBGMCU_EnableDBGStopMode(void)
{
  SET_BIT(DBGMCU->CR, DBGMCU_CR_DBG_STOP);
}

void HAL_DBGMCU_DisableDBGStopMode(void)
{
  CLEAR_BIT(DBGMCU->CR, DBGMCU_CR_DBG_STOP);
}

void HAL_DBGMCU_EnableDBGStandbyMode(void)
{
  SET_BIT(DBGMCU->CR, DBGMCU_CR_DBG_STANDBY);
}

void HAL_DBGMCU_DisableDBGStandbyMode(void)
{
  CLEAR_BIT(DBGMCU->CR, DBGMCU_CR_DBG_STANDBY);
}

#endif 

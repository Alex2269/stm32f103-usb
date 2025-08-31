// main.c

#include "main.h"
#include "usb_device.h"
#include "usbd_core.h"
#include "usbd_cdc.h"
#include "usbd_cdc_if.h"
#include "adc_read.h"
#include "gpio_init.h"
#include "SystemClock_Config.h"

extern USBD_DescriptorsTypeDef FS_Desc;
extern USBD_ClassTypeDef  USBD_CDC;
extern USBD_HandleTypeDef hUsbDeviceFS;

// void LL_mDelay(uint32_t Delay);
void SystemClock_Config(void);

// Функція налаштування піну для USB (для скидування USB)
void USB_GPIO_Init(void) {
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;

    // PA11 (DM) - input floating
    GPIOA->CRH &= ~(GPIO_CRH_MODE11 | GPIO_CRH_CNF11);
    GPIOA->CRH |= GPIO_CRH_CNF11_0;

    // PA12 (DP) - AF push-pull output 50MHz
    GPIOA->CRH &= ~(GPIO_CRH_MODE12 | GPIO_CRH_CNF12);
    GPIOA->CRH |= GPIO_CRH_MODE12_1 | GPIO_CRH_MODE12_0;   // Output mode 50 MHz
    GPIOA->CRH |= GPIO_CRH_CNF12_1;                        // Alternate function push-pull

    RCC->APB1ENR |= RCC_APB1ENR_USBEN;
}

// Функція налаштування піну для USB (для скидування USB)
void USB_DEVICE_PinReset(void)
{
    // Увімкнення тактуючого сигналу для GPIOA
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;

    // Налаштування піну PA12 як вихід (Push-Pull, високошвидкісний)
    GPIOA->CRH &= ~(GPIO_CRH_MODE12 | GPIO_CRH_CNF12); // Очищаємо біти
    GPIOA->CRH |= (GPIO_CRH_MODE12_1 | GPIO_CRH_MODE12_0); // Output mode, 50 MHz
    GPIOA->CRH |= GPIO_CRH_CNF12_0; // General purpose output push-pull

    // Встановлення логічного рівня високого на PA12
    GPIOA->BSRR = GPIO_BSRR_BS12;

    // Невелика затримка (імітація)
    for (volatile int i = 0; i < 720000; i++); // ~10 мс (на 72 МГц)

    // Встановлення логічного рівня низького на PA12
    GPIOA->BSRR = GPIO_BSRR_BR12;
}

void USB_DEVICE_Init(void)
{
  USBD_Init(&hUsbDeviceFS, &FS_Desc, DEVICE_FS);
  USBD_RegisterClass(&hUsbDeviceFS, &USBD_CDC);
  USBD_CDC_RegisterInterface(&hUsbDeviceFS, &USBD_Interface_fops_FS);
  USBD_Start(&hUsbDeviceFS);
}

int main(void)
{
  /* USER CODE BEGIN 1 */

  SET_BIT(RCC->APB2ENR, RCC_APB2ENR_AFIOEN); // LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_AFIO);
  SET_BIT(RCC->APB1ENR, RCC_APB1ENR_PWREN); // LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);

  NVIC_SetPriorityGrouping(0x3U);
  MODIFY_REG(AFIO->MAPR, AFIO_MAPR_SWJ_CFG, AFIO_MAPR_SWJ_CFG_JTAGDISABLE); // LL_GPIO_AF_Remap_SWJ_NOJTAG();

  SystemClock_Config();

  int SystemCoreClock = 72000000; // 72 MHz
  // Кількість тактів за 1 мс = SystemCoreClock / 1000.
  // Викличте SysTick_Config з обчисленим значенням
  SysTick_Config(SystemCoreClock / 1000); // 1мсек, для LL_mDelay

  gpio_init(GPIOC, 13, GPIO_MODE_OUTPUT_PP);

  USB_GPIO_Init();
  USB_DEVICE_PinReset();

  USBD_LL_SetSpeed(&hUsbDeviceFS, USBD_SPEED_FULL);
  // USBD_LL_Reset(&hUsbDeviceFS);

  // MX_GPIO_Init();
  MX_USB_DEVICE_Init();

  while (1)
  {
    //uint8_t usb_send_buf[] = "hello\r\n";
    // CDC_Transmit_FS(usb_send_buf, 7);
    printf("hello\r\n");

    gpio_toggle_pin(GPIOC, 13);
    LL_mDelay(350);
  }
  /* USER CODE END 3 */
}

void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}

#ifdef  USE_FULL_ASSERT

void assert_failed(uint8_t *file, uint32_t line)
{
}
#endif /* USE_FULL_ASSERT */


/**
  ******************************************************************************
  * @file    WWDG/WWDG_ResetAfterSwFailure/Src/main.c
  * @author  MCD Application Team
  * @brief   This example shows how to update the WWDG counter at regular
  *          time intervals and how to simulate a software fault generating
  *          a MCU WWDG reset on expiry of a programmed time period.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/** @addtogroup STM32H7xx_HAL_Examples
  * @{
  */

/** @addtogroup WWDG_ResetAfterSwFailure
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* WWDG handler declaration */
static WWDG_HandleTypeDef   WwdgHandle;

/* Private function prototypes -----------------------------------------------*/
static void MPU_Config(void);
static void SystemClock_Config(void);
static void Error_Handler(void);
static void CPU_CACHE_Enable(void);
static uint32_t TimeoutCalculation(uint32_t timevalue);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
  uint32_t delay;

  /* This project calls firstly function CPU_CACHE_Enable().
     This function is provided as implementation that the user may integrate
     into his application to enable the Coretx-M7 I/D cache and enhance the performance.
   */

  /* Configure the MPU attributes */
  MPU_Config();

  /* Enable the CPU Cache */
  CPU_CACHE_Enable();

  /* STM32H7xx HAL library initialization:
       - Configure the Systick to generate an interrupt each 1 msec
       - Set NVIC Group Priority to 4
       - Low Level Initialization
   */
  HAL_Init();

  /* Configure the system clock to 280 MHz */
  SystemClock_Config();

  /* Configure LED1 and LED2 */
  BSP_LED_Init(LED1);
  BSP_LED_Init(LED2);

  /* Configure Tamper push-button */
  BSP_PB_Init(BUTTON_TAMPER, BUTTON_MODE_EXTI);

  /*##-1- Check if the system has resumed from WWDG reset ####################*/
  if (__HAL_RCC_GET_FLAG(RCC_FLAG_WWDG1RST) != RESET)
  {
    /* WWDG1RST flag set: Turn LED1 on */
    BSP_LED_On(LED1);

    /* Insert 4s delay */
    HAL_Delay(4000);

    /* Prior to clear WWDG1RST flag: Turn LED1 off */
    BSP_LED_Off(LED1);
  }

  /* Clear reset flags in any case */
  __HAL_RCC_CLEAR_RESET_FLAGS();

  /*##-2- Init & Start WWDG peripheral ######################################*/
  /*  Configuration:
      1] Set WWDG counter to maximum 0x7F (127 cycles)  and window to 0x50 (80 cycles)
      2] Set Prescaler to 128 (2^7)

      Timing calculation:
      a) WWDG clock counter period (in ms) = (4096 * 128) / (PCLK1 / 1000)
                                          ~= 3,744 ms
      b) WWDG timeout (in ms)  = (127 + 1) * 3,744
                              ~= 479 ms
      --> After refresh, WWDG will expire after 479 ms and generate reset if
      counter is not reloaded.
      c) Time to enter inside window
      Window timeout (in ms) = (127 - 80 + 1) * 3,744
                            ~= 179 ms */
  WwdgHandle.Instance = WWDG1;
  WwdgHandle.Init.Prescaler = WWDG_PRESCALER_128;
  WwdgHandle.Init.Window    = 0x50;
  WwdgHandle.Init.Counter   = 0x7F;
  WwdgHandle.Init.EWIMode   = WWDG_EWI_DISABLE;
  if (HAL_WWDG_Init(&WwdgHandle) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler();
  }

  /* Calculate delay to enter window. Add 1ms to secure round number to upper number  */
  delay = TimeoutCalculation((WwdgHandle.Init.Counter - WwdgHandle.Init.Window) + 1) + 1;

  /* Infinite loop */
  while (1)
  {
    /* Toggle LED1 */
    BSP_LED_Toggle(LED1);

    /* Insert calculated delay */
    HAL_Delay(delay);

    if (HAL_WWDG_Refresh(&WwdgHandle) != HAL_OK)
    {
      Error_Handler();
    }
  }
}

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow :
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 280000000 (CPU Clock)
  *            HCLK(Hz)                       = 280000000 (Bus matrix and AHBs Clock)
  *            AHB Prescaler                  = 1
  *            CD APB3 Prescaler              = 2 (APB3 Clock  140MHz)
  *            CD APB1 Prescaler              = 2 (APB1 Clock  140MHz)
  *            CD APB2 Prescaler              = 2 (APB2 Clock  140MHz)
  *            SRD APB4 Prescaler             = 2 (APB4 Clock  140MHz)
  *            HSE Frequency(Hz)              = 24000000
  *            PLL_M                          = 12
  *            PLL_N                          = 280
  *            PLL_P                          = 2
  *            PLL_Q                          = 2
  *            PLL_R                          = 2
  *            VDD(V)                         = 3.3
  *            Flash Latency(WS)              = 6
  * @param  None
  * @retval None
  */
static void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;
  HAL_StatusTypeDef ret = HAL_OK;

  /* The voltage scaling allows optimizing the power consumption when the device is
  clocked below the maximum system frequency, to update the voltage scaling value
  regarding system frequency refer to product datasheet.
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /* Enable HSE Oscillator and activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_OFF;
  RCC_OscInitStruct.CSIState = RCC_CSI_OFF;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;

  RCC_OscInitStruct.PLL.PLLM = 12;
  RCC_OscInitStruct.PLL.PLLN = 280;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLQ = 2;

  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_1;
  ret = HAL_RCC_OscConfig(&RCC_OscInitStruct);
  if(ret != HAL_OK)
  {
  while(1) {};
  }

  /* Select PLL as system clock source and configure  bus clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_D1PCLK1 | RCC_CLOCKTYPE_PCLK1 | \
    RCC_CLOCKTYPE_PCLK2  | RCC_CLOCKTYPE_D3PCLK1);

  RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider  = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider  = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;
  ret = HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_6);
  if(ret != HAL_OK)
  {
  while(1) {};
  }
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
static void Error_Handler(void)
{
  /* Turn LED2 on */
  BSP_LED_On(LED2);
  while(1)
  {
  }
}

/**
* @brief  CPU L1-Cache enable.
* @param  None
* @retval None
*/
static void CPU_CACHE_Enable(void)
{
  /* Enable I-Cache */
  SCB_EnableICache();

  /* Enable D-Cache */
  SCB_EnableDCache();
}

/**
  * @brief  Timeout calculation function.
  *         This function calculates any timeout related to
  *         WWDG with given prescaler and system clock.
  * @param  timevalue: period in term of WWDG counter cycle.
  * @retval None
  */
static uint32_t TimeoutCalculation(uint32_t timevalue)
{
  uint32_t timeoutvalue = 0;
  uint32_t pclk1 = 0;
  uint32_t wdgtb = 0;

  /* Get PCLK1 value */
  pclk1 = HAL_RCC_GetPCLK1Freq();

  /* get prescaler */
  switch(WwdgHandle.Init.Prescaler)
  {
    case WWDG_PRESCALER_1:   wdgtb = 1;   break;
    case WWDG_PRESCALER_2:   wdgtb = 2;   break;
    case WWDG_PRESCALER_4:   wdgtb = 4;   break;
    case WWDG_PRESCALER_8:   wdgtb = 8;   break;
    case WWDG_PRESCALER_16:  wdgtb = 16;  break;
    case WWDG_PRESCALER_32:  wdgtb = 32;  break;
    case WWDG_PRESCALER_64:  wdgtb = 64;  break;
    case WWDG_PRESCALER_128: wdgtb = 128; break;

    default: Error_Handler(); break;
  }

  /* calculate timeout */
  timeoutvalue = ((4096 * wdgtb * timevalue) / (pclk1 / 1000));

  return timeoutvalue;
}


/**
  * @brief  Configure the MPU attributes
  * @param  None
  * @retval None
  */
static void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct;

  /* Disable the MPU */
  HAL_MPU_Disable();

  /* Configure the MPU as Strongly ordered for not defined regions */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.BaseAddress = 0x00;
  MPU_InitStruct.Size = MPU_REGION_SIZE_4GB;
  MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.SubRegionDisable = 0x87;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /* Enable the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/**
  * @}
  */

/**
  * @}
  */


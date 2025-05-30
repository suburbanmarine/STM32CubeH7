/**
  @page LCD_DSI_ULPM_DataClock : LCD DSI example : enter and exit DSI ULPM Mode on data lanes 
  and clock lane while displaying a picture 800x480 (WVGA) 
  in landscape mode in DSI mode Video Burst on LCD screen.

  @verbatim
  ******************************************************************************
  * @file    LCD_DSI/LCD_DSI_ULPM_DataClock/readme.txt
  * @author  MCD Application Team
  * @brief   Description of the LCD DSI enter and exit DSI ULPM Mode on data lane 
  *          and clock lane example.
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
  @endverbatim

@par Example Description

This example provides a description of how to use the embedded LCD DSI controller 
(using IPs LTDC and DSI Host) to drive the KoD LCD mounted on board and manage entry and exit in 
DSI ULPM mode on data lane and clock lane. 
In this mode, the DSI PHY state machine is entering a low power state
on data lane and clock lane.
The DSI Dphy_Clk should be derived from the clock tree PLL.PLLR (ck_plldsi) with a 60 MHz clock for instance
to allow a switch off of the PLL DPHY to save power during the ULPM phase.
When exiting from ULPM, the PLL DPHY is first switched back on and locked, then the DSI DPHY will exit
power down and display again as before.

The SystemClock_Config() function is used to set the Flash latency and  to configure the system clock :
  - The Cortex-M7 at 400MHz 
  - Cortex-M4 at 200MHz.
  - The HCLK for D1 Domain AXI/AHB3 peripherals , D2 Domain AHB1/AHB2 peripherals 
    and D3 Domain AHB4  peripherals at 200MHz.
  - The APB clock dividers for D1 Domain APB3 peripherals, D2 Domain APB1/APB2 peripherals 
    and D3 Domain APB4 peripherals to run at 100MHz.

CPU1 (Cortex-M7) and CPU2 (Cortex-M4) are booting at once (with respect to configured boot Flash options)
System Init, System clock, voltage scaling and L1-Cache configuration are done by CPU1 (Cortex-M7).
In the meantime Domain D2 is put in STOP mode (CPU2: Cortex-M4 in deep sleep mode) to save power consumption.
When system initialization is finished, CPU1 (Cortex-M7) could release CPU2 (Cortex-M4) when needed
by means of HSEM notification or by any D2 Domain wakeup source (SEV,EXTI..).

The above will guarantee that CPU2 (Cortex-M4) code execution starts after system initialization :
(system clock config, external memory configuration..).

After Domain D2 wakeup, if  CPU1 attempts to use any resource from such a domain, 
the access will not be safe until “clock ready flag” of such a domain is set (by hardware). 
The check could be done using this macro : __HAL_RCC_GET_FLAG(RCC_FLAG_D2CKRDY).

In this example a landscape picture (320x240) is displayed on LCD in DSI Mode Video Burst.
On a Wakeup button press from the user, the LCD display will be switched off. One second later, 
the DSI PHY Data and clock lane will enter ULPM mode. DphyClk is derived from PLL.PLLR, then
DPHY PLL is switched Off.

After 6 seconds in Off mode, The PLL DPHY is switched back on and locked,
then the ULPM on data and clock lanes will be exited in DPHY DSI state machine.
The LCD will then be switched back on and display the same image as before 
with a text notifying that the exit from ULPM was successful. 

The image_320x240_argb8888 is of format ARGB8888 and is initially copied from Flash to SDRAM Frame Buffer.
The LTDC is reading continuously the LCD Frame buffer from SDRAM, sent it to DSI Host which sends it in burst mode (DCS commands)
via the DSI DPHY to the KoD display that decodes DSI packets and refresh its internal Graphic RAM to display on glass.

The Frame number display is managed by Line Event callback in which the Frame number is incremented
each time a line event occurs. When entering to ULPM, The Frame number is unchanged until the
exit from this mode.

LED4 ON: DSI PHY Data and clock lane in ULPM mode
LED4 OFF: DSI PHY Data and clock lane in run mode
LED3 ON: an error occurred.

@Note For the Cortex-M7, if the application is using the DTCM/ITCM memories (@0x20000000/ 0x0000000: not cacheable and only accessible
      by the Cortex-M7 and the  MDMA), no need for cache maintenance when the Cortex M7 and the MDMA access these RAMs.
      If the application needs to use DMA(or other masters) based access or requires more RAM, then  the user has to:
              - Use a non TCM SRAM. (example : D1 AXI-SRAM @ 0x24000000)
              - Add a cache maintenance mechanism to ensure the cache coherence between CPU and other masters(DMAs,DMA2D,LTDC,MDMA).
              - The addresses and the size of cacheable buffers (shared between CPU and other masters)
                must be properly defined to be aligned to L1-CACHE line size (32 bytes). 
 
@Note It is recommended to enable the cache and maintain its coherence.
      Depending on the use case it is also possible to configure the cache attributes using the MPU.
      Please refer to the AN4838 "Managing memory protection unit (MPU) in STM32 MCUs"
      Please refer to the AN4839 "Level 1 cache on STM32F7 Series"

@note Care must be taken when using HAL_Delay(), this function provides accurate
      delay (in milliseconds) based on variable incremented in SysTick ISR. This
      implies that if HAL_Delay() is called from a peripheral ISR process, then
      the SysTick interrupt must have higher priority (numerically lower)
      than the peripheral interrupt. Otherwise the caller ISR process will be blocked.
      To change the SysTick interrupt priority you have to use HAL_NVIC_SetPriority() function.

@note The application need to ensure that the SysTick time base is always set to 1 millisecond
      to have correct HAL operation.


      
@par Keywords

Graphics, Display, LCD, DSI, MIPI Alliance, Ultra low power mode, Tearing effect, Partial refresh, Single buffer,
LTDC,DMA2D, WVGA, ARGB8888, SDRAM, Burst mode, Data lane, clock lane



@par Directory contents

  - LCD_DSI/LCD_DSI_ULPM_DataClock/Common/Src/system_stm32h7xx.c     STM32H7xx system configuration source file 
  
  - LCD_DSI/LCD_DSI_ULPM_DataClock/CM7/Inc/stm32h7xx_hal_conf.h          HAL configuration file for Cortex-M7
  - LCD_DSI/LCD_DSI_ULPM_DataClock/CM7/Inc/stm32h7xx_it.h                Interrupt handlers header file for Cortex-M7
  - LCD_DSI/LCD_DSI_ULPM_DataClock/CM7/Inc/main.h                        Header for main.c module for Cortex-M7
  - LCD_DSI/LCD_DSI_ULPM_DataClock/CM7/Inc/image_320x240_argb8888        Image 320x240 in ARGB8888 to display on LCD
  - LCD_DSI/LCD_DSI_ULPM_DataClock/CM7/Src/stm32h7xx_it.c                Interrupt handlers for Cortex-M7
  - LCD_DSI/LCD_DSI_ULPM_DataClock/CM7/Src/main.c                        Main program for Cortex-M7
  - LCD_DSI/LCD_DSI_ULPM_DataClock/CM7/Src/stm32h7xx_hal_msp.c           HAL MSP file for Cortex-M7

  - LCD_DSI/LCD_DSI_ULPM_DataClock/CM4/Inc/stm32h7xx_hal_conf.h    HAL configuration file for Cortex-M4
  - LCD_DSI/LCD_DSI_ULPM_DataClock/CM4/Inc/stm32h7xx_it.h          Interrupt handlers header file for Cortex-M4
  - LCD_DSI/LCD_DSI_ULPM_DataClock/CM4/Inc/main.h                  Main configuration file for Cortex-M4
  - LCD_DSI/LCD_DSI_ULPM_DataClock/CM4/Src/stm32h7xx_it.c          Interrupt handlers for Cortex-M4
  - LCD_DSI/LCD_DSI_ULPM_DataClock/CM4/Src/main.c                  Main program  for Cortex-M4
  - LCD_DSI/LCD_DSI_ULPM_DataClock/CM4/Src/stm32h7xx_hal_msp.c     HAL MSP module for Cortex-M4 
  
@par Hardware and Software environment

  - This example runs on STM32H747xx/STM32H757xx devices.

  - This example has been tested with STMicroelectronics STM32H747I-DISCO 
    board and can be easily tailored to any other supported device
    and development board.

@par How to use it ?

In order to make the program work, you must do the following :
 - Open your preferred toolchain 
 - For each target configuration (STM32H747I_DISCO_CM4 first then STM32H747I_DISCO_CM7) : 
     - Rebuild all files 
     - Load images into target memory
 - After loading the two images, you have to reset the board in order to boot (Cortex-M7) and CPU2 (Cortex-M4) at once.
 - Run the example



 */

/**
  @page FreeRTOS_HwSemaphoreCoreSync HSEM Core Notification application
  
  @verbatim
  ******************************************************************************
  * @file    FreeRTOS/FreeRTOS_HwSemaphoreCoreSync/readme.txt
  * @author  MCD Application Team
  * @brief   Description of the FreeRTOS HwSemaphore CoreSync application.
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

@par Application Description

This application shows how to use  embedded hardware semaphore to send notification between cores.

CPU1 (Cortex-M7) and CPU2 (Cortex-M4) are booting at once (with respect to configured boot Flash options)
System Init, System clock, voltage scaling and L1-Cache configuration are done by CPU1 (Cortex-M7).
In the meantime Domain D2 is put in STOP mode (CPU2: Cortex-M4 in deep sleep mode) to save power consumption.
When system initialization is finished, CPU1 (Cortex-M7) will release CPU2 (Cortex-M4) when needed
by means of HSEM notification but release could be also ensured by any Domain D2 wakeup source (SEV,EXTI..).

The above will guarantee that CPU2 (Cortex-M4) code execution starts after system initialization :
(system clock config, external memory configuration..).

After Domain D2 wakeup, if  CPU1 attempts to use any resource from such a domain, 
the access will not be safe until �clock ready flag� of such a domain is set (by hardware). 
The check could be done using this macro : __HAL_RCC_GET_FLAG(RCC_FLAG_D2CKRDY).

In this example:
    - System Init, voltage scaling, UART and system clock configuration are done only once by CPU1(CM7). 
      
    - CPU1(CM7) Creates a task that toggles the LED1 each 500ms.
    - CPU1(CM7) send Notification to CPU2(CM4) using HSEM:
      * It takes The HSEM n.
      * It toggles the LED1 and wait 500ms.
      * Then, it releases the HSEM n ( This will generate an interrupt to CPU2(CM4))
    - CPU2(CM4) Creates a task looping on waiting for a Software Semaphore and toggling LED3.
    - CPU2(CM4) activates HSEM Notification and wait for Notification to be send by CPU1(CM7).

    - If CPU2(CM4) receives the HSEM Notification, it releases the SW Semaphore.
    - When the SW Semaphore is released, CPU2 toggles LED3    
    
STM32H747I-DISCO board's LEDs can be used to monitor the execution status:
- LED1 and LED3 toggle synchronously

The SystemClock_Config() function is used to set the Flash latency and  to configure the system clock :
  - The Cortex-M7 at 400MHz 
  - Cortex-M4 at 200MHz.
  - The HCLK for D1 Domain AXI/AHB3 peripherals , D2 Domain AHB1/AHB2 peripherals 
    and D3 Domain AHB4  peripherals at 200MHz.
  - The APB clock dividers for D1 Domain APB3 peripherals, D2 Domain APB1/APB2 peripherals 
    and D3 Domain APB4 peripherals to run at 100MHz.

@Note For the Cortex-M7, if the application is using the DTCM/ITCM memories (@0x20000000/ 0x0000000: not cacheable and only accessible
      by the Cortex-M7 and the �MDMA), no need for cache maintenance when the Cortex M7 and the MDMA access these RAMs.
      If the application needs to use DMA(or other masters) based access or requires more RAM, then �the user has to:
              - Use a non TCM SRAM. (example : D1 AXI-SRAM @ 0x24000000)
              - Add a cache maintenance mechanism to ensure the cache coherence between CPU and other masters(DMAs,DMA2D,LTDC,MDMA).
              - The addresses and the size of cacheable buffers (shared between CPU and other masters)
                must be properly�defined to be aligned to L1-CACHE line size (32 bytes). 
�
@Note It is recommended to enable the cache and maintain its coherence.
      Depending on the use case it is also possible to configure the cache attributes using the MPU.
������Please refer to the AN4838 "Managing memory protection unit (MPU) in STM32 MCUs"
������Please refer to the AN4839 "Level 1 cache on STM32F7 Series"

@note Care must be taken when using HAL_Delay(), this function provides accurate delay (in milliseconds)
      based on variable incremented in SysTick ISR. This implies that if HAL_Delay() is called from
      a peripheral ISR process, then the SysTick interrupt must have higher priority (numerically lower)
      than the peripheral interrupt. Otherwise the caller ISR process will be blocked.
      To change the SysTick interrupt priority you have to use HAL_NVIC_SetPriority() function.
      
@note The application need to ensure that the SysTick time base is always set to 1 millisecond
      to have correct HAL operation.

@note The FreeRTOS heap size configTOTAL_HEAP_SIZE defined in FreeRTOSConfig.h is set accordingly to the 
      OS resources memory requirements of the application with +10% margin and rounded to the upper Kbyte boundary.

For more details about FreeRTOS implementation on STM32Cube, please refer to UM1722 "Developing Applications 
on STM32Cube with RTOS".

@par Keywords

RTOS, FreeRTOS, multiprocessor, Threading, HSEM, Semaphore, Software Semaphore, Hardware Semaphore, Priorities, Notification, Interrupt

@par Directory contents

    - FreeRTOS/FreeRTOS_HwSemaphoreCoreSync/Common/Src/system_stm32h7xx.c  STM32H7xx system configuration source file
    - FreeRTOS/FreeRTOS_HwSemaphoreCoreSync/Common/Inc/FreeRTOSConfig.h    FreeRTOS Configuration file
	
    - FreeRTOS/FreeRTOS_HwSemaphoreCoreSync/CM7/Inc/stm32h7xx_hal_conf.h    HAL configuration file for Cortex-M7
    - FreeRTOS/FreeRTOS_HwSemaphoreCoreSync/CM7/Inc/stm32h7xx_it.h          Interrupt handlers header file for Cortex-M7
    - FreeRTOS/FreeRTOS_HwSemaphoreCoreSync/CM7/Inc/main.h                  Header for main.c module for Cortex-M7  
    - FreeRTOS/FreeRTOS_HwSemaphoreCoreSync/CM7/Src/stm32h7xx_it.c          Interrupt handlers for Cortex-M7
    - FreeRTOS/FreeRTOS_HwSemaphoreCoreSync/CM7/Src/main.c                  Main program for Cortex-M7

    - FreeRTOS/FreeRTOS_HwSemaphoreCoreSync/CM4/Inc/stm32h7xx_hal_conf.h    HAL configuration file for Cortex-M4
    - FreeRTOS/FreeRTOS_HwSemaphoreCoreSync/CM4/Inc/stm32h7xx_it.h          Interrupt handlers header file for Cortex-M4
    - FreeRTOS/FreeRTOS_HwSemaphoreCoreSync/CM4/Inc/main.h                  Header for main.c module for Cortex-M4  
    - FreeRTOS/FreeRTOS_HwSemaphoreCoreSync/CM4/Src/stm32h7xx_it.c          Interrupt handlers for Cortex-M4
    - FreeRTOS/FreeRTOS_HwSemaphoreCoreSync/CM4/Src/main.c                  Main program for Cortex-M4


@par Hardware and Software environment

  - This example runs on STM32H747xx devices.
    
  - This example has been tested with STM32H747I-DISCO board and can be
    easily tailored to any other supported device and development board.

@par How to use it ? 

In order to make the program work, you must do the following :
 - Open your preferred toolchain
 - Rebuild all files and load your 2 images into target memory for each core.
 - Reset the Board so the 2 CPUs start running their respective code.



 */

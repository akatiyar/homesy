//*****************************************************************************
// main.c
//
// Camera Application
//
// Copyright (C) 2014 Texas Instruments Incorporated - http://www.ti.com/
//
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions
//  are met:
//
//    Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//
//    Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the
//    distribution.
//
//    Neither the name of Texas Instruments Incorporated nor the names of
//    its contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
//  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
//  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
//  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
//  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
//  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
//  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
//  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
//  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//*****************************************************************************
//*****************************************************************************
//
// Application Name     -   Camera Application
// Application Overview -   This is a sample application demonstrating camera on
//                          a CC3200 Launchpad setup. Micron MT9D111
//                          camera sensor driver is used in this example
//
// Application Details  -
// http://processors.wiki.ti.com/index.php/CC32xx_Camera_Application
// or
// doc\examples\CC32xx_Camera_Application.pdf
//
//*****************************************************************************

//*****************************************************************************
//
//! \addtogroup camera_app
//! @{
//
//*****************************************************************************

// Driverlib includes
#include "rom.h"
#include "rom_map.h"
#include "hw_types.h"
#include "hw_ints.h"
#include "hw_memmap.h"
#include "interrupt.h"
#include "prcm.h"

// Simplelink includes
#include "simplelink.h"

// OS includes
#include "osi.h"

// Common interface includes
#include "uart_if.h"
#include "udma_if.h"
#include "gpio_if.h"
#include "common.h"
#ifndef NOTERM
#include "uart_if.h"
#endif

#include "pinmux.h"
#include "camera_app.h"

#include "lightSens_isl29035.h"
#include "i2cconfig.h"
#include "mt9d111.h"
#include "camera_app.h"
//*****************************************************************************
// Macros
//*****************************************************************************
//#define OSI_STACK_SIZE                  1024

#define APPLICATION_NAME        "CAMERA_APPLICATION"
#define APPLICATION_VERSION     "1.1.0"

//*****************************************************************************
//                 GLOBAL VARIABLES -- Start
//*****************************************************************************
volatile unsigned char g_CaptureImage = 0;
OsiTaskHandle g_CameraTaskHandle;

void EnterHIBernate();
void CameraService(void *pvParameters);

// Vector table entry
#if defined(ewarm)
extern uVectorEntry __vector_table;
#endif
#if defined(ccs)
extern void (* const g_pfnVectors[])(void);
#endif
//*****************************************************************************
//                 GLOBAL VARIABLES -- End
//*****************************************************************************


//****************************************************************************
//                      FUNCTION DEFINITIONS
//****************************************************************************
#ifdef USE_FREERTOS
//*****************************************************************************
// FreeRTOS User Hook Functions enabled in FreeRTOSConfig.h
//*****************************************************************************

//*****************************************************************************
//
//! \brief Application defined hook (or callback) function - assert
//!
//! \param[in]  pcFile - Pointer to the File Name
//! \param[in]  ulLine - Line Number
//!
//! \return none
//!
//*****************************************************************************
void
vAssertCalled( const char *pcFile, unsigned long ulLine )
{
    //Handle Assert here
    while(1)
    {
    }
}

//*****************************************************************************
//
//! \brief Application defined idle task hook
//!
//! \param  none
//!
//! \return none
//!
//*****************************************************************************
void
vApplicationIdleHook( void)
{
	//UART_PRINT("k");
    //Handle Idle Hook for Profiling, Power Management etc
}

//*****************************************************************************
//
//! \brief Application defined malloc failed hook
//!
//! \param  none
//!
//! \return none
//!
//*****************************************************************************
void vApplicationMallocFailedHook()
{
    //Handle Memory Allocation Errors
    while(1)
    {
    }
}

//*****************************************************************************
//
//! \brief Application defined stack overflow hook
//!
//! \param  none
//!
//! \return none
//!
//*****************************************************************************
void vApplicationStackOverflowHook( OsiTaskHandle *pxTask,
                                   signed char *pcTaskName)
{
    //Handle FreeRTOS Stack Overflow
    while(1)
    {
    }
}
#endif //USE_FREERTOS


//*****************************************************************************
//
//! Board Initialization & Configuration
//!
//! \param  None
//!
//! \return None
//
//*****************************************************************************
static void BoardInit(void)
{
// In case of TI-RTOS vector table is initialize by OS itself
#ifndef USE_TIRTOS
    //
    // Set vector table base
    //
#if defined(ccs) || defined(gcc)
    MAP_IntVTableBaseSet((unsigned long)&g_pfnVectors[0]);
#endif
#if defined(ewarm)
    MAP_IntVTableBaseSet((unsigned long)&__vector_table);
#endif
#endif
    //
    // Enable Processor
    //
    MAP_IntMasterEnable();
    MAP_IntEnable(FAULT_SYSTICK);

    PRCMCC3200MCUInit();
}
void EnterHIBernate()
{
    //
    // Enable the HIB RTC
    //
    MAP_PRCMHibernateWakeupSourceEnable(PRCM_HIB_GPIO2);
    MAP_PRCMHibernateWakeUpGPIOSelect(PRCM_HIB_GPIO2, PRCM_HIB_FALL_EDGE);

    DBG_PRINT("\n\rHIB: Entering HIBernate...\n\r Switch on light to wake up CC3200\n\r");
    MAP_UtilsDelay(80000);
    getLightsensor_intrptStatus();

    //
    // Enter HIBernate mode
    //
    MAP_PRCMHibernateEnter();
}
//*****************************************************************************
//
//!     Camera Service starts here
//!
//! \param     pvParameters     Parametrs to the task's entry function
//! \return                     None
//
//*****************************************************************************

void CameraService(void *pvParameters)
{
    StartCamera();

    getLightsensor_intrptStatus(); //Reading the register clears the interrupt
    UART_PRINT("\n\rCleared light sensor interrupt register\n\r");

    EnterHIBernate();

    osi_TaskDelete(&g_CameraTaskHandle);
}
//*****************************************************************************
//
//!     Main()
//!     Initializes the system
//!     Spawns CameraService and other resources
//!
//!    \param                      None
//!     \return                     On Sucess, zero is returned
//
//*****************************************************************************

int main()
{
  long lRetVal = -1;

  //
  // Initialize Board configurations
  //
  BoardInit();

  PinMuxConfig();

  //
  // Configuring UART
  //
#ifndef NOTERM
    InitTerm();
    UART_PRINT("Test\n");
#endif  //NOTERM

	//
	// Initialize I2C Interface
	//
	lRetVal = I2CInit();
	ASSERT_ON_ERROR(lRetVal);


	if(MAP_PRCMSysResetCauseGet() == 0)
	{
		UtilsDelay(8000000*20);//Time to open UART port
		DBG_PRINT("HIB: Wake up on Power ON\n\r");

		verifyISL29035();
		configureISL29035(0);
		UART_PRINT("\n\rConfigured Light Sensor for wake up\n\r");

		CamControllerInit(); // Init parallel camera interface of cc3200
							// since image sensor needs XCLK for its I2C module to work
		CameraSensorInit();
		#ifdef ENABLE_JPEG
			//
			// Configure Sensor in Capture Mode
			//
			lRetVal = StartSensorInJpegMode();
			if(lRetVal < 0)
			{
				LOOP_FOREVER();
			}
		#endif
		UART_PRINT("I2C Camera config done\n");

		EnterHIBernate();
	}
	else if(MAP_PRCMSysResetCauseGet() == PRCM_HIB_EXIT)
	{
		DBG_PRINT("HIB: Woken up from Hibernate\n\r");
	}

  //
  // Simplelinkspawntask
  //
  lRetVal = VStartSimpleLinkSpawnTask(SPAWN_TASK_PRIORITY);
  if(lRetVal < 0)
  {
      ERR_PRINT(lRetVal);
      LOOP_FOREVER();
  }

  //
  // Create 'CameraService' task
  //
  lRetVal = osi_TaskCreate(CameraService,(signed char *) "CameraService",
                                             OSI_STACK_SIZE, NULL, 1, NULL);
  if(lRetVal < 0)
  {
      ERR_PRINT(lRetVal);
      LOOP_FOREVER();
  }

  osi_start();

  return 0;
}


//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************

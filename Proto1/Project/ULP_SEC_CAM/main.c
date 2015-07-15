//*****************************************************************************
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
// Application Name     - Get Weather
// Application Overview - Get Weather application connects to "openweathermap.org"
//                        server, requests for weather details of the specified
//                        city, process data and displays it on the Hyperterminal.
//
// Application Details  -
// http://processors.wiki.ti.com/index.php/CC32xx_Info_Center_Get_Weather_Application
// or
// docs\examples\CC32xx_Info_Center_Get_Weather_Application.pdf
//
//*****************************************************************************


//****************************************************************************
//
//! \addtogroup get_weather
//! @{
//
//****************************************************************************

// Standard includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// simplelink includes
#include "device.h"

// driverlib includes

#include "hw_types.h"
#include "hw_ints.h"
#include "interrupt.h"
#include "prcm.h"
#include "rom.h"
#include "rom_map.h"
//#include "timer.h"
#include "utils.h"
#include "pinmux.h"

#include "simplelink.h"
//Free_rtos/ti-rtos includes
#include "osi.h"

// common interface includes

#include "gpio_if.h"
#include "timer_if.h"
#include "udma_if.h"
#include "common.h"

#include "i2c_if.h"
#include "i2c_app.h"

#include "camera_app.h"
#include "network_related_fns.h"
#include "app_common.h"
//****************************************************************************
//                          LOCAL DEFINES                                   
//****************************************************************************
#define APPLICATION_VERSION     "1.1.0"
#define APP_NAME                "HD IMAGE, SENSOR DATA UPLOAD ON A/M TRIGGER"

#define SERVER_RESPONSE_TIMEOUT 10
#define SLEEP_TIME              8000000
#define SUCCESS                 0
//*****************************************************************************
//                 GLOBAL VARIABLES -- Start
//*****************************************************************************
unsigned long g_ulTimerInts;   //  Variable used in Timer Interrupt Handler
volatile unsigned char g_CaptureImage; //An app status
SlSecParams_t SecurityParams = {0};  // AP Security Parameters

//unsigned long g_image_buffer[NUM_OF_4B_CHUNKS]; //Appropriate name change to be done
unsigned long g_image_buffer[(IMAGE_BUF_SIZE_BYTES/sizeof(unsigned long))]; //Appropriate name change to be done

unsigned long  g_ulStatus = 0;//SimpleLink Status
unsigned long  g_ulGatewayIP = 0; //Network Gateway IP address
unsigned char  g_ucConnectionSSID[SSID_LEN_MAX+1]; //Connection SSID
unsigned char  g_ucConnectionBSSID[BSSID_LEN_MAX]; //Connection BSSID
signed char g_cWlanSSID[AP_SSID_LEN_MAX];
int g_uiSimplelinkRole = ROLE_INVALID;
signed char g_cWlanSecurityType[2];
signed char g_cWlanSecurityKey[50];
SlSecParams_t g_SecParams;
unsigned char g_ucConnectedToConfAP = 0, g_ucProvisioningDone = 0;
unsigned char g_ucPriority = 0;
unsigned char g_ucConfigsDone = 0;

OsiTaskHandle g_UserConfigTaskHandle = NULL ;
OsiTaskHandle g_MainTaskHandle = NULL ;
OsiTaskHandle g_ImageCaptureConfigs_TaskHandle = NULL ;

Sl_WlanNetworkEntry_t g_NetEntries[SCAN_TABLE_SIZE];
char g_token_get [TOKEN_ARRAY_SIZE][STRING_TOKEN_SIZE] = {"__SL_G_US0",
                                        "__SL_G_US1", "__SL_G_US2","__SL_G_US3",
                                                    "__SL_G_US4", "__SL_G_US5"};

#if defined(ccs)
extern void (* const g_pfnVectors[])(void);
#endif
#if defined(ewarm)
extern uVectorEntry __vector_table;
#endif
//*****************************************************************************
//                 GLOBAL VARIABLES -- End
//*****************************************************************************


//****************************************************************************
//                      LOCAL FUNCTION PROTOTYPES                           
//****************************************************************************
static void BoardInit();


extern void Main_Task(void *pvParameters);
extern void Main_Task_withHibernate(void *pvParameters);
extern void UserConfigure_Task(void *pvParameters);
extern void Test_Task(void *pvParameters);
extern void ImageSensor_CaptureConfigs_Task(void *pvParameters);

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
	UART_PRINT("Malloc Failed\n\r");
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
static void
BoardInit(void)
{
/* In case of TI-RTOS vector table is initialize by OS itself */
#ifndef USE_TIRTOS
  //
  // Set vector table base
  //
#if defined(ccs)
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

//****************************************************************************
//
//! Main function
//!
//! \param none
//! 
//! This function  
//!    1. Invokes the SLHost task
//!    2. Invokes the GetWeather Task
//!
//! \return None.
//
//****************************************************************************
void main()
{
	long lRetVal = -1;

	g_ulAppStatus = START;

    //
    // Initialize Board configurations
    //
    BoardInit();

    //
    // Pinmux for UART
    //
    PinMuxConfig();

#ifndef NOTERM
    //
    // Configuring UART
    //
    InitTerm();
    //UART_PRINT("Test");
#endif

	//Display Application Banner
	//
	//DisplayBanner(APP_NAME);

    //
    // I2C Init
    //
    I2C_IF_Open(I2C_APP_MODE);

    //
	// Initilalize DMA
	//
	UDMAInit();

	//
    // Start the SimpleLink Host - SpawnTask is needed for NWP asynch events
    //
    lRetVal = VStartSimpleLinkSpawnTask(SPAWN_TASK_PRIORITY);
    if(lRetVal < 0)
    {
        ERR_PRINT(lRetVal);
        LOOP_FOREVER();
    }

    //
	// Start the tasks
	//
	lRetVal = osi_TaskCreate(
					UserConfigure_Task,
					//Test_Task,
					(const signed char *)"User Config",
					OSI_STACK_SIZE,
					NULL,
					1,
					&g_UserConfigTaskHandle );
	if(lRetVal < 0)
	{
		ERR_PRINT(lRetVal);
		LOOP_FOREVER();
	}

    lRetVal = osi_TaskCreate(
					//Main_Task,
					Main_Task_withHibernate,
					//Test_Task,
					(const signed char *)"Collect And Txit ImgTempRM",
					OSI_STACK_SIZE,
					NULL,
					1,
					&g_MainTaskHandle );
	if(lRetVal < 0)
	{
		ERR_PRINT(lRetVal);
		LOOP_FOREVER();
	}

//    lRetVal = osi_TaskCreate(
//    				ImageSensor_CaptureConfigs_Task,
//					(const signed char *)"Collect And Txit ImgTempRM",
//					OSI_STACK_SIZE,
//					NULL,
//					1,
//					&g_ImageCaptureConfigs_TaskHandle );
//	if(lRetVal < 0)
//	{
//		ERR_PRINT(lRetVal);
//		LOOP_FOREVER();
//	}

    //
    // Start the task scheduler
    //
    osi_start();
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************

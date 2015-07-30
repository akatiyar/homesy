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

#include "appFns.h"
#include "hibernate_related_fns.h"
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

	//
	// Pinmux
	//
	PinMuxConfig();

#ifndef NOTERM
	//
	// Configuring UART
	//
	InitTerm();
	//UART_PRINT("Test");
#endif

	//
	// I2C Init
	//
	I2C_IF_Open(I2C_APP_MODE);

	//
	// Initilalize DMA
	//
	UDMAInit();
}
//*****************************************************************************
//
//! \brief This function initializes the application variables
//!
//! \param    None
//!
//! \return None
//!
//*****************************************************************************
static void InitializeAppVariables()
{
	g_ulAppStatus = START;
	g_I2CPeripheral_inUse_Flag = NEVER;
	g_Task3_Notification = 0;
	g_ulSimplelinkStatus = 0;

	g_TimeStamp_cc3200Up = 0;
	g_TimeStamp_NWPUp = 0;
	g_TimeStamp_CamUp = 0;
	g_TimeStamp_PhotoSnap = 0;
	g_TimeStamp_PhotoUploaded = 0;
	g_TimeStamp_DoorClosed = 0;
	g_TimeStamp_minAngle = 0;
	g_TimeStamp_maxAngle = 0;
	g_ucReasonForFailure = NEVER_WENT_TO_ANGLECHECK;
	g_fMinAngle = 361;
	g_fMaxAngle = 0;
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

	//
    // Initialize Board configurations
    //
    BoardInit();

    //
    // Initialize application variables
    //
    InitializeAppVariables();

    //Tag:Timestamp CC3200 up
    struct u64_time time_now;
	cc_rtc_get(&time_now);
	g_TimeStamp_cc3200Up = time_now.secs * 1000 + time_now.nsec / 1000000;

	if(Get_BatteryPercent() <= BATTERY_LOW_THRESHOLD)
	{
		UART_PRINT("Battery too low\n");

		PowerDown_LightSensor();
#ifndef  IOEXPANDER_PRESENT
		//Hibernate with no wake. Give a power reset i.e remove battery and put
		//it back (after charging)
		UART_PRINT("Hibernating...\n");
		HIBernate(NULL, NULL, NULL, NULL);
#else
		//To be used when IO expander is included in the ckt
		//Hibernate with GPIO wake enable interrupt
		HIBernate(ENABLE_GPIO_WAKESOURCE, FALL_EDGE, NULL, NULL);
#endif
	}

#ifdef IOEXPANDER_PRESENT
	if((!IsInterruptFromBatteryADC()) & (!IsInterruptFromLightSensor()))
	{
		ClearInterrupt_IOExpander();
		HIBernate(ENABLE_GPIO_WAKESOURCE, FALL_EDGE, NULL, NULL);
	}
#endif

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
    // UserConfigure_Task - Runs the functionality for Mobile App configuration
    //				of WiFi credentials, Magnetometer calibration, door angles'
    //				ecompass value, etc.
	lRetVal = osi_TaskCreate(
					UserConfigure_Task,
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

	// Main Task - Runs the major functionality of the application. It has two
	//				branches: One is executed only once at the time of power on
	//				(or when the firmware is loading first after the OTA). The
	//				other branch is executed every time the device wakes from
	//				hibernate. This branch includes angle detection, image
	//				capture and upload.
    lRetVal = osi_TaskCreate(
					//Main_Task,
					Main_Task_withHibernate,
					//Test_Task,
					(const signed char *)"Collect And Txit ImgTempRM",
					OSI_STACK_SIZE_MAIN_TASK,
					NULL,
					1,
					&g_MainTaskHandle );
	if(lRetVal < 0)
	{
		ERR_PRINT(lRetVal);
		LOOP_FOREVER();
	}

	// ImageSensor_CaptureConfigs_Task - Runs the register configuration of
	//				MT9D111 parallelly as the simplelink initializations are
	//				done in the Main Task parallelly.
    lRetVal = osi_TaskCreate(
    				ImageSensor_CaptureConfigs_Task,
					(const signed char *)"ImageSensor capture configs",
					OSI_STACK_SIZE,
					NULL,
					1,
					&g_ImageCaptureConfigs_TaskHandle );
	if(lRetVal < 0)
	{
		ERR_PRINT(lRetVal);
		LOOP_FOREVER();
	}

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

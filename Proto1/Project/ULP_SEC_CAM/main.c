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
#include "timer.h"
#include "utils.h"
#include "pinmux.h"

#include "simplelink.h"
//Free_rtos/ti-rtos includes
#include "osi.h"

// common interface includes

#include "gpio_if.h"
#include "timer_if.h"
//#include "network_if.h"
#include "udma_if.h"
#include "common.h"

#include "i2c_if.h"
#include "i2cconfig.h"

#include "accelomtrMagntomtr_fxos8700.h"
#include "tempRHSens_si7020.h"
#include "lightSens_isl29035.h"

#include "math.h"

#include "mt9d111.h"
#include "camera_app.h"
#include "network_related_fns.h"
#include "parse.h"
#include "parse_uploads.h"
#include "hibernate_related_fns.h"
#include "flash_files.h"
#include "app_common.h"
#include "timer_fns.h"
#include "wifi_provisioing_thruAPmode.h"

#include "test_modules.h"
#include "test_fns.h"
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
unsigned char g_ucProfileAdded = 1;
unsigned char g_ucConnectedToConfAP = 0, g_ucProvisioningDone = 0;
unsigned char g_ucPriority = 0;

Sl_WlanNetworkEntry_t g_NetEntries[SCAN_TABLE_SIZE];
char g_token_get [TOKEN_ARRAY_SIZE][STRING_TOKEN_SIZE] = {"__SL_G_US0",
                                        "__SL_G_US1", "__SL_G_US2","__SL_G_US3",
                                                    "__SL_G_US4", "__SL_G_US5"};
extern int32_t Collect_InitMangReadings();
extern int32_t WaitFor40Degrees();
int32_t Config_And_Start_CameraCapture();

extern void fxos_main();

extern int32_t fxos_get_initialYaw(float_t* pfInitYaw);
extern int32_t fxos_waitFor40Degrees(float_t fYaw_closedDoor);
extern int32_t fxos_calibrate();

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
static void DisplayBanner(char * AppName);


int32_t CollectTxit_ImgTempRH();
void Main_Task(void *pvParameters);
void Main_Task_withHibernate(void *pvParameters);
void ProvisionAP_Task(void *pvParameters);
void Test_Task(void *pvParameters);

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
//	Main Task
//
//	Initialize and hibernate. Wake on trigger and wait for 40degree closing
//	door condition and capture image and sensor data and upload to cloud
//
//*****************************************************************************
void Main_Task(void *pvParameters)
{
	LED_On();

	createAndWrite_ImageHeaderFile();
	create_JpegImageFile();

	Config_And_Start_CameraCapture();
	while(1)
	{
		CollectTxit_ImgTempRH();
	}
}

void Test_Task(void *pvParameters)
{
	//Config_And_Start_CameraCapture();
	while(1)
	{
		CollectTxit_ImgTempRH();
	}

	//verifyISL29035();
	//verifyTempRHSensor();
	//verifyAccelMagnSensor();
	//Config_And_Start_CameraCapture();
	//Verify_ImageSensor();
//	FXOS8700_Init();
//	float fMagnFlux[3];
//	while(1)
//	{
//		UtilsDelay(0.25*80000000/6);
//		getMagnFlux_3axis(fMagnFlux);
//		UART_PRINT("%f, %f, %f\n\r", fMagnFlux[0], fMagnFlux[1], fMagnFlux[2]);
//	}
//	while(1)
//		fxos_main();
}

void ProvisionAP_Task(void *pvParameters)
{
	LED_On();
	ProvisioningAP();
	while(1);
}
void Main_Task_withHibernate(void *pvParameters)
{
    if (MAP_PRCMSysResetCauseGet() == PRCM_POWER_ON)
	{
    	LED_Blink(30, 1);
		//LED_Blink(10, 1);
		LED_On();

		createAndWrite_ImageHeaderFile();
		create_JpegImageFile();

		Config_And_Start_CameraCapture();

		LED_Off();
		HIBernate(ENABLE_GPIO02_WAKESOURCE, FALL_EDGE, NULL);
	}
	if (MAP_PRCMSysResetCauseGet() == PRCM_HIB_EXIT)
	{
		UART_PRINT("\n\rI'm up\n\r");
		//LED_Blink(4, 0.3);
		LED_On();

		CollectTxit_ImgTempRH();
		LED_Off();
		HIBernate(ENABLE_GPIO02_WAKESOURCE, FALL_EDGE, NULL);
	}
}
int32_t Config_And_Start_CameraCapture()
{
	int32_t lRetVal;

	UART_PRINT("\n\rCAMERA MODULE:\n\r");
	CamControllerInit();	// Init parallel camera interface of cc3200
							// since image sensor needs XCLK for
							//its I2C module to work

	UtilsDelay(24/3 + 10);	// Initially, 24 clock cycles needed by MT9D111
							// 10 is margin

	SoftReset_ImageSensor();

	UART_PRINT("\n\rCam Sensor Init ");
	CameraSensorInit();

	// Configure Sensor in Capture Mode
	UART_PRINT("\n\rStart Sensor ");
	lRetVal = StartSensorInJpegMode();
	STOPHERE_ON_ERROR(lRetVal);

	disableAE();
	disableAWB();
	WriteAllAEnAWBRegs();

//	LL_Configs();

//	uint16_t x;
//	Variable_Read(0xA743, &x);
//	Variable_Read(0xA744, &x);
//	Variable_Read(0xA115, &x);
//	Variable_Read(0xA118, &x);
//
//	Variable_Read(0xA116, &x);
//	Variable_Read(0xA117, &x);

	UART_PRINT("I2C Camera config done\n\r");

	MAP_PRCMPeripheralReset(PRCM_CAMERA);
	MAP_PRCMPeripheralClkDisable(PRCM_CAMERA, PRCM_RUN_MODE_CLK);

	return lRetVal;
}
//*****************************************************************************
//
//! Application startup display on UART
//!
//! \param  none
//!
//! \return none
//!
//*****************************************************************************
static void
DisplayBanner(char * AppName)
{
    UART_PRINT("\n\n\n\r");
    UART_PRINT("********************************************************\n\r");
    UART_PRINT("%s\n\r", AppName);
    UART_PRINT("********************************************************\n\r");
    UART_PRINT("\n\n\n\r");
}

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
    // Start the SimpleLink Host
    //
    lRetVal = VStartSimpleLinkSpawnTask(SPAWN_TASK_PRIORITY);
    if(lRetVal < 0)
    {
        ERR_PRINT(lRetVal);
        LOOP_FOREVER();
    }

    //
    // Start the task
    //
	lRetVal = osi_TaskCreate(
					//Main_Task,
					//ProvisionAP_Task,
					Main_Task_withHibernate,
					//Test_Task,
					(const signed char *)"Collect And Txit ImgTempRM",
					OSI_STACK_SIZE,
					NULL,
					1,
					0 );
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

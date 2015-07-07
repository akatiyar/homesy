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
#include "batteryVoltLvlSens_adc081c021.h"
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
#include "ota.h"

#include "test_modules.h"
#include "test_fns.h"

#include "appFns.h"
#include "dbgFns.h"

#include "flc_api.h"
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

//unsigned long g_ulResetCause;

Sl_WlanNetworkEntry_t g_NetEntries[SCAN_TABLE_SIZE];
char g_token_get [TOKEN_ARRAY_SIZE][STRING_TOKEN_SIZE] = {"__SL_G_US0",
                                        "__SL_G_US1", "__SL_G_US2","__SL_G_US3",
                                                    "__SL_G_US4", "__SL_G_US5"};
extern int32_t Collect_InitMangReadings();
extern int32_t WaitFor40Degrees();
int32_t Config_And_Start_CameraCapture();
extern int32_t create_AngleValuesFile();

extern void fxos_main();

extern int32_t fxos_get_initialYaw(float_t* pfInitYaw);
extern int32_t fxos_waitFor40Degrees(float_t fYaw_closedDoor);
extern int32_t fxos_calibrate();

int16_t IsLightOff(uint16_t usThresholdLux);

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

int32_t CollectTxit_ImgTempRH();
void Main_Task(void *pvParameters);
void Main_Task_withHibernate(void *pvParameters);
void UserConfigure_Task(void *pvParameters);
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
//	Main Task
//
//	Initialize and hibernate. Wake on trigger and wait for 40degree closing
//	door condition and capture image and sensor data and upload to cloud
//
//*****************************************************************************
void Main_Task(void *pvParameters)
{
	UART_PRINT("Firmware 2\n\r");
	OTA_Update_2();
	//LED_Blink(30, 1);

	LED_On();
//	while(g_ulAppStatus == USER_CONFIG_TAKING_PLACE)
//	{
//		osi_Sleep(100);	//Wait if User Config is happening presently
//	}
//	osi_TaskDelete(&g_UserConfigTaskHandle);
	UART_PRINT("!!Application running!!\n\r");

	softResetTempRHSensor();
	configureTempRHSensor();

	createAndWrite_ImageHeaderFile();
	create_JpegImageFile();

	configureISL29035(0, LUX_THRESHOLD, LIGHTON_TRIGGER);	//Configures for reading Lux and wakeup interrupt

	Config_And_Start_CameraCapture();
	while(1)
	{
		CollectTxit_ImgTempRH();
	}
}

void Test_Task(void *pvParameters)
{
	Check_I2CDevices();
	while(1)
	{
		UART_PRINT("%d\n\r", Get_BatteryPercent());
		UtilsDelay(80000000);
	}
#ifdef COMPILE_I2CDEVICE_CHECK
	Check_I2CDevices();
#endif

//#define COMPILE_THIS_1
#ifdef COMPILE_THIS_1
	uint8_t ucADCVal;
	while(1)
	{
		Get_BatteryVoltageLevel_ADC081C021(&ucADCVal);
		UART_PRINT("%xH\n\r", ucADCVal);
		UART_PRINT("%d\n\r", Get_BatteryPercent());
	}
#endif

#ifdef COMPILE_THIS
	Config_And_Start_CameraCapture();
	while(1)
	{
		CollectTxit_ImgTempRH();
	}
#endif

#ifdef COMPILE_THIS
	UART_PRINT("%d\n",GPIOPinRead(GPIOA0_BASE, 0x04));
while(1)
{
	configureISL29035(0, LUX_THRESHOLD, LIGHTON_TRIGGER);
	getLightsensor_intrptStatus();
	UART_PRINT("Light supposedly off %d\n",GPIOPinRead(GPIOA0_BASE, 0x04));	//1
	while(GPIOPinRead(GPIOA0_BASE, 0x04));
	UART_PRINT("Light supposedly on %d\n",GPIOPinRead(GPIOA0_BASE, 0x04)); //0

	configureISL29035(0, LUX_THRESHOLD, LIGHTOFF_TRIGGER);
	getLightsensor_intrptStatus();
	UART_PRINT("Light supposedly on %d\n",GPIOPinRead(GPIOA0_BASE, 0x04));	//1
	while(GPIOPinRead(GPIOA0_BASE, 0x04));
	UART_PRINT("Light supposedly off %d\n",GPIOPinRead(GPIOA0_BASE, 0x04)); //0
}
#endif

#ifdef COMPILE_THIS
	while(1)
	{
		start_100mSecTimer();
		while(!(Elapsed_100MilliSecs == 100))
		{
		}
		stop_100mSecTimer();
	}
#endif

//	while(1)
//	{
//		InitializeTimer();
//		StartTimer();
//		UtilsDelay(5*80000000/6);
//	}

#ifdef COMPILE_THIS
	verifyISL29035();
	configureISL29035(0);
	uint16_t lux;
	while(1)
	{
		lux = getLightsensor_data();
		if(lux <= 5)
		{
			UART_PRINT("aaa\n");
		}
		if(IsLightOff(100))
		{
			UART_PRINT("ooo\n");
		}
	}
#endif

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

void UserConfigure_Task(void *pvParameters)
{
	//LED_On();
	g_ulAppStatus  = START;
	UtilsDelay(1000000);
	if ((MAP_PRCMSysResetCauseGet() == PRCM_POWER_ON) ||(MAP_PRCMSysResetCauseGet() == PRCM_SOC_RESET))
	{
		UART_PRINT("***SHORT PRESS-CONFIGURE THRU PHONE APP***\n\r"
						"***LONG PRESS-OTA***\n\r");
		while(GPIOPinRead(GPIOA1_BASE, GPIO_PIN_0))
		{
			osi_Sleep(20);
		}
		g_ulAppStatus = USER_CONFIG_TAKING_PLACE;	//includes OTA also for now
		//UART_PRINT("Switch Pressed\n\r");
		start_100mSecTimer();
		//while(1);
#define TIMEOUT_LONGPRESS		(30/6)	//3 sec press is defined as long press. Remove /6 if sys clk issue is resolved

		while(1)
		{
			if(Elapsed_100MilliSecs >= TIMEOUT_LONGPRESS)
			{
				UART_PRINT("Time out\n\r");
				break;
			}
			uint8_t i;
			for(i = 0; i < 3; i++)
			{
				if(GPIOPinRead(GPIOA1_BASE, GPIO_PIN_0))
				{
					UART_PRINT("pinhigh%d\n\r", i);
					UtilsDelay(12*80000/6);	//12 millisec delay
					//osi_Sleep(100);
				}
				else
				{
					//UART_PRINT("pinlow\n\r");
					break;
				}
			}
			if (i>=3)
			{
				UART_PRINT("pin high... short press\n\r");
				break;
			}

			//UtilsDelay(12*80000/6);	//12 millisec delay
		}
		stop_100mSecTimer();
		if(Elapsed_100MilliSecs >= TIMEOUT_LONGPRESS)
		{
			UART_PRINT("long press : %d\n\r", Elapsed_100MilliSecs);
			OTA_Update();
		}
		else if (Elapsed_100MilliSecs < TIMEOUT_LONGPRESS)
		{
			UART_PRINT("short press : %d\n\r", Elapsed_100MilliSecs);
			User_Configure();
			UART_PRINT("User Cofig Mode EXIT\n\r");
		}
		g_ulAppStatus = USER_CONFIG_DONE;
		while(1);
	}
	else
	{
		osi_TaskDelete(&g_UserConfigTaskHandle);
	}
}

void Main_Task_withHibernate(void *pvParameters)
{

    if ((MAP_PRCMSysResetCauseGet() == PRCM_POWER_ON)||(MAP_PRCMSysResetCauseGet() == PRCM_SOC_RESET))
	{
    	//Print the Purose of changing to this firmware here
    	UART_PRINT("*** F12 ***\n\r");
    	LED_Blink(30, 1);
		//LED_Blink(10, 1);
		LED_On();
		while(g_ulAppStatus == USER_CONFIG_TAKING_PLACE)	//Will exit if UserConfig is over or UserConfig was never entered
		{
			osi_Sleep(100);	//Wait if User Config is happening presently
		}
		osi_TaskDelete(&g_UserConfigTaskHandle);
		UART_PRINT("!!Application running!!\n\r");

		Check_I2CDevices();

		configureISL29035(0, LUX_THRESHOLD, LIGHTON_TRIGGER);	//Configures for reading Lux and wakeup interrupt
		softResetTempRHSensor();
		configureTempRHSensor();

		createAndWrite_ImageHeaderFile();
		create_JpegImageFile();

		Config_And_Start_CameraCapture();

		OTA_CommitImage();
	}
	if (MAP_PRCMSysResetCauseGet() == PRCM_HIB_EXIT)
	{
		UART_PRINT("*** F12 HIB ***\n\r");
		UART_PRINT("\n\rI'm up\n\r");
		LED_On();

		if(!IsLightOff(LUX_THRESHOLD))	//If condition met, it indicates that device was hibernated while light was on
		{
			CollectTxit_ImgTempRH();
		}
	}
	LED_Off();
	HIBernate(ENABLE_GPIO_WAKESOURCE, FALL_EDGE, NULL, NULL);
}
int32_t Config_And_Start_CameraCapture()
{
	int32_t lRetVal;

	UART_PRINT("\n\rConfig Camera:\n\r");
	CamControllerInit();	// Init parallel camera interface of cc3200
							// since image sensor needs XCLK for
							//its I2C module to work

	UtilsDelay(24/3 + 10);	// Initially, 24 clock cycles needed by MT9D111
							// 10 is margin

	SoftReset_ImageSensor();

	//UART_PRINT("\n\rCam Sensor Init ");
	CameraSensorInit();

	// Configure Sensor in Capture Mode
	//UART_PRINT("\n\rStart Sensor ");
	lRetVal = StartSensorInJpegMode();
	ASSERT_ON_ERROR(lRetVal);

//	uint16_t x;
	disableAE();
	disableAWB();
	WriteAllAEnAWBRegs();
	Refresh_mt9d111Firmware();

//	LL_Configs();

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
    // Start the SimpleLink Host
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

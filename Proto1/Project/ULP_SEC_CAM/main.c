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

#include "test_fns.h"
//****************************************************************************
//                          LOCAL DEFINES                                   
//****************************************************************************
#define APPLICATION_VERSION     "1.1.0"
#define APP_NAME                "VGA IMAGE, SENSOR DATA UPLOAD TO PARSE ON LIGHT TRIGGER"

#define SERVER_RESPONSE_TIMEOUT 10
#define SLEEP_TIME              8000000
#define SUCCESS                 0

#define PREFIX_BUFFER "GET /data/2.5/weather?q="
#define POST_BUFFER "&mode=xml&units=imperial HTTP/1.1\r\nHost: api.openweathermap.org\r\nAccept: */"
#define POST_BUFFER2 "*\r\n\r\n"

//*****************************************************************************
//                 GLOBAL VARIABLES -- Start
//*****************************************************************************
unsigned long g_ulTimerInts;   //  Variable used in Timer Interrupt Handler
volatile unsigned char g_CaptureImage; //An app status
SlSecParams_t SecurityParams = {0};  // AP Security Parameters

unsigned long g_image_buffer[NUM_OF_4B_CHUNKS]; //Appropriate name change to be done

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


void CollectTxit_ImgTempRH(void *pvParameters);

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


void CollectTxit_ImgTempRH(void *pvParameters)
{

	CaptureImage();

	ConfigureSimpleLinkToDefaultState();
	ConnectToNetwork_STA();

	ParseClient clientHandle;
	clientHandle = InitialiseParse();

	uint8_t ucParseImageUrl[100];
	memset(ucParseImageUrl,NULL, 100);
	UploadImageToParse(clientHandle, (unsigned char*) USER_FILE_NAME, ucParseImageUrl);

	UART_PRINT("\n%s\n", ucParseImageUrl);

	/*//
	//	Collect Temperature and RH values from Si7020 IC
	//
	float_t fTemp, fRH;
	verifyTempRHSensor();
	softResetTempRHSensor();
	configureTempRHSensor();
	getTempRH(&fTemp, &fRH);


	uint8_t ucSensorDataTxt[DEVICE_STATE_OBJECT_SIZE]; // Device state JSON object
	memset(ucSensorDataTxt, '\0', DEVICE_STATE_OBJECT_SIZE);
	ucSensorDataTxt[0] = NULL;

	//
	// Construct the JSON object string
	//
	uint8_t ucCharConv[20];
	strncat(ucSensorDataTxt, "{\"DeviceID\":\"", sizeof("{\"DeviceID\":\""));
	strncat(ucSensorDataTxt, VCOGNITION_DEVICE_ID, sizeof("VCOGNITION_DEVICE_ID"));
	strncat(ucSensorDataTxt, "\",\"Temperature\":\"", sizeof("\",\"Temperature\":\""));
	intToASCII((uint16_t)(fTemp*100), ucCharConv);
	strncat(ucSensorDataTxt, ucCharConv, 2);
	strncat(ucSensorDataTxt, ".", 1);
	strncat(ucSensorDataTxt, ucCharConv + 2, 2);
	strncat(ucSensorDataTxt, "\",\"RH\":\"", sizeof("\",\"RH\":\""));
	intToASCII((uint16_t)(fRH*100), ucCharConv);
	strncat(ucSensorDataTxt, ucCharConv, 2);
	strncat(ucSensorDataTxt, ".", 1);
	strncat(ucSensorDataTxt, ucCharConv+2, 2);
	strncat(ucSensorDataTxt, "\"}", sizeof("\"}"""));
	UART_PRINT("\n%s\n", ucSensorDataTxt);

	UploadSensorDataToParse(clientHandle, ucSensorDataTxt);*/

//	CollectSensorData();
//	txitSensorData(clientHandle,"",ucParseImageUrl);


#ifndef USB_DEBUG

	uint16_t temp = getLightsensor_data();

	while(temp > 0x01FF)
	{
		temp = getLightsensor_data();
		UtilsDelay(80000000/3);
	}

//	while(!(GPIOPinRead(GPIOA0_BASE, 0x02)));
	temp = sl_Stop(0xFFFF);
	if(temp<0)
		UART_PRINT("\nConnection close error");
	HIBernate();
#else
	while(1);
#endif
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
#endif
    //
    // I2C Init
    //
    I2C_IF_Open(I2C_APP_MODE);

    if(MAP_PRCMSysResetCauseGet() == 0)
	{
#ifndef USB_DEBUG
		UtilsDelay(8000000*10);//Time to open UART port
#endif
		//
    	//Display Application Banner
		//
		DisplayBanner(APP_NAME);

		DBG_PRINT("HIB: Wake up on Power ON\n\r");

		UART_PRINT("\n\rLIGHT SENSOR:\n\r");
		verifyISL29035();
		configureISL29035(0);
		UART_PRINT("Configured Light Sensor for wake up\n\r");

		UART_PRINT("\n\rCAMERA MODULE:\n\r");
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
		UART_PRINT("I2C Camera config done\n\r");
#ifndef USB_DEBUG
		HIBernate();
#endif
	}
	else if(MAP_PRCMSysResetCauseGet() == PRCM_HIB_EXIT)
	{
		DBG_PRINT("\n\r\n\rHIB: Woken up from Hibernate\n\r");

//		DBG_PRINT("\n\rACCELMTR, MAGNTMTR:\n\r");
//		float_t fDoorDirectionAngle, fAccel;
//		verifyAccelMagnSensor();
//		FXOS8700CQ_Mag_Calibration();
//		configureFXOS8700(0);
//
//		while(1)
//		{
//			getDoorDirection( &fDoorDirectionAngle );
//			DBG_PRINT("Angle: %f\n\r", fDoorDirectionAngle);
////			if( fDoorDirectionAngle > 40 )
////				break;
//			UtilsDelay(80000000*.001);
//		}
//		DBG_PRINT("Angle condition satisfied\n\r");
	}

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
	lRetVal = osi_TaskCreate(CollectTxit_ImgTempRH,
					//TestSi7020ApiReadWrite,
					//TestFxosApiReadWrite,
					(const signed char *)"Collect And Txit ImgTempRM",
					OSI_STACK_SIZE,
					NULL,
					1,
					NULL );
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

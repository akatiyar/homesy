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


void CollectTxit_ImgTempRH();
void Main_Task(void *pvParameters);

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
//	The function captures image and sensor data and uploads to Parse
//
//	param none
//
//	return none
//
//*****************************************************************************
void CollectTxit_ImgTempRH()
{
#ifndef NO_CAMERA
	//
	//	Captute image and save in flash
	//
	CaptureAndStore_Image();
#endif
	//
	//	Connect cc3200 to wifi AP
	//
	ConfigureSimpleLinkToDefaultState();
	ConnectToNetwork_STA();

	//
	//	Parse initialization
	//
	ParseClient clientHandle;
	clientHandle = InitialiseParse();

	//
	//	Upload image to Parse and retreive image's unique ID
	//
	uint8_t ucParseImageUrl[100];
	memset(ucParseImageUrl,NULL, 100);
	UploadImageToParse(clientHandle,
						(unsigned char*) USER_FILE_NAME,
						ucParseImageUrl);
	UART_PRINT("\n%s\n", ucParseImageUrl);

	//
	//	Collect Temperature and RH values from Si7020 IC
	//
	float_t fTemp = 12.34, fRH = 56.78;
//	verifyTempRHSensor();
//	softResetTempRHSensor();
//	configureTempRHSensor();
//	UtilsDelay(80000000);
//	getTempRH(&fTemp, &fRH);

	//
	// Construct the JSON object string
	//
	uint8_t ucSensorDataTxt[DEVICE_STATE_OBJECT_SIZE]; // DeviceState JSONobject
	memset(ucSensorDataTxt, '\0', DEVICE_STATE_OBJECT_SIZE);
	ConstructDeviceStateObject(ucParseImageUrl, fTemp, fRH, ucSensorDataTxt);

	//
	//	Upload sensor data to Parse
	//
	UploadSensorDataToParse(clientHandle, ucSensorDataTxt);

//	CollectSensorData();
//	txitSensorData(clientHandle,"",ucParseImageUrl);
}

//*****************************************************************************
//
//	Main Task
//
//	Initialize and hibernate. Wake on trigger and wait for 40degree closing
//	door condition and capture image and sensor data and upload to cloud
//
//	The logic used to detect '40 degrees while door is closing':
//		Opening of door beyond 45 degrees is detected. After that, we check
//	when the door angle becomes less than 40 degrees
//
//*****************************************************************************
void Main_Task(void *pvParameters)
{
	long lRetVal = -1;

    if(MAP_PRCMSysResetCauseGet() == 0)
	{
#ifndef USB_DEBUG
		UtilsDelay(8000000*10);//Time to open UART port and close the
#endif
		UART_PRINT("HIB: Wake up on Power ON\n\r");
		UART_PRINT("***PLEASE KEEP THE DOOR CLOSED***\n\r");

/*		//
		// Collect the Initial Magnetometr Readings(door closed)
		//
		UART_PRINT("\n\rMAGNETOMETER:\n\r");
		verifyAccelMagnSensor();
		configureFXOS8700(MODE_READ_ACCEL_MAGN_DATA);
		//DBG
		float_t fInitDoorDirectionAngle;
		getDoorDirection( &fInitDoorDirectionAngle );
		DBG_PRINT("DBG: Testing Magnetometer working:\n"\
					" Angle: %f\n\r", fInitDoorDirectionAngle);

		float_t fMagnFlx_Init[3];
		float_t fMFluxMagnitudeInit = 0;
		uint8_t j;
		for (j = 0; j < MOVING_AVG_FLTR_L; j++)
		{
			getMagnFlux_3axis(fMagnFlx_Init);

			fMFluxMagnitudeInit += sqrtf( (fMagnFlx_Init[0]*fMagnFlx_Init[0]) +
												(fMagnFlx_Init[1]*fMagnFlx_Init[1]) +
												(fMagnFlx_Init[2]*fMagnFlx_Init[2]) );
		}
		fMFluxMagnitudeInit /= MOVING_AVG_FLTR_L;

		//
		// Save initial magnetometer readings in flash
		//
		lRetVal = sl_Start(0, 0, 0);
		//ASSERT_ON_ERROR(lRetVal);

		lRetVal = CreateFile_Flash((uint8_t*)MAGN_INIT_VALS_FILE_NAME, MAGN_INIT_VALS_FILE_MAXSIZE);
		lRetVal = WriteFile_ToFlash((uint8_t*)fMagnFlx_Init, (uint8_t*)MAGN_INIT_VALS_FILE_NAME, 12, 0 );

	    lRetVal = sl_Stop(0xFFFF);
	    //lRetVal = sl_Stop(SL_STOP_TIMEOUT);
		//ASSERT_ON_ERROR(lRetVal);
*/

/*
		//
		// Set up the camera module through I2C
		//
#ifndef NO_CAMERA
		UART_PRINT("\n\rCAMERA MODULE:\n\r");
		CamControllerInit();	// Init parallel camera interface of cc3200
								// since image sensor needs XCLK for
								//its I2C module to work
		CameraSensorInit();
		#ifdef ENABLE_JPEG
			// Configure Sensor in Capture Mode
			lRetVal = StartSensorInJpegMode();
			if(lRetVal < 0)
			{
				LOOP_FOREVER();
			}
		#endif
		UART_PRINT("I2C Camera config done\n\r");

	    MAP_PRCMPeripheralReset(PRCM_CAMERA);
	    MAP_PRCMPeripheralClkDisable(PRCM_CAMERA, PRCM_RUN_MODE_CLK);
#endif
*/

#ifndef USB_DEBUG
		//
		// Set up the trigger source and enter hibernate
		//
		HIBernate();
#endif
	}
#ifdef USB_DEBUG
/*    sensorsTriggerSetup();
	while(GPIOPinRead(GPIOA0_BASE, 0x04))
	{
	}
*/
#else
    else if(MAP_PRCMSysResetCauseGet() == PRCM_HIB_EXIT)
#endif
	{
#ifndef USB_DEBUG
		long lRetVal;
#endif
		DBG_PRINT("\n\r\n\rHIB: Woken up from Hibernate\n\r");

/*		float_t fMagnFluxInit3Axis_Read[3];

		//
		// Read initial magnetometer readings from flash
		//
		lRetVal = sl_Start(0, 0, 0);
		//ASSERT_ON_ERROR(lRetVal);

		lRetVal = ReadFile_FromFlash((uint8_t*)fMagnFluxInit3Axis_Read, (uint8_t*)MAGN_INIT_VALS_FILE_NAME, 12, 0 );

		lRetVal = sl_Stop(0xFFFF);
		//lRetVal = sl_Stop(SL_STOP_TIMEOUT);
		//ASSERT_ON_ERROR(lRetVal);

		//
		// Calculate magnitude corresponding to 40 and 45 degrees
		//
		float_t fMFluxMagnitudeInit;
		fMFluxMagnitudeInit = sqrtf( (fMagnFluxInit3Axis_Read[0]*fMagnFluxInit3Axis_Read[0]) +
									(fMagnFluxInit3Axis_Read[1]*fMagnFluxInit3Axis_Read[1]) +
									(fMagnFluxInit3Axis_Read[2]*fMagnFluxInit3Axis_Read[2]) );
		UART_PRINT("DBG: Initial Magnitude: %f\n\r", fMFluxMagnitudeInit);
		float_t fThreshold_magnitudeOfDiff, fThreshold_higher_magnitudeOfDiff;
		fThreshold_magnitudeOfDiff = 2 * fMFluxMagnitudeInit * sinf( (DOOR_ANGLE_TO_DETECT/2) * PI / 180 );
//		fThreshold_magnitudeOfDiff *= fThreshold_magnitudeOfDiff;
		fThreshold_higher_magnitudeOfDiff = 2 * fMFluxMagnitudeInit * sinf( (DOOR_ANGLE_HIGHER/2) * PI / 180);
//		fThreshold_higher_magnitudeOfDiff *= fThreshold_higher_magnitudeOfDiff;
		UART_PRINT("DBG: Threshold Magnitude: %f   %f\n\r", fThreshold_magnitudeOfDiff, fThreshold_higher_magnitudeOfDiff);

		//
		// Configure sensor for reading accelerometer/magnetometer
		//
		configureFXOS8700(MODE_READ_ACCEL_MAGN_DATA);

		//
		// Wait for '40 degrees while closing condition'
		//
		float_t fMagnFlx_Now[3];
		float_t fMagnFlx_Dif[3];
		float_t fMFluxMagnitudeArray[MOVING_AVG_FLTR_L];
		float_t fMFluxMagnitude, fMFluxMagnitudePrev = 0;
		memset(fMFluxMagnitudeArray, '0', MOVING_AVG_FLTR_L * sizeof(float_t));
		uint8_t i = 0, j;
		uint8_t ucOpenFlag = 0;
		uint8_t ucFirstTimeFlag = 1;
		while(1)
		{
			getMagnFlux_3axis(fMagnFlx_Now);

			fMagnFlx_Dif[0] = fMagnFluxInit3Axis_Read[0] - fMagnFlx_Now[0];
			fMagnFlx_Dif[1] = fMagnFluxInit3Axis_Read[1] - fMagnFlx_Now[1];
			fMagnFlx_Dif[2] = fMagnFluxInit3Axis_Read[2] - fMagnFlx_Now[2];
			fMFluxMagnitudeArray[i++] = sqrtf( (fMagnFlx_Dif[0] * fMagnFlx_Dif[0]) +
										(fMagnFlx_Dif[1] * fMagnFlx_Dif[1]) +
										(fMagnFlx_Dif[2] * fMagnFlx_Dif[2]) );
//			fMFluxMagnitudeArray[i++] = ( (fMagnFlx_Dif[0] * fMagnFlx_Dif[0]) +
//										(fMagnFlx_Dif[1] * fMagnFlx_Dif[1]) +
//										(fMagnFlx_Dif[2] * fMagnFlx_Dif[2]) );
			i = i % MOVING_AVG_FLTR_L;
//			UART_PRINT("%f\n\r", fMFluxMagnitudeInit);

			float_t fDoorDirectionAngle;
			getDoorDirection( &fDoorDirectionAngle );

			fMFluxMagnitude = 0;
			for (j = 0; j < MOVING_AVG_FLTR_L; j++)
			{
				fMFluxMagnitude += fMFluxMagnitudeArray[j];
			}
			if( ucFirstTimeFlag == 0 )
			{
				fMFluxMagnitude /= (i);
				if (i == 0)
				{
					ucFirstTimeFlag = 1;
				}
			}
			else
			{
				fMFluxMagnitude /= MOVING_AVG_FLTR_L;
			}

			float_t fAngle;
			fAngle = 2 * asinf((fMFluxMagnitude/2)/fMFluxMagnitudeInit) *180/PI;
//			UART_PRINT("%f, ", fMFluxMagnitude);
//			UART_PRINT("%f\n", fAngle);

			if( ucOpenFlag == 0 )
			{
				UART_PRINT("0");
				if( fMFluxMagnitude > fThreshold_higher_magnitudeOfDiff )
				{
					ucOpenFlag = 1;
					UART_PRINT("Open Detected");
				}
			}
			else
			{
				UART_PRINT("@");
				if( fMFluxMagnitude < fThreshold_magnitudeOfDiff )
				{
					UART_PRINT("Door at 40 degres while closing");
					UART_PRINT("\n Angle =  %f\nMagnitude = %f\n",fAngle,fMFluxMagnitude);
//					ucOpenFlag = 0; //To repeat DGB
					break;

				}
			}
			UtilsDelay(30000);
		}
*/
		//
		// Collect and transmit Image and sensor data
		//
		CollectTxit_ImgTempRH();

#ifndef USB_DEBUG
//	uint16_t temp = getLightsensor_data();
//
//	while(temp > 0x01FF)
//	{
//		temp = getLightsensor_data();
//		UtilsDelay(80000000/3);
//	}

	//
	// Stop simplelink
	//
	UtilsDelay(80000000);
	int16_t temp;
	temp = sl_Stop(0xFFFF);
	if( temp < 0)
		UART_PRINT("\nConnection close error");

	//
	// Setup wake sources and hibernate
	//
	HIBernate();
#else
	clearAccelMotionIntrpt();
	configureFXOS8700(MODE_ACCEL_INTERRUPT);
	UART_PRINT("Configured Accelerometer for wake up\n\r");
	while(1);
#endif
	}
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

    //
	// Initilalize DMA
	//
	UDMAInit();

    //
	//Display Application Banner
	//
	DisplayBanner(APP_NAME);

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
	lRetVal = osi_TaskCreate(Main_Task,
					//TestSi7020ApiReadWrite,
					//TestFxosApiReadWrite,
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

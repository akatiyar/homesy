#include "app.h"
#include "parse_uploads.h"
#include "app_common.h"

#include "stdlib.h"
#include "math.h"
#include "string.h"

#include "camera_app.h"
#include "network_related_fns.h"
#include "mt9d111.h"
#include "tempRHSens_si7020.h"
#include "accelomtrMagntomtr_fxos8700.h"

#include "flash_files.h"
#include "appFns.h"
#include "timer_fns.h"

#include "stdbool.h"
bool g_tempflag=true;

extern int32_t g_lFileHandle;
extern int32_t SendGroundData();


int32_t application_fn()
{
    long lRetVal;
    long lFileHandle;
	unsigned long ulToken = NULL;
    struct u64_time time_now;
    uint32_t ulTimeDuration_ms;

    uint8_t ucSensorDataTxt[DEVICE_STATE_OBJECT_SIZE]; // DeviceState JSONobject
	ParseClient clientHandle = NULL;
	ParseClient clientHandle1 = NULL;
	uint8_t ucParseImageUrl[PARSE_IMAGE_URL_SIZE];
	float_t fTemp = 12.34, fRH = 56.78;
	uint8_t ucBatteryLvl = 80;
	uint8_t ucFridgeCamID[FRIDGECAM_ID_SIZE];

	//This flag is necessary since parallel task also needs to access the
	//I2C peripheral. The flag is set before IsLightOff() and reset on
	//returning from IsLightOff(). Flag is not switched back in case where
	//the if condition is not met since the device hibernates anyways
	g_I2CPeripheral_inUse_Flag = YES;

	//Only if light is found to be on, follow the sequence of detecting
	//angle, image capture and upload. If Condition is NOT met (ie light
	//is off), it indicates that device hibernated while light was on to
	//save power. So, just hibernate after setting light sensor registers
	//trigger on light on.
	if(!IsLightOff(LUX_THRESHOLD))
	{
		//* * * * * * * * * Wake-up Initializations * * * * * * * * *
		//Task3 also runs parallelly, doing some of the initializations.
		//Initializations done on this task:
		//	1. NWP on
		//	2. Magnetometer flash file reading
		//	3. File open
		//	4. Wait for Task3 to finish
		//Initializations done on Task3:
		//	1. MT9D111 wakeup, start capture
		//	2. Magnetometer I2C configuration
		//	3. Few intitial angle reads

		g_I2CPeripheral_inUse_Flag = NO;

		start_100mSecTimer();	//Tag:Remove when waketime optimization is over

		// Start SimpleLink - NWP ON - required for all flash operations
		lRetVal = NWP_SwitchOn();
		ASSERT_ON_ERROR(lRetVal);

		//g_tempflag=false;
		//Initialize magnetometer for angle calculation
		angleCheck_Initializations();

		g_Task3_Notification = READ_MAGNTMTRFILE_DONE;

		// Open Image file in Flash to write image. File open for write takes
		//time, so we do it ahead of angle check, so that at the instant angle
		//is detected, image can be clicked
		UART_PRINT("b Fileopen\n\r");	//Tag:Remove when waketime optimization is over
		lRetVal = sl_FsOpen((unsigned char *)JPEG_IMAGE_FILE_NAME,
						   FS_MODE_OPEN_WRITE, &ulToken, &lFileHandle);
		UART_PRINT("a Fileopen\n\r");	//Tag:Remove when waketime optimization is over
		if(lRetVal < 0)
		{
		   sl_FsClose(lFileHandle, 0, 0, 0); ASSERT_ON_ERROR(lRetVal);
		}

		//Tag:Timestamp NWP up
		cc_rtc_get(&time_now);
		g_TimeStamp_NWPUp = time_now.secs * 1000 + time_now.nsec / 1000000;

//Use the folowing code to test without hibernate
/*
		// Wake the image sensor and begin image capture
		Wakeup_ImageSensor();
		ReStart_CameraCapture();
		ImagCapture_Init();				//Initialize image capture
		//Tag:Timestamp Camera module up
		cc_rtc_get(&time_now);
		g_TimeStamp_CamUp = time_now.secs * 1000 + time_now.nsec / 1000000;
		magnetometer_initialize();
		g_I2CPeripheral_inUse_Flag = NO;
		g_Task3_Notification = MAGNTMTRINIT_DONE;
*/

		while(g_Task3_Notification != MAGNTMTRINIT_DONE)
		{
			UART_PRINT("#");
			osi_Sleep(10);
		}

		ulTimeDuration_ms = get_timeDuration();
		stop_100mSecTimer();
		UART_PRINT("Total Wake Time - %d ms\n\r", ulTimeDuration_ms);

		//* * * * * * * * * End of Wake-up Initializations * * * * * * * * *

		//NOTE: For Ground data upload to Parse only.
		//Check once more for light. If light is off, then upload the GroundData object and exit function
		if(IsLightOff(LUX_THRESHOLD))
		{
			lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
			ASSERT_ON_ERROR(lRetVal);

			lRetVal = NWP_SwitchOff();
			ASSERT_ON_ERROR(lRetVal);

			//Tag:Upload GroundData object
			SendGroundData();

			g_ulAppStatus = LIGHT_IS_OFF_BEFORE_IMAGING;
			Standby_ImageSensor();
			return g_ulAppStatus;
		}

		g_ucReasonForFailure = NOTOPEN_NOTCLOSED;

  		start_100mSecTimer();	//For Timeout detection
		//sensorsTriggerSetup(); //Tag:Remove DGB Wake Time Profile
		while(1)
		{
			//angleCheck();
			get_angle();
			check_doorpos();

			if(g_flag_door_closing_45degree)
			{
				g_ulAppStatus = IMAGING_POSITION_DETECTED;
				LED_On();
				break;
			}

			//Cases where we have to abort and hibernate
			if(checkForLight_Flag)	//This flag Goes up once every 100mSec
			{
				//Light is off
				if(IsLightOff(LUX_THRESHOLD))
				{
					g_ulAppStatus = LIGHT_IS_OFF_BEFORE_IMAGING;
					break;
				}
				//Timeout
				if(Elapsed_100MilliSecs > (DOORCHECK_TIMEOUT_SEC * 10))
				{
					g_ulAppStatus = TIMEOUT_BEFORE_IMAGING;
					g_ucReasonForFailure = TIMEOUT_BEFORE_IMAGESNAP;
					UART_PRINT("Timeout\n\r");
					break;
				}
			}
		}
		standby_accelMagn_fxos8700();
		stop_100mSecTimer();

		g_ulAppStatus = IMAGING_POSITION_DETECTED;

		// Capture image if Imaging position is detected
		if(g_ulAppStatus == IMAGING_POSITION_DETECTED)
		{
			g_ucReasonForFailure = IMAGE_NOTCAPTURED;
			start_1Sec_TimeoutTimer();	//To timeout incase image capture is not successful
			captureTimeout_Flag = 0;
			lRetVal = CaptureImage(lFileHandle);
			if(captureTimeout_Flag)
			{
				//NOTE: For Ground data upload to Parse only.
				lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
				ASSERT_ON_ERROR(lRetVal);
				lRetVal = NWP_SwitchOff();
				ASSERT_ON_ERROR(lRetVal);

				//Tag:Upload GroundData object
				SendGroundData();

				//Read MT9D111 register statuses. View in UART PRINTs
				Read_AllRegisters();
				CCMRegs_Read();
				PCLK_Rate_read();

				PRCMSOCReset();
			}
			stop_1Sec_TimeoutTimer();

			//Tag:Timestamp photo click
			cc_rtc_get(&time_now);
			g_TimeStamp_PhotoSnap = time_now.secs * 1000 + time_now.nsec / 1000000;

			g_ulAppStatus = IMAGE_CAPTURED;
			g_ucReasonForFailure = IMAGE_NOTUPLOADED;
		}

		// Close the file after writing the image into it
	    lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
	    ASSERT_ON_ERROR(lRetVal);

	    //ReadFile_FromFlash((char*)(g_image_buffer+20), (char*)JPEG_IMAGE_FILE_NAME, uiImageFile_Offset, 0);
	    lRetVal = NWP_SwitchOff();
		ASSERT_ON_ERROR(lRetVal);

		Standby_ImageSensor();	//Put the Image Sensor in standby

		//Upload only if image was capture
		if(g_ulAppStatus == IMAGE_CAPTURED)
		{
			UART_PRINT("Inside if\n");	//Tag:Rm

  			//Connect to WiFi
			lRetVal = WiFi_Connect();
			if (lRetVal < 0)
			{
				NWP_SwitchOff();
				ASSERT_ON_ERROR(lRetVal);
			}

			//	Parse initialization
			clientHandle = InitialiseParse();

			//	Upload image to Parse and retreive image's unique ID
			lRetVal = UploadImageToParse(clientHandle,
											(unsigned char*) JPEG_IMAGE_FILE_NAME,
											&ucParseImageUrl[0]);

			//	Collect and Upload sensor data to Parse
			if (lRetVal >= 0)	//Check whether image upload was successful
			{
				getTempRH(&fTemp, &fRH);	//Collect Temperature and RH values from
															//Si7020 IC
				ucBatteryLvl = Get_BatteryPercent();	//Get Battery Level from ADC

				Get_FridgeCamID(&ucFridgeCamID[0]);	//Get FridgeCam ID from unique MAC
													//ID of the CC3200 device

				//	Upload sensor data to Parse
				lRetVal = UploadSensorDataToParse(clientHandle,
								&ucFridgeCamID[0], &ucParseImageUrl[0], fTemp,
								fRH, ucBatteryLvl, &ucSensorDataTxt[0]);
				PRINT_ON_ERROR(lRetVal);
				if(lRetVal >= 0)
				{
					g_ucReasonForFailure = SUCCESS;
					//Tag:Timestamp
					cc_rtc_get(&time_now);
					g_TimeStamp_PhotoUploaded = time_now.secs * 1000 + time_now.nsec / 1000000;
				}
			}

			//Uncomment if not collecting GroundData
			/*//	Free the memory allocated for clientHandle in InitialiseParse()
			free((void*)clientHandle);

			sl_Stop(0xFFFF);	//sl_start() in WiFi_Connect()
			CLR_STATUS_BIT(g_ulSimplelinkStatus, STATUS_BIT_NWP_INIT);*/

		}
		UART_PRINT("After if\n");	//Tag:Rm

		//Tag:Timestamp door close/30sec timeout/door still open but image was uploaded - use a light on
		//Tag:Upload GroundData object
		if(IsLightOff(LUX_THRESHOLD))
		{
			cc_rtc_get(&time_now);
			g_TimeStamp_DoorClosed = time_now.secs * 1000 + time_now.nsec / 1000000;
		}

		g_TimeStamp_maxAngle = g_Struct_TimeStamp_MaxAngle.secs * 1000 + g_Struct_TimeStamp_MaxAngle.nsec / 1000000;
		g_TimeStamp_minAngle = g_Struct_TimeStamp_MinAngle.secs * 1000 + g_Struct_TimeStamp_MinAngle.nsec / 1000000;

		if(clientHandle == NULL)
		{
			//Connect to WiFi
			lRetVal = WiFi_Connect();
			if (lRetVal < 0)
			{
				NWP_SwitchOff();
				ASSERT_ON_ERROR(lRetVal);
			}

			//	Parse initialization
			clientHandle1 = InitialiseParse();

			Get_FridgeCamID(&ucFridgeCamID[0]);	//Get FridgeCam ID from unique MAC
												//ID of the CC3200 device

			UploadGroundDataObjectToParse(clientHandle1, &ucFridgeCamID[0]);

			free((void*)clientHandle1);	//malloc() in InitializeParse()
		}
		else
		{
			UploadGroundDataObjectToParse(clientHandle, &ucFridgeCamID[0]);

			free((void*)clientHandle);	//malloc() in InitializeParse()
		}

		//free((void*)clientHandle_1);	//malloc() in InitializeParse()
		NWP_SwitchOff();
	}

	return lRetVal;
}


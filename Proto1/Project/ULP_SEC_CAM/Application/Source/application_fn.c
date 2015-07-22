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

int32_t application_fn()
{
    long lRetVal;
    long lFileHandle;
	unsigned long ulToken = NULL;

    uint8_t ucSensorDataTxt[DEVICE_STATE_OBJECT_SIZE]; // DeviceState JSONobject
	ParseClient clientHandle;
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
		g_I2CPeripheral_inUse_Flag = NO;

		start_100mSecTimer();	//Tag:Remove when waketime optimization is over

		// Start SimpleLink - NWP ON - required for all flash operations
		UART_PRINT("b sl_start\n\r");	//Tag:Remove when waketime optimization is over
		lRetVal = sl_Start(0, 0, 0);
		UART_PRINT("a sl_start\n\r");	//Tag:Remove when waketime optimization is over
		ASSERT_ON_ERROR(lRetVal);

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

		//Initialize image capture
		ImagCapture_Init();

		//Initialize magnetometer for angle calculation
		angleCheck_Initializations();

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
					UART_PRINT("Timeout\n\r");
					break;
				}
			}
		}
		standby_accelMagn_fxos8700();
		stop_100mSecTimer();

		// Capture image if Imaging position is detected
		if(g_ulAppStatus == IMAGING_POSITION_DETECTED)
		{
			start_1Sec_TimeoutTimer();	//To timeout incase image capture is not successful
			lRetVal = CaptureImage(lFileHandle);
			stop_1Sec_TimeoutTimer();

			g_ulAppStatus = IMAGE_CAPTURED;
		}

		// Close the file after writing the image into it
	    lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
	    ASSERT_ON_ERROR(lRetVal);

	    //ReadFile_FromFlash((char*)(g_image_buffer+20), (char*)JPEG_IMAGE_FILE_NAME, uiImageFile_Offset, 0);
	    lRetVal = sl_Stop(0xFFFF);
		ASSERT_ON_ERROR(lRetVal);

		Standby_ImageSensor();	//Put the Image Sensor in standby

		//Upload only if image was capture
		if(g_ulAppStatus == IMAGE_CAPTURED)
		{
			UART_PRINT("Inside if\n");	//Tag:Rm
			lRetVal = WiFi_Connect();
			if (lRetVal < 0)
			{
				sl_Stop(0xFFFF);
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
			}

			//	Free the memory allocated for clientHandle in InitialiseParse()
			free((void*)clientHandle);

			sl_Stop(0xFFFF);	//sl_start() in WiFi_Connect()
		}
		UART_PRINT("After if\n");	//Tag:Rm
	}

	return lRetVal;
}


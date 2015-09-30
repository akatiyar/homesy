#include "stdlib.h"
#include "math.h"
#include "string.h"
#include "stdbool.h"

#include <app_fns.h>
#include "app.h"
#include "app_common.h"

#include "camera_app.h"
#include "parse_uploads.h"
#include "network_related_fns.h"
#include "mt9d111.h"
#include "timer_fns.h"
#include <temp_rh_sens_si7020.h>
#include <accelomtr_magntomtr_fxos8700.h>

//******************************************************************************
// 	The main application function
//
//	It does the following main functions:
//	1. Checks for door snap position continuously
//	2. Captures an image and uploads it to Parse
//	3. Collects and uploads Temperature, RH and Battery charge percent to Parse
//	4. Collects and uploads ground data
// 	5. Wake-up initializations
//
//	This function is the main function that is run each time the device wakes
//from hibernate.
//
//	NOTE: This function is tightly coupled with ImageCaptureConfig task
//******************************************************************************
int32_t application_fn()
{
    long lRetVal;
    long lFileHandle;
	unsigned long ulToken = NULL;
    struct u64_time time_now;

    uint8_t ucSensorDataTxt[DEVICE_STATE_OBJECT_SIZE]; // DeviceState JSONobject
	ParseClient clientHandle = NULL;
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
		//Task3 also runs parallelly, doing some of the initializations. NWP
		// related init steps are done by this task. I2C related init steps are
		// done by task3
		//Initializations done by this task:
		//	1. NWP on
		//	2. Magnetometer configs flash file reading
		//	3. Images sensor configs flash file reading
		//	4. Image File in flash open
		//	5. Wait for Task3 to finish
		//Initializations done by Task3:
		//	1. MT9D111 wakeup, start capture
		//	2. Magnetometer I2C configuration
		//	3. Few intitial angle reads

		g_I2CPeripheral_inUse_Flag = NO;	//Both tasks should not try to
											//access I2C at a time

		start_100mSecTimer();	//@Remove when waketime optimization is over

		// Start SimpleLink - NWP ON - required for all flash operations
		lRetVal = NWP_SwitchOn();
		RETURN_ON_ERROR(lRetVal);

		//Initialize magnetometer for angle calculation
		angleCheck_Initializations();
		start_periodicInterrupt_timer(2.5);	//Once started, it is stopped on
											//exiting the angle check loop

		g_Task3_Notification = READ_MAGNTMTRFILE_DONE; //Indication to task3

#ifdef USB_DEBUG	//Debugger will not work with CC3200 hibernate
					//So, to do task3 functionalities in this this task itself
		g_Task1_Notification = MAGNETOMETERINIT_STARTED;
		magnetometer_initialize();
		uint16_t* ImageConfigData = (uint16_t *) g_image_buffer;
		ImageConfigData = ImageConfigData + (OFFSET_MT9D111/sizeof(uint16_t));
#endif

		//Read image sensor configurations from the flash sensor configs file
		ReadFile_FromFlash((uint8_t*)g_image_buffer,
							(uint8_t*)FILENAME_SENSORCONFIGS,
							CONTENT_LENGTH_SENSORS_CONFIGS, 0);

		//Wait for indication from task3 before changing notification to Task3
		while(g_Task1_Notification != MAGNETOMETERINIT_STARTED);
		g_Task3_Notification = READ_SENSORCONFIGFILE_DONE; //Indication to task3

#ifdef USB_DEBUG
		Wakeup_ImageSensor();		//Wake the image sensor
		ReStart_CameraCapture(ImageConfigData);	//Restart image capture
		//Initialize image capture
		memset((void*)g_image_buffer, 0x00, IMAGE_BUF_SIZE_BYTES);
		ImagCapture_Init();
		//Use this if you're debugging image
		g_flag_door_closing_45degree = 1;
#endif

		// Open Image file in Flash to write image. File open for write takes
		//time, so we do it ahead of angle check, so that at the instant angle
		//is detected, image can be clicked
		g_Task1_Notification = IMAGEFILE_OPEN_BEGUN;
		DEBG_PRINT("b Fileopen\n");	//@Remove when waketime optimization is over
		lRetVal = sl_FsOpen((unsigned char *)JPEG_IMAGE_FILE_NAME,
						   FS_MODE_OPEN_WRITE, &ulToken, &lFileHandle);
		if(lRetVal < 0)
		{
		   sl_FsClose(lFileHandle, 0, 0, 0); RETURN_ON_ERROR(lRetVal);
		}
		DEBG_PRINT("a Fileopen\n");	//@Remove when waketime optimizatn is over
		g_Task1_Notification = IMAGEFILE_OPEN_COMPLETE;

		cc_rtc_get(&time_now);
		g_TimeStamp_NWPUp = time_now.secs * 1000 + time_now.nsec / 1000000;

#ifndef USB_DEBUG
		while(g_I2CPeripheral_inUse_Flag == YES)
		{
			DEBG_PRINT("&");
			osi_Sleep(1);
		}
#endif

		//NOTE: For Ground data upload to Parse only.
		//Check once more for light. If light is off, then upload the
		//GroundData object and exit function
		if((IsLightOff(LUX_THRESHOLD)) || (g_ucReasonForFailure == DOOR_ATSNAP_DURING_FILEOPEN))
		{
			lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
			RETURN_ON_ERROR(lRetVal);

			lRetVal = NWP_SwitchOff();
			RETURN_ON_ERROR(lRetVal);

			//Upload GroundData object
			LED_Blink_2(.2,1,BLINK_FOREVER);
			//g_ucReasonForFailure will either be DOOR_SHUT_DURING_FILEOPEN or
			//NOTOPEN_NOTCLOSED or OPEN_NOTCLOSED
			SendObject_ToParse(GROUND_DATA);

			g_ulAppStatus = LIGHT_IS_OFF_BEFORE_IMAGING;
			Standby_ImageSensor();

			return g_ulAppStatus;
		}

		g_ucReasonForFailure = NOTOPEN_NOTCLOSED;

#ifndef USB_DEBUG
		while(g_Task1_Notification != TIMERS_DISABLED)
		{
			DEBG_PRINT("$");
		}
#endif
		//* * * * * * * * * End of Wake-up Initializations * * * * * * * * *

#ifdef IMAGE_DEBUG	//To debug image without magnetometer functionality
		g_flag_door_closing_45degree = 1;
#endif

		start_100mSecTimer();	//For timeout detection - door open for too long
		//sensorsTriggerSetup(); //Use to Profile Wake-up Time using CRO
		while(1)
		{
			get_angle();
			check_doorpos();

			if(g_flag_door_closing_45degree)
			{
				g_ulAppStatus = IMAGING_POSITION_DETECTED;
				break;
			}

			//Cases where we have to abort angle checking and hibernate
			if(checkForLight_Flag)	//This flag goes up once every 100mSec
			{
				//Light is off
				if(IsLightOff(LUX_THRESHOLD))
				{
					g_ulAppStatus = LIGHT_IS_OFF_BEFORE_IMAGING;
					RELEASE_PRINT("Lightout\n");
					break;
				}
				//Timeout
				if(Elapsed_100MilliSecs > (DOORCHECK_TIMEOUT_SEC * 10))
				{
					g_ulAppStatus = TIMEOUT_BEFORE_IMAGING;
					g_ucReasonForFailure = TIMEOUT_BEFORE_IMAGESNAP;
					RELEASE_PRINT("Timeout\n");
					break;
				}
				//Check for button press to let the user enter UserConfig mode
				// or do OTA
				if(IS_PUSHBUTTON_PRESSED)
				{
					RELEASE_PRINT("Resetting...\n");
					Reset_byStarvingWDT();
				}
			}
		}
		standby_accelMagn_fxos8700();
		stop_100mSecTimer();
		stop_periodicInterrupt_timer();
		LED_Blink_2(.2,1,BLINK_FOREVER);

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
				RETURN_ON_ERROR(lRetVal);
				lRetVal = NWP_SwitchOff();
				RETURN_ON_ERROR(lRetVal);

				//Upload GroundData object
				SendObject_ToParse(GROUND_DATA);

				//Read MT9D111 register statuses. View in UART PRINTs
				Read_AllRegisters();
				CCMRegs_Read();
				PCLK_Rate_read();

				LED_Blink_2(.2,.2,5);
				osi_Sleep(.2*2*5*1000);
				PRCMSOCReset();	//Should be replaced by SYSOFF reset
			}
			stop_1Sec_TimeoutTimer();

			g_ulAppStatus = IMAGE_CAPTURED;
			g_ucReasonForFailure = IMAGE_NOTUPLOADED;
		}

		// Close the file after writing the image into it
	    lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
	    RETURN_ON_ERROR(lRetVal);

	    lRetVal = NWP_SwitchOff();
		RETURN_ON_ERROR(lRetVal);

		Standby_ImageSensor();	//Put the Image Sensor in standby

		//Upload only if image was capture
		if(g_ulAppStatus == IMAGE_CAPTURED)
		{
  			//Connect to WiFi
			lRetVal = WiFi_Connect();
			if (lRetVal < 0)
			{
				NWP_SwitchOff();
				RETURN_ON_ERROR(lRetVal);
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
				getTempRH(&fTemp, &fRH);	//Collect Temperature and RH values
											//from Si7020 IC
				ucBatteryLvl = Get_BatteryPercent();	//Get Battery Level
														//from ADC
				Get_FridgeCamID(&ucFridgeCamID[0]);	//Get FridgeCam ID from
													//unique MAC ID of the
													//CC3200 device

				//	Upload sensor data to Parse
				lRetVal = UploadSensorDataToParse(clientHandle,
								&ucFridgeCamID[0], &ucParseImageUrl[0], fTemp,
								fRH, ucBatteryLvl, &ucSensorDataTxt[0]);
				if(lRetVal >= 0)
				{
					g_ucReasonForFailure = SUCCESS;
					cc_rtc_get(&time_now);
					g_TimeStamp_PhotoUploaded = time_now.secs * 1000 + time_now.nsec / 1000000;
				}
			}

			free((void*)clientHandle);	//malloc() in InitializeParse()
			NWP_SwitchOff();
		}

#ifndef IMAGE_DEBUG
		SendObject_ToParse(GROUND_DATA);
#endif
	}

	return lRetVal;
}


#include "stdbool.h"

#include <app_fns.h>
#include "app_common.h"

#include "network_related_fns.h"
#include "ota.h"
#include "ecompass.h"
#include "watchdog.h"
#include "timer_fns.h"
#include "camera_app.h"
#include "mt9d111.h"
#include "flash_files.h"

#define CONFIG_MODE_USER_ACTION_TIMEOUT		600	//in units of 100ms
												//1 minute = 1*60*10 = 600

extern bool g_bCameraOn;
uint16_t g_shutterwidth=0;
uint16_t g_Gain[4];
float_t* g_pfUserConfigData = (float_t*) &g_image_buffer[(USER_CONFIGS_OFFSET_BUF)/sizeof(long)];

int32_t SaveImageConfig();
int32_t SaveUserConfigs();
void InitializeUserConfigVariables();	//Shift fn to suitable place

//******************************************************************************
// This function
//		1. calls the function corresponding to the button pressed by the user
//	in the app (button presses are translated to global flag values in
//	http_server_callback). Buttons can be
//			a) Magnetometer calibration
//			b) Angle buttons
//			c) WiFi credentials
//			d) Image AWB on / off
//			e) Gain(s) / Integration time alter
//			f) Preview
//		2. It stores the configuration and exits on press of exit button.
//		3. NWP on, putting device in AP mode, beginning HTTP server, stopping
//	HTTP server, NWP off are also handled inside the function.
//		4. Checks for push-button press continuously. If pressed quits
//	config mode
//	return success or error code
//******************************************************************************
int32_t User_Configure()
{
	long lRetVal = -1;
	bool run_flag=true;
	int32_t lFileHandle;
	bool bCapturePreviewImage= false;
	uint16_t tempReg=0;
	uint16_t tempCnt=0;

	//
	//Initialize app timeout to USERCONFIG_TIMEOUT so that the user gets
	//USERCONFIG_TIMEOUT amount of time to do the configurations
	//
	g_ulWatchdogCycles = 0;
	g_ulAppTimeout_ms = USERCONFIG_TIMEOUT;

	InitializeUserConfigVariables();

	AccessPtMode_HTTPServer_Start();

	// Initialize the camera
	Wakeup_ImageSensor();
	SoftReset_ImageSensor();
	Config_CameraCapture();
	Start_CameraCapture();	//Do this once. Not needed after standby wake_up

	//User Message
	if(!g_PhoneConnected_ToCC3200AP_flag)
	{
		RELEASE_PRINT("Please connect phone to CC3200-AP\n");
	}

	//Reading the file, since write erases contents already present
	ReadFile_FromFlash((uint8_t*)g_pfUserConfigData,
						(uint8_t*)USER_CONFIGS_FILENAME,
						CONTENT_LENGTH_USER_CONFIGS, 0);

	start_100mSecTimer();	//The timer does not timeout on it's own. We check
							//the elapsed time and break the while loop on timeout

	//Wait till one of these happen:
	//	1. phone connect to cc3200 AP
	//	2. push button on hardware is pressed - to abort Config mode and exit
	//	3. timeout
	while(1)
	{
		if(IS_PUSHBUTTON_PRESSED || g_PhoneConnected_ToCC3200AP_flag)
		{
			break;
		}
		if((Elapsed_100MilliSecs > CONFIG_MODE_USER_ACTION_TIMEOUT))
		{
			stop_100mSecTimer();	//Stop the timeout timer.
			RELEASE_PRINT("Timeout... No phone connected\n");
			return 0;	//Gotta replace this by run_flag = false
		}
		osi_Sleep(10);
	}

	//Message to user
	if(g_PhoneConnected_ToCC3200AP_flag)
	{
		RELEASE_PRINT("Please open mobile app to configure\n");
	}

	LED_Blink_2(.5,.5,BLINK_FOREVER);

	stop_100mSecTimer();	//Stop the timeout timer.

	while(run_flag == true)
    {
		//Abort the UserConfig mode if the user presses the button
		//If user had configured something before pressing thebutton,
		//it will not be saved.
		if(IS_PUSHBUTTON_PRESSED)
		{
			RELEASE_PRINT("Exit button pressed\n");
			LED_On();	//Indicating to user
			run_flag = false;
			//break;
		}

		if(g_ucCalibration == BUTTON_PRESSED)
		{
			RELEASE_PRINT("*** Calibration - ROTATE DEVICE TILL LED STARTS BLINKING/GOES OFF ***\n");
			LED_On();
			g_ucCalibration = BUTTON_NOT_PRESSED;
			Get_Calibration_MagSensor();
			//We save so that the angle mmts user is likely to do immediately
			//will be based on the new calibration
			SaveUserConfigs();
			standby_accelMagn_fxos8700();
			LED_Blink_2(0.5,0.5,BLINK_FOREVER);
			//Elapsed_100MilliSecs = 0;	//Resetting the timer count
    	}
		if(g_ucAngle40 == BUTTON_PRESSED)
		{
			LED_On();
			CollectAngle(ANGLE_40);
			standby_accelMagn_fxos8700();
			g_ucAngle40 = BUTTON_NOT_PRESSED;
			LED_Blink_2(0.5,0.5,BLINK_FOREVER);
			//Elapsed_100MilliSecs = 0;	//Resetting the timer count
		}

		if(g_ucAngle90 == BUTTON_PRESSED)
		{
			LED_On();
			CollectAngle(ANGLE_90);
			standby_accelMagn_fxos8700();
			g_ucAngle90 = BUTTON_NOT_PRESSED;
			LED_Blink_2(0.5,0.5,BLINK_FOREVER);
			//Elapsed_100MilliSecs = 0;	//Resetting the timer count
		}

		if(g_ucConfig == BUTTON_PRESSED)
		{
			LED_On();
			WiFiProvisioning();
			g_ucConfig = BUTTON_NOT_PRESSED;
			LED_Blink_2(0.5,0.5,BLINK_FOREVER);
			//Elapsed_100MilliSecs = 0;	//Resetting the timer count
		}

		if(g_ucAWBOn == BUTTON_PRESSED)
		{
			LED_On();
			RELEASE_PRINT("AWB on\n");
//			//Refresh_mt9d111Firmware();
//			osi_Sleep(100);
			enableAWB();
		//	enableAE();
			Variable_Read(0xA102,&tempReg);
			DEBG_PRINT("Reg %x : %x",0xA102,tempReg);

			//Enable AWB in capture mode
			Variable_Write(0xA120,0x22);
			Variable_Read(0xA120,&tempReg);
			DEBG_PRINT("Reg %x : %x",0xA120,tempReg);

			Refresh_mt9d111Firmware();

			g_ucAWBOn = BUTTON_NOT_PRESSED;
		}

		if(g_ucAWBOff == BUTTON_PRESSED)
		{
			RELEASE_PRINT("AWB off\n");
//			Variable_Write(0xA912,0x00);
//			Refresh_mt9d111Firmware();
			disableAWB();
			Variable_Read(0xA102,&tempReg);
			DEBG_PRINT("Reg %x : %x",0xA102,tempReg);

			//Disable AWB in capture mode
			Variable_Write(0xA120,0x02);
			Variable_Read(0xA120,&tempReg);
			DEBG_PRINT("Reg %x : %x",0xA120,tempReg);

			Refresh_mt9d111Firmware();

			g_ucAWBOff = BUTTON_NOT_PRESSED;
			LED_Blink_2(0.5,0.5,BLINK_FOREVER);
		}

		if(g_ucPreviewStart == BUTTON_PRESSED)
		{
			RELEASE_PRINT("Preview On\n");
			//    Wakeup_ImageSensor();

			// Set the IMage size to 640x480

			// Enable boolean to capture Image and save

			lRetVal = sl_FsOpen((unsigned char *)JPEG_PREVIEW_IMAGE_FILE_NAME,
					FS_MODE_OPEN_CREATE(40959, _FS_FILE_OPEN_FLAG_COMMIT | _FS_FILE_PUBLIC_WRITE),
					NULL, (_i32*)&lFileHandle);
			if(lRetVal<0)
			{
				PRINT_ON_ERROR(lRetVal);
			}
			sl_FsClose(lFileHandle,NULL,NULL,NULL);

			uint16_t temp;
			Reg_Read(0, 0x20, &temp);
			DEBG_PRINT("0x20: %x\n",temp);
			Reg_Read(0, 0x03, &temp);
			DEBG_PRINT("0x03: %x\n",temp);

			bCapturePreviewImage = true;
			tempCnt=0;
			g_ucPreviewStart = BUTTON_NOT_PRESSED;
		}

		if(g_ucPreviewStop == BUTTON_PRESSED)
		{
			RELEASE_PRINT("Preview Off\n");
			lRetVal = sl_FsDel(JPEG_PREVIEW_IMAGE_FILE_NAME,NULL);
			if(lRetVal<0)
			{
				PRINT_ON_ERROR(lRetVal);
			}
			bCapturePreviewImage = false;
			g_ucPreviewStop = BUTTON_NOT_PRESSED;
			// Enable HD Image
		}

		if(g_ucActionButton == BUTTON_PRESSED)
		{
			LED_On();
			if(g_ucAction == CAM_RESTART_CAPTURE)
			{
				Restart_Camera();
				Variable_Write(0x2707,640);
				Variable_Write(0x2709,480);

				Refresh_mt9d111Firmware();
				osi_Sleep(200);
			}
			else if (g_ucAction == OTA_FIRMWARE_UPDATE)
			{
				LED_Blink_2(.2,.2,BLINK_FOREVER);
				//We save so that if the user has jst configured WiFi
				//credentials, it would be used for the OTA
				SaveUserConfigs();
				OTA_Update();

				//If OTA update was successful, CC3200 is reset so that the
				//firmware can be run. Therefore, following lines of code are
				//not executed. The user has to enter UserConfig again by
				//resetting
				//If no updates are available / the OTA was a failure, CC3200
				//should come back to AP mode
				InitializeUserConfigVariables();
				AccessPtMode_HTTPServer_Start();
			}
			g_ucAction = NONE;
			g_ucActionButton = BUTTON_NOT_PRESSED;
			LED_Blink_2(0.5,0.5,BLINK_FOREVER);
		}

		if(g_ucSAVE == BUTTON_PRESSED)
		{
			LED_On();
			SaveImageConfig();
		//	Read_AllRegisters();
			g_ucSAVE = BUTTON_NOT_PRESSED;
			LED_Blink_2(0.5,0.5,BLINK_FOREVER);
		}

		if(bCapturePreviewImage)
		{
			osi_Sleep(211);
			DEBG_PRINT("%d",tempCnt++);
			CaptureandSavePreviewImage();
		}

		if((g_ucExitButton == BUTTON_PRESSED)/*||(Elapsed_100MilliSecs > CONFIG_MODE_USER_ACTION_TIMEOUT)*/)
		{
			LED_On();
			RELEASE_PRINT("Exiting Config Mode\n");
			SaveUserConfigs();

			/*//Verification
			for(i=0;i<15;i++)
			{
				fAngle = 0;
				ReadFile_FromFlash((uint8_t*)&fAngle, (uint8_t*)USER_CONFIGS_FILENAME, sizeof(float), (i)*sizeof(float));
				DEBG_PRINT("%d:%3.2f\n\r",i,fAngle);
			}
			// For verification - DBG
			lRetVal = ReadFile_FromFlash(((uint8_t*)g_pfUserConfigData+3),
											(uint8_t*)USER_CONFIGS_FILENAME,
											CONTENT_LENGTH_USER_CONFIGS,
											0);*/

			g_ucExitButton = BUTTON_NOT_PRESSED;
			run_flag = false;
		}

		osi_Sleep(10);
    }

	//Stop Internal HTTP Server
	lRetVal = sl_NetAppStop(SL_NET_APP_HTTP_SERVER_ID);
	if(lRetVal < 0)
	{
		ERR_PRINT(lRetVal);
		LOOP_FOREVER();
	}

	if(ConfigureMode(ROLE_STA) !=ROLE_STA)
	{
		RETURN_ON_ERROR(ROLE_STA_ERR);
	}

	Restart_Camera();
	ReadGainReg(g_Gain);
	Reg_Read(0x00,0x09,&g_shutterwidth);
	Standby_ImageSensor();

	NWP_SwitchOff();

    return SUCCESS;
}

//****************************************************************************
//	Save the Image senor configs into flash file
//****************************************************************************
int32_t SaveImageConfig()
{
	int32_t lFileHandle;
	int32_t lRetVal;
	uint16_t* ImageConfigData = (uint16_t *) g_image_buffer;
	ImageConfigData = ImageConfigData + 4096;
	// Reading the file, since write erases contents already present
	ReadFile_FromFlash((uint8_t*)ImageConfigData,
						(uint8_t*)FILENAME_SENSORCONFIGS,
						CONTENT_LENGTH_SENSORS_CONFIGS, 0);

	ReadImageConfigReg(ImageConfigData + (OFFSET_MT9D111/sizeof(uint16_t)));

	lRetVal = WriteFile_ToFlash((uint8_t*)ImageConfigData,
							(uint8_t*)FILENAME_SENSORCONFIGS,
							CONTENT_LENGTH_SENSORS_CONFIGS, 0,
							SINGLE_WRITE, &lFileHandle);
	RETURN_ON_ERROR(lRetVal);
	//ReadAllAEnAWBRegs();
	return lRetVal;
}

//****************************************************************************
//	Save the user configs (magnetometer calib, 40, 90 angles wifi credentials)
//into flash file
//****************************************************************************
int32_t SaveUserConfigs()
{
	int32_t lRetVal;
	int32_t lFileHandle;

	DEBG_PRINT("saving user configs\n");	//Temp - for testing
	*(g_pfUserConfigData + (OFFSET_ANGLE_OPEN/sizeof(float))) = Calculate_DoorOpenThresholdAngle(*(g_pfUserConfigData + (OFFSET_ANGLE_40/sizeof(float))),*(g_pfUserConfigData + (OFFSET_ANGLE_90/sizeof(float))));
	//Write all the User Config contents into the flash
	lRetVal = WriteFile_ToFlash((uint8_t*)g_pfUserConfigData,
							(uint8_t*)USER_CONFIGS_FILENAME,
							CONTENT_LENGTH_USER_CONFIGS, 0,
							SINGLE_WRITE, &lFileHandle);
	PRINT_ON_ERROR(lRetVal);

	return lRetVal;
}

//****************************************************************************
//
//!    \brief This function initializes the application variables
//!
//!    \param[in]  None
//!
//!    \return     0 on success, negative error-code on error
//
//****************************************************************************
void InitializeUserConfigVariables()
{
	g_ulSimplelinkStatus = 0;
    g_ulGatewayIP = 0;
    memset(g_ucConnectionSSID,0,sizeof(g_ucConnectionSSID));
    memset(g_ucConnectionBSSID,0,sizeof(g_ucConnectionBSSID));

    g_ucProfileAdded = 0;
    g_ucConnectedToConfAP = 0;
	g_ucProvisioningDone = 0;
	g_PhoneConnected_ToCC3200AP_flag = 0;

	g_ucAngle90 = BUTTON_NOT_PRESSED;
	g_ucAngle40 = BUTTON_NOT_PRESSED;
	g_ucExitButton = BUTTON_NOT_PRESSED;
	g_ucConfig= BUTTON_NOT_PRESSED;
	g_ucCalibration= BUTTON_NOT_PRESSED;
	g_ucPushButtonPressedTwice = BUTTON_NOT_PRESSED;
}

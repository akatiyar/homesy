//#include "wlan.h"
#include "include_all.h"
#include "network_related_fns.h"
#include "flash_files.h"
#include "simplejson.h"
#include "camera_app.h"
#include "stdbool.h"
#include "ota.h"
#include "accelomtrMagntomtr_fxos8700.h"
#include "app_common.h"
#include "appFns.h"
#include "watchdog.h"
#include "timer_fns.h"
#include "mt9d111.h"

#define ANGLE_90		0
#define ANGLE_40		1

extern int gdoor_90deg_angle;//290
extern int gdoor_close_angle; //110
extern int gdoor_snap_angle;//120
struct MagCalibration thisMagCal;
//extern unsigned long g_image_buffer[(IMAGE_BUF_SIZE_BYTES/sizeof(unsigned long))];
volatile uint8_t g_ucMagCalb;
extern bool g_bCameraOn;

int32_t create_AngleValuesFile();
extern float_t get_angle();
extern int16_t angleCheck_Initializations();
extern int16_t angleCheck_WithCalibration();
extern int16_t magnetometer_initialize();

int16_t fxos_Calibration();
int16_t fxosDefault_Initializations();

static int32_t AccessPtMode_HTTPServer_Start();
static int32_t WiFiProvisioning();
static long WlanConnect();
static int32_t Get_Calibration_MagSensor();
static int32_t CollectAngle(uint8_t ucAngle);
int32_t SaveImageConfig();
uint16_t g_shutterwidth=0;
uint16_t g_Gain[4];

#define CONFIG_MODE_USER_ACTION_TIMEOUT		600	//in units of 100ms
												//1 minute = 1*60*10 = 600


float_t* g_pfUserConfigData = (float_t*) &g_image_buffer[(USER_CONFIGS_OFFSET_BUF)/sizeof(long)];

//******************************************************************************
// This function calls the function corresponding to button presses (button
//	presses are translated to global flag changes in network_related_fns.c) from
//	the mobile app.
//		It exits on press of exit button.
//		NWP on, putting device in AP mode, beginning HTTP server, stopping HTTP
//	server, NWP off are also handled inside the function.
//
//	return success or error code
//
//	note: if sl_Start() had already been called and sl_Stop() was not,
//			sl_Start() will take longer	to return
//******************************************************************************
int32_t User_Configure()
{
	long lRetVal = -1;
	bool run_flag=true;
	int32_t lFileHandle;
	bool bCapturePreviewImage= false;
	uint16_t tempReg=0;
	uint16_t tempCnt=0;

	//So the user gets USERCONFIG_TIMEOUT amount of time to do the configurations
	g_ulWatchdogCycles = 0;
	g_ulAppTimeout_ms = USERCONFIG_TIMEOUT;

	// Initialize the camera
	//if(g_bCameraOn == false)
	{
		Wakeup_ImageSensor();
	}

	SoftReset_ImageSensor();
	Config_CameraCapture();
	Start_CameraCapture();	//Do this once. Not needed after standby wake_up
//	Standby_ImageSensor();

//	g_pfUserConfigData = (float_t*)malloc(CONTENT_LENGTH_USER_CONFIGS);

	AccessPtMode_HTTPServer_Start();
	// Reading the file, since write erases contents already present
	ReadFile_FromFlash((uint8_t*)g_pfUserConfigData,
						(uint8_t*)USER_CONFIGS_FILENAME,
						CONTENT_LENGTH_USER_CONFIGS, 0);

	start_100mSecTimer();	//The timer does not timeout on it's own. We check
							//the elapsed time and break the while loop on timeout

	//Wait till phone connect to cc3200 AP or till push button is pressed or
	//timeout happens
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
			return 0;
		}
		osi_Sleep(10);
	}

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
			RELEASE_PRINT("*** Calibration - ROTATE DEVICE NOW ***\n");
			LED_On();
			Get_Calibration_MagSensor();
			standby_accelMagn_fxos8700();
			g_ucCalibration = BUTTON_NOT_PRESSED;
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
			if(g_ucAction ==  CAM_RESTART_CAPTURE)
			{
				Restart_Camera();
				Variable_Write(0x2707,640);
				Variable_Write(0x2709,480);

				Refresh_mt9d111Firmware();
				osi_Sleep(200);
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
			*(g_pfUserConfigData + (OFFSET_ANGLE_OPEN/sizeof(float))) = Calculate_DoorOpenThresholdAngle(*(g_pfUserConfigData + (OFFSET_ANGLE_40/sizeof(float))),*(g_pfUserConfigData + (OFFSET_ANGLE_90/sizeof(float))));
			//Write all the User Config contents into the flash
			lRetVal = WriteFile_ToFlash((uint8_t*)g_pfUserConfigData,
									(uint8_t*)USER_CONFIGS_FILENAME,
									CONTENT_LENGTH_USER_CONFIGS, 0,
									SINGLE_WRITE, &lFileHandle);
			ASSERT_ON_ERROR(lRetVal);

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

	//free(g_pfUserConfigData);
	//Stop Internal HTTP Server
	lRetVal = sl_NetAppStop(SL_NET_APP_HTTP_SERVER_ID);
	if(lRetVal < 0)
	{
		ERR_PRINT(lRetVal);
		LOOP_FOREVER();
	}

	if(ConfigureMode(ROLE_STA) !=ROLE_STA)
	{
		ASSERT_ON_ERROR(ROLE_STA_ERR);
	}

	Restart_Camera();

	ReadGainReg(g_Gain);

	Reg_Read(0x00,0x09,&g_shutterwidth);
	Standby_ImageSensor();

	NWP_SwitchOff();

    return SUCCESS;
}

//******************************************************************************
//	Puts CC3200 in AP mode and starts its HTTP server
//******************************************************************************
static int32_t AccessPtMode_HTTPServer_Start()
{
	int32_t lRetVal = -1;
	unsigned char ucFridgeCamID[50];
	unsigned short  length;

	InitializeUserConfigVariables();

	//
	// Following function configure the device to default state by cleaning
	// the persistent settings stored in NVMEM (viz. connection profiles &
	// policies, power policy etc)
	//
	// Applications may choose to skip this step if the developer is sure
	// that the device is in its default state at start of applicaton
	//
	// Note that all profiles and persistent settings that were done on the
	// device will be lost
	//
   lRetVal = ConfigureSimpleLinkToDefaultState();
   if(lRetVal < 0)
   {
	   if (DEVICE_NOT_IN_STATION_MODE == lRetVal)
	   {
		   DEBG_PRINT("Failed to configure NWP in default state\n");
	   }

	   LOOP_FOREVER();
   }
	//DEBG_PRINT("Device is configured in default state \n\r");

	//	Start NWP
	//lRetVal = sl_Start(0, 0, 0);
    lRetVal = NWP_SwitchOn();
	if (lRetVal < 0)
	{
	   DEBG_PRINT("Failed to start NWP\n");
	   LOOP_FOREVER();
	}

    // Set SSID name for AP mode
    Get_FridgeCamID(&ucFridgeCamID[0]);
	length = strlen((const char *)&ucFridgeCamID[0]);
	lRetVal = sl_WlanSet(SL_WLAN_CFG_AP_ID, WLAN_AP_OPT_SSID, length, &ucFridgeCamID[0]);

	//
	// Configure to AP Mode
	//
	if(ConfigureMode(ROLE_AP) !=ROLE_AP)
	{
		DEBG_PRINT("Unable to set AP mode...\n");
		lRetVal = sl_Stop(SL_STOP_TIMEOUT);
		//CLR_STATUS_BIT_ALL(g_ulSimplelinkStatus);
		g_ulSimplelinkStatus = 0;
		DEBG_PRINT("SL3: %x\n", g_ulSimplelinkStatus);
		LOOP_FOREVER();
	}
	while(!IS_IP_ACQUIRED(g_ulSimplelinkStatus));	//looping till ip is acquired
	DEBG_PRINT("CC3200 in APmode\n");

	//Stop Internal HTTP Server
	lRetVal = sl_NetAppStop(SL_NET_APP_HTTP_SERVER_ID);
	if(lRetVal < 0)
	{
		ERR_PRINT(lRetVal);
		LOOP_FOREVER();
	}
	//Start Internal HTTP Server
	lRetVal = sl_NetAppStart(SL_NET_APP_HTTP_SERVER_ID);
	if(lRetVal < 0)
	{
		ERR_PRINT(lRetVal);
		LOOP_FOREVER();
	}

	DEBG_PRINT("HTTPServer Started\n");

	return lRetVal;
}

//****************************************************************************
//
// Task function implements the WiFi provisioning functionality
//
// \param none
//
// This function
//		1. Switch to STA Mode and tries to Connect to AP in global
//		2. If connection is successful, it stores the credential
//		3. Switches back to AP mode and starts the HTTP server and returns
//
// \return None.
//
//	\note	Ensure this function is called, when g_cWlanSSID and g_SecParams
//			hold the WiFi AP credentials.
//****************************************************************************
static int32_t WiFiProvisioning()
{
	int32_t lRetVal;
	uint8_t* pucConfigFileData = (uint8_t*)g_pfUserConfigData;
	pucConfigFileData += WIFI_DATA_OFFSET;

	g_ucProfileAdded = 0;

	//
	//	Test connection
	//
	// Configure to STA Mode
	if(ConfigureMode(ROLE_STA) !=ROLE_STA)
	{
		DEBG_PRINT("Unable to set STA mode\n");
		lRetVal = sl_Stop(SL_STOP_TIMEOUT);
		//CLR_STATUS_BIT_ALL(g_ulSimplelinkStatus);
		g_ulSimplelinkStatus = 0;
		DEBG_PRINT("SL3: %x\n", g_ulSimplelinkStatus);
		LOOP_FOREVER();
	}
	// Connect to the Configured Access Point
	lRetVal = WlanConnect();
	if(lRetVal == SUCCESS)
	{
		g_ucProvisioningDone = 1;	//Set the global variable
		g_ucConnectedToConfAP = IS_CONNECTED(g_ulSimplelinkStatus);
		RELEASE_PRINT("User WiFi connection success\n");

		//Disconnect back
		lRetVal = sl_WlanDisconnect();
		if(0 == lRetVal)
		{
			while(IS_CONNECTED(g_ulSimplelinkStatus));
		}
		//DEBG_PRINT("WLAN Disconnected back\n\r");

/*		//Write to Flash file
		lRetVal = ReadFile_FromFlash(ucConfigFileData,
										(uint8_t*)USER_CONFIGS_FILENAME,
										CONTENT_LENGTH_USER_CONFIGS,
										0);*/

		strcpy((char*)pucConfigFileData, (const char*)g_cWlanSSID);
		pucConfigFileData += strlen((const char*)pucConfigFileData)+1;

		strcpy((char*)pucConfigFileData, (const char*)g_cWlanSecurityType);
		pucConfigFileData += strlen((const char*)pucConfigFileData)+1;

		strcpy((char*)pucConfigFileData, (const char*)g_cWlanSecurityKey);

/*		lRetVal = WriteFile_ToFlash((uint8_t*)&ucConfigFileData[0],
									(uint8_t*)USER_CONFIGS_FILENAME,
									CONTENT_LENGTH_USER_CONFIGS,
									0, 1, &lFileHandle);
		ASSERT_ON_ERROR(lRetVal);

		// For verification - DBG
		lRetVal = ReadFile_FromFlash((ucConfigFileData+3),
										(uint8_t*)USER_CONFIGS_FILENAME,
										WIFI_DATA_SIZE-3,
										WIFI_DATA_OFFSET);
		ASSERT_ON_ERROR(lRetVal);*/
	}
	else
	{
		RELEASE_PRINT("User WiFi connect test failed\nSSID:%s, Key:%s, "
						"SecurityType:%s\n***TRY AGAIN*** \n", g_cWlanSSID,
						g_cWlanSecurityType, g_cWlanSecurityKey);
		//Switch off LED for 5 sec to indicate the WiFi config didnt go well
		LED_Off();
		osi_Sleep(5000);	//5 seconds
	}

	//
	// Configure to AP Mode to display message
	//
	if(ConfigureMode(ROLE_AP) !=ROLE_AP)
	{
		DEBG_PRINT("Unable to set AP mode...\n");
		lRetVal = sl_Stop(SL_STOP_TIMEOUT);
		//CLR_STATUS_BIT_ALL(g_ulSimplelinkStatus);
		g_ulSimplelinkStatus = 0;
		DEBG_PRINT("SL3: %x\n", g_ulSimplelinkStatus);
		LOOP_FOREVER();
	}
	while(!IS_IP_ACQUIRED(g_ulSimplelinkStatus));	//looping till ip is acquired
	DEBG_PRINT("User WiFi Testing over\nConfigd APmode\n");

	//Stop Internal HTTP Server
	lRetVal = sl_NetAppStop(SL_NET_APP_HTTP_SERVER_ID);
	if(lRetVal < 0)
	{
		ERR_PRINT(lRetVal);
		LOOP_FOREVER();
	}
	//Start Internal HTTP Server
	lRetVal = sl_NetAppStart(SL_NET_APP_HTTP_SERVER_ID);
	if(lRetVal < 0)
	{
		ERR_PRINT(lRetVal);
		LOOP_FOREVER();
	}
	DEBG_PRINT("HTTPServer Started again\n");
	MAP_UtilsDelay(80000000);	//Wait for some time to let http msgs txit

	return 0;
}

//******************************************************************************
//
// \brief Connecting to a WLAN Accesspoint
//
//  This function connects to the required AP with Security
//  parameters in the global parameters g_cWlanSSID and g_SecParams.
//
// \param  None
//
// \return  0 - Success, or negative error code
//
// \NOTE: use this only while verifying connection while getting User WiFi
//	credentials through Mobile App
//
//******************************************************************************
static long WlanConnect()
{
	long lRetVal = -1;
	unsigned int uiConnectTimeoutCnt =0;

    //Connecting to the Access point
	lRetVal = sl_WlanConnect(g_cWlanSSID, strlen((char*)g_cWlanSSID), 0,
								&g_SecParams, 0);
	ASSERT_ON_ERROR(lRetVal);

    // Wait for WLAN Event
    while(uiConnectTimeoutCnt<CONNECTION_TIMEOUT_COUNT &&
		((!IS_CONNECTED(g_ulSimplelinkStatus)) || (!IS_IP_ACQUIRED(g_ulSimplelinkStatus))))
    {
        osi_Sleep(10); //Sleep 1 millisecond
        uiConnectTimeoutCnt++;
    }

    if (!(IS_CONNECTED(g_ulSimplelinkStatus) || IS_IP_ACQUIRED(g_ulSimplelinkStatus)))
    {
    	return USER_WIFI_PROFILE_FAILED_TO_CONNECT;
    }
    else
    {
    	return SUCCESS;
    }
}

//******************************************************************************
//	This function does magnetometer calibration and saves the calibration values
//		in flash file
//******************************************************************************
static int32_t Get_Calibration_MagSensor()
{
	uint8_t tmpCnt=0;
	long lRetVal = -1;
	int32_t lFileHandle;


	//Collect the readings
	fxosDefault_Initializations();

	g_ucMagCalb = 0;

#define MAG_SENSOR_CALIBCOUNT		1
//#define MAG_SENSOR_CALIBCOUNT		3
//#define MAG_SENSOR_CALIBCOUNT		2
	while(g_ucMagCalb < MAG_SENSOR_CALIBCOUNT)
	{
		fxos_Calibration();
	}

	tmpCnt = (OFFSET_MAG_CALB/sizeof(float_t));
	g_pfUserConfigData[tmpCnt++] = thisMagCal.finvW[0][0];
	g_pfUserConfigData[tmpCnt++] = thisMagCal.finvW[0][1];
	g_pfUserConfigData[tmpCnt++] = thisMagCal.finvW[0][2] ;
	g_pfUserConfigData[tmpCnt++] = thisMagCal.finvW[1][0] ;
	g_pfUserConfigData[tmpCnt++] = thisMagCal.finvW[1][1] ;
	g_pfUserConfigData[tmpCnt++] = thisMagCal.finvW[1][2] ;
	g_pfUserConfigData[tmpCnt++] = thisMagCal.finvW[2][0] ;
	g_pfUserConfigData[tmpCnt++] = thisMagCal.finvW[2][1] ;
	g_pfUserConfigData[tmpCnt++] = thisMagCal.finvW[2][2] ;
	g_pfUserConfigData[tmpCnt++] = thisMagCal.fV[0];
	g_pfUserConfigData[tmpCnt++] = thisMagCal.fV[1];
	g_pfUserConfigData[tmpCnt++] = thisMagCal.fV[2];

	g_pfUserConfigData[(OFFSET_FIT_ERROR/sizeof(float_t))] = thisMagCal.fFitErrorpc;

	uint8_t i=0;
	for(i=OFFSET_MAG_CALB; i<12;i++)
		DEBG_PRINT("%3.2f\n",g_pfUserConfigData[i]);
	RELEASE_PRINT("%3.2f%%\n",g_pfUserConfigData[i]);

#define ACCEPTABLE_FITERROR		5
	//Switch off LED for 5 sec to indicate that the fit error was not within acceptable limits
	if(thisMagCal.fFitErrorpc > ACCEPTABLE_FITERROR)
	{
		LED_Off();
		osi_Sleep(5000);	//5 seconds delay
	}

	lRetVal = WriteFile_ToFlash((uint8_t*)g_pfUserConfigData,
							(uint8_t*)USER_CONFIGS_FILENAME,
							CONTENT_LENGTH_USER_CONFIGS, 0,
							SINGLE_WRITE, &lFileHandle);
	ASSERT_ON_ERROR(lRetVal);

	return 0;
}

//******************************************************************************
//	Collects ecompass value and stores the same in flash
//
//	param	ucAngle - can be either ANGLE_40 or ANGLE_90. This parameter
//				determines the offset within the flash file where the compass
//				value will be stored
//******************************************************************************
static int32_t CollectAngle(uint8_t ucAngle)
{
	int32_t lRetVal = -1;
	float_t fAngleTemp;

	//Collect the readings
	angleCheck_Initializations();

	start_periodicInterrupt_timer(2.5);

	magnetometer_initialize();

	//Tag:Work-around for invalid first fet ecompass readings
//	uint8_t tmpCnt=0;
//	for(tmpCnt =0;tmpCnt<75;tmpCnt++)
//	{
//		fAngleTemp = get_angle();
//		//DEBG_PRINT("Measured Angle: %f\n\r",fAngleTemp);
//	}

	fAngleTemp = get_angle();

	stop_periodicInterrupt_timer();
	if(thisSV_6DOF_GB_BASIC.fLPPhi<0)
	{
		fAngleTemp = thisSV_6DOF_GB_BASIC.fLPRho +180;
		if(fAngleTemp>360)
		{
			fAngleTemp = fAngleTemp-360;
		}
	}

	//Save it in flash in the right place
	if(ucAngle == ANGLE_90)
	{
		//DEBG_PRINT("90deg: %f\n\r",fAngleTemp);
		//DEBG_PRINT("90deg: %f, 40deg: %f\n\r",g_pfUserConfigData[0], g_pfUserConfigData[1]);
		*(g_pfUserConfigData+(OFFSET_ANGLE_90/sizeof(float))) = fAngleTemp;
		RELEASE_PRINT("90deg: %3.2f\n",g_pfUserConfigData[(OFFSET_ANGLE_90/sizeof(float))]);
	}
	else if (ucAngle == ANGLE_40)
	{
		//DEBG_PRINT("40deg: %f\n\r",fAngleTemp);
		//DEBG_PRINT("90deg: %f, 40deg: %f\n\r",g_pfUserConfigData[0], g_pfUserConfigData[1]);
		*(g_pfUserConfigData+(OFFSET_ANGLE_40/sizeof(float))) = fAngleTemp;
		//DEBG_PRINT("40deg: %f\n\r",g_pfUserConfigData[1]);
		RELEASE_PRINT("40deg: %3.2f\n",g_pfUserConfigData[(OFFSET_ANGLE_40/sizeof(float))]);
	}

	return lRetVal;
}

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
	ASSERT_ON_ERROR(lRetVal);
	//ReadAllAEnAWBRegs();
	return lRetVal;
}


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

#define ANGLE_90		0
#define ANGLE_40		1

extern int gdoor_90deg_angle;//290
extern int gdoor_close_angle; //110
extern int gdoor_snap_angle;//120
struct MagCalibration thisMagCal;
extern unsigned long g_image_buffer[(IMAGE_BUF_SIZE_BYTES/sizeof(unsigned long))];
volatile uint8_t g_ucMagCalb;

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

float_t* g_pfUserConfigData = (float_t *) g_image_buffer;
#define CONFIG_MODE_USER_ACTION_TIMEOUT		600	//in units of 100ms
												//1 minute = 1*60*10 = 600
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

	//So the user gets USERCONFIG_TIMEOUT amount of time to do the configurations
	g_ulWatchdogCycles = 0;
	g_ulAppTimeout_ms = USERCONFIG_TIMEOUT;

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
			UART_PRINT("Timeout... User did not connect phone\n");
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
			UART_PRINT("User pressed button to exit\n");
			LED_On();	//Indicating to user
			run_flag = false;
			//break;
		}

		if(g_ucCalibration == BUTTON_PRESSED)
		{
			UART_PRINT("*** Calibration\n\r");
			UART_PRINT("***ROTATE DEVICE NOW***\n\r");
			LED_On();
			Get_Calibration_MagSensor();
			standby_accelMagn_fxos8700();
			UART_PRINT("***WAITING FOR BUTTON PRESS***");
			g_ucCalibration = BUTTON_NOT_PRESSED;
			LED_Blink_2(0.5,0.5,BLINK_FOREVER);
			//Elapsed_100MilliSecs = 0;	//Resetting the timer count
    	}
		if(g_ucAngle40 == BUTTON_PRESSED)
		{
			LED_On();
			UART_PRINT("*** Angle40\n\r");
			CollectAngle(ANGLE_40);
			standby_accelMagn_fxos8700();
			UART_PRINT("***WAITING FOR BUTTON PRESS***");
			g_ucAngle40 = BUTTON_NOT_PRESSED;
			LED_Blink_2(0.5,0.5,BLINK_FOREVER);
			//Elapsed_100MilliSecs = 0;	//Resetting the timer count
		}

		if(g_ucAngle90 == BUTTON_PRESSED)
		{
			LED_On();
			UART_PRINT("*** Angle90\n\r");
			CollectAngle(ANGLE_90);
			standby_accelMagn_fxos8700();
			UART_PRINT("***WAITING FOR BUTTON PRESS***");
			g_ucAngle90 = BUTTON_NOT_PRESSED;
			LED_Blink_2(0.5,0.5,BLINK_FOREVER);
			//Elapsed_100MilliSecs = 0;	//Resetting the timer count
		}

		if(g_ucConfig == BUTTON_PRESSED)
		{
			LED_On();
			WiFiProvisioning();
			UART_PRINT("***WAITING FOR BUTTON PRESS***");
			g_ucConfig = BUTTON_NOT_PRESSED;
			LED_Blink_2(0.5,0.5,BLINK_FOREVER);
			//Elapsed_100MilliSecs = 0;	//Resetting the timer count
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
				UART_PRINT("%d:%3.2f\n\r",i,fAngle);
			}*/
			// For verification - DBG
			lRetVal = ReadFile_FromFlash(((uint8_t*)g_pfUserConfigData+3),
											(uint8_t*)USER_CONFIGS_FILENAME,
											CONTENT_LENGTH_USER_CONFIGS,
											0);

			g_ucExitButton = BUTTON_NOT_PRESSED;
			run_flag = false;
			//break;
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
		UART_PRINT("Unable to set to Station mode\n\r");
	}

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
		   UART_PRINT("Failed to configure the device in its default state\n\r");
	   }

	   LOOP_FOREVER();
   }
	//UART_PRINT("Device is configured in default state \n\r");

	//	Start NWP
	//lRetVal = sl_Start(0, 0, 0);
    lRetVal = NWP_SwitchOn();
	if (lRetVal < 0)
	{
	   UART_PRINT("Failed to start the device \n\r");
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
		UART_PRINT("Unable to set AP mode...\n\r");
		lRetVal = sl_Stop(SL_STOP_TIMEOUT);
		//CLR_STATUS_BIT_ALL(g_ulSimplelinkStatus);
		g_ulSimplelinkStatus = 0;
		UART_PRINT("Simplelink Status3: %x\n", g_ulSimplelinkStatus);
		LOOP_FOREVER();
	}
	while(!IS_IP_ACQUIRED(g_ulSimplelinkStatus));	//looping till ip is acquired
	UART_PRINT("CC3200 in APmode\n\r");

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

	UART_PRINT("HTTPServer Started\n\r");

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
		UART_PRINT("Unable to set STA mode...\n\r");
		lRetVal = sl_Stop(SL_STOP_TIMEOUT);
		//CLR_STATUS_BIT_ALL(g_ulSimplelinkStatus);
		g_ulSimplelinkStatus = 0;
		UART_PRINT("Simplelink Status3: %x\n", g_ulSimplelinkStatus);
		LOOP_FOREVER();
	}
	// Connect to the Configured Access Point
	lRetVal = WlanConnect();
	if(lRetVal == SUCCESS)
	{
		g_ucProvisioningDone = 1;	//Set the global variable
		g_ucConnectedToConfAP = IS_CONNECTED(g_ulSimplelinkStatus);

		//Disconnect back
		UART_PRINT("WLAN Connect Done\n\r");
		lRetVal = sl_WlanDisconnect();
		if(0 == lRetVal)
		{
			while(IS_CONNECTED(g_ulSimplelinkStatus));
		}
		//UART_PRINT("WLAN Disconnected back\n\r");

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
		UART_PRINT("WiFi connect test failed\n\r***TRY AGAIN*** \n\r");
		//Switch off LED for 5 sec to indicate the WiFi config didnt go well
		LED_Off();
		osi_Sleep(5000);	//5 seconds
	}

	//
	// Configure to AP Mode to display message
	//
	if(ConfigureMode(ROLE_AP) !=ROLE_AP)
	{
		UART_PRINT("Unable to set AP mode...\n\r");
		lRetVal = sl_Stop(SL_STOP_TIMEOUT);
		//CLR_STATUS_BIT_ALL(g_ulSimplelinkStatus);
		g_ulSimplelinkStatus = 0;
		UART_PRINT("Simplelink Status3: %x\n", g_ulSimplelinkStatus);
		LOOP_FOREVER();
	}
	while(!IS_IP_ACQUIRED(g_ulSimplelinkStatus));	//looping till ip is acquired
	UART_PRINT("WiFi Testing over\n\rConfigd APmode\n\r");

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
	UART_PRINT("HTTPServer Started again\n\r");
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
		for(i=OFFSET_MAG_CALB; i<13;i++)
			UART_PRINT("%f\n",g_pfUserConfigData[i]);

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

	magnetometer_initialize();

	//Tag:Work-around for invalid first fet ecompass readings
//	uint8_t tmpCnt=0;
//	for(tmpCnt =0;tmpCnt<75;tmpCnt++)
//	{
//		fAngleTemp = get_angle();
//		//UART_PRINT("Measured Angle: %f\n\r",fAngleTemp);
//	}

	fAngleTemp = get_angle();

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
		//UART_PRINT("90deg: %f\n\r",fAngleTemp);
		//UART_PRINT("90deg: %f, 40deg: %f\n\r",g_pfUserConfigData[0], g_pfUserConfigData[1]);
		*(g_pfUserConfigData+(OFFSET_ANGLE_90/sizeof(float))) = fAngleTemp;
		UART_PRINT("90deg: %f, 40deg: %f\n\r",g_pfUserConfigData[(OFFSET_ANGLE_90/sizeof(float))], g_pfUserConfigData[(OFFSET_ANGLE_40/sizeof(float))]);
	}
	else if (ucAngle == ANGLE_40)
	{
		//UART_PRINT("40deg: %f\n\r",fAngleTemp);
		//UART_PRINT("90deg: %f, 40deg: %f\n\r",g_pfUserConfigData[0], g_pfUserConfigData[1]);
		*(g_pfUserConfigData+(OFFSET_ANGLE_40/sizeof(float))) = fAngleTemp;
		//UART_PRINT("40deg: %f\n\r",g_pfUserConfigData[1]);
		UART_PRINT("90deg: %f, 40deg: %f\n\r",g_pfUserConfigData[(OFFSET_ANGLE_90/sizeof(float))], g_pfUserConfigData[(OFFSET_ANGLE_40/sizeof(float))]);
	}

	return lRetVal;
}


//#include "wlan.h"
#include "wifi_provisioing_thruAPmode.h"
#include "network_related_fns.h"
#include "flash_files.h"
#include "include_all.h"
#include "simplejson.h"
#include "camera_app.h"
#include "stdbool.h"
#include "ota.h"
static long WlanConnect();

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

int16_t fxos_Calibration();
int16_t fxosDefault_Initializations();
//****************************************************************************
//
//! Task function implements the ProvisioningAP functionality
//!
//! \param none
//!
//! This function
//!    3. Switch to AP Mode and Wait for AP Configuration from Browser
//!    4. Switch to STA Mode and Connect to Configured AP
//!
//! \return None.
//
//****************************************************************************
int32_t ProvisioningAP()
{
   long lRetVal = -1;

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
    UART_PRINT("Device is configured in default state \n\r");

	//	Start NWP
	lRetVal = sl_Start(0, 0, 0);
	if (lRetVal < 0)
	{
	   UART_PRINT("Failed to start the device \n\r");
	   LOOP_FOREVER();
	}

	while(!g_ucProvisioningDone)
    {
    	UART_PRINT("Top of while()\n\r");

        //
        // Configure to AP Mode
        //
        if(ConfigureMode(ROLE_AP) !=ROLE_AP)
        {
            UART_PRINT("Unable to set AP mode...\n\r");
            lRetVal = sl_Stop(SL_STOP_TIMEOUT);
            CLR_STATUS_BIT_ALL(g_ulStatus);
            LOOP_FOREVER();
        }
        while(!IS_IP_ACQUIRED(g_ulStatus));	//looping till ip is acquired
        UART_PRINT("Configd APmode\n\r");

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
        UART_PRINT("HTTPSrvr Strtd\n\r");

        //
		// Wait for AP Configuraiton, Open Browser and Configure AP
		//
		while(!g_ucProfileAdded)
		{
			MAP_UtilsDelay(100);
			//UART_PRINT("w");
		}
		UART_PRINT("\n\rwhile exit\n\r");

		g_ucProfileAdded = 0;

		//
		//	Test connection
		//
        // Configure to STA Mode
        if(ConfigureMode(ROLE_STA) !=ROLE_STA)
        {
            UART_PRINT("Unable to set STA mode...\n\r");
            lRetVal = sl_Stop(SL_STOP_TIMEOUT);
            CLR_STATUS_BIT_ALL(g_ulStatus);
            LOOP_FOREVER();
        }
        // Connect to the Configured Access Point
        lRetVal = WlanConnect();
		if(lRetVal == SUCCESS)
		{
			g_ucProvisioningDone = 1;	//Set the global variable
			g_ucConnectedToConfAP = IS_CONNECTED(g_ulStatus);

			//Disconnect back
			UART_PRINT("WLAN Connect Done\n\r");
			lRetVal = sl_WlanDisconnect();
			if(0 == lRetVal)
			{
				while(IS_CONNECTED(g_ulStatus));
			}
			UART_PRINT("WLAN Disconnected back\n\r");

			//Write to Flash file
			lRetVal = CreateFile_Flash(FILENAME_USERWIFI, MAX_FILESIZE_USERWIFI);
			ASSERT_ON_ERROR(lRetVal);
			uint8_t ucConfigFileData[CONTENTSIZE_FILE_USERWIFI];
			uint8_t* pucConfigFileData = &ucConfigFileData[0];

			strcpy((char*)pucConfigFileData, (const char*)g_cWlanSSID);
			pucConfigFileData += strlen((const char*)pucConfigFileData)+1;

			strcpy((char*)pucConfigFileData, (const char*)g_cWlanSecurityType);
			pucConfigFileData += strlen((const char*)pucConfigFileData)+1;

			strcpy((char*)pucConfigFileData, (const char*)g_cWlanSecurityKey);

			int32_t lFileHandle;
			lRetVal = WriteFile_ToFlash((uint8_t*)&ucConfigFileData[0],
										(uint8_t*)FILENAME_USERWIFI,
										CONTENTSIZE_FILE_USERWIFI,
										0, 1, &lFileHandle);
			ASSERT_ON_ERROR(lRetVal);

			// For verification - DBG
			lRetVal = ReadFile_FromFlash((ucConfigFileData+3),
											(uint8_t*)FILENAME_USERWIFI,
											CONTENTSIZE_FILE_USERWIFI-3,
											0);
			ASSERT_ON_ERROR(lRetVal);

			/*lRetVal = WriteFile_ToFlash((uint8_t*)&g_cWlanSSID[0], (uint8_t*)FILENAME_USERWIFI,
										AP_SSID_LEN_MAX, 0);
			lRetVal = WriteFile_ToFlash((uint8_t*)(&(g_SecParams.Type)), (uint8_t*)FILENAME_USERWIFI,
										AP_SECTYPE_LEN_MAX, AP_SSID_LEN_MAX);
			lRetVal = WriteFile_ToFlash((uint8_t*)g_SecParams.Key, (uint8_t*)FILENAME_USERWIFI,
										AP_PASSWORD_LEN_MAX,
										AP_SSID_LEN_MAX + AP_SECTYPE_LEN_MAX);*/

			/*uint8_t ucConfigFileData[FILE_SIZE_USER_CONFIG];
			uint8_t ucSSID[33];
			uint8_t uc[33];
			ReadFile_FromFlash(ucConfigFileData, FILE_NAME_USER_CONFIG, FILE_SIZE_USER_CONFIG, 0);
			simpleJsonProcessor(ucConfigFileData, "SSID", ucSSID, SSID_LEN_MAX+1);*/
		}

		//
		// Configure to AP Mode to display message
		//
		if(ConfigureMode(ROLE_AP) !=ROLE_AP)
		{
			UART_PRINT("Unable to set AP mode...\n\r");
			lRetVal = sl_Stop(SL_STOP_TIMEOUT);
			CLR_STATUS_BIT_ALL(g_ulStatus);
			LOOP_FOREVER();
		}
		while(!IS_IP_ACQUIRED(g_ulStatus));	//looping till ip is acquired
		UART_PRINT("Configd APmode\n\r");

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
		UART_PRINT("HTTPSrvr Strtd again\n\r");
		MAP_UtilsDelay(80000000);	//Wait for some time to let http msgs txit
    }

	//Stop Internal HTTP Server
	lRetVal = sl_NetAppStop(SL_NET_APP_HTTP_SERVER_ID);
	if(lRetVal < 0)
	{
		ERR_PRINT(lRetVal);
		LOOP_FOREVER();
	}

	lRetVal = sl_Stop(SL_STOP_TIMEOUT);
	CLR_STATUS_BIT_ALL(g_ulStatus);

    return SUCCESS;
}

int32_t WiFiProvisioning()
{
	int32_t lRetVal;

	g_ucProfileAdded = 0;

	//
	//	Test connection
	//
	// Configure to STA Mode
	if(ConfigureMode(ROLE_STA) !=ROLE_STA)
	{
		UART_PRINT("Unable to set STA mode...\n\r");
		lRetVal = sl_Stop(SL_STOP_TIMEOUT);
		CLR_STATUS_BIT_ALL(g_ulStatus);
		LOOP_FOREVER();
	}
	// Connect to the Configured Access Point
	lRetVal = WlanConnect();
	if(lRetVal == SUCCESS)
	{
		g_ucProvisioningDone = 1;	//Set the global variable
		g_ucConnectedToConfAP = IS_CONNECTED(g_ulStatus);

		//Disconnect back
		UART_PRINT("WLAN Connect Done\n\r");
		lRetVal = sl_WlanDisconnect();
		if(0 == lRetVal)
		{
			while(IS_CONNECTED(g_ulStatus));
		}
		//UART_PRINT("WLAN Disconnected back\n\r");

		//Write to Flash file
		lRetVal = CreateFile_Flash(FILENAME_USERWIFI, MAX_FILESIZE_USERWIFI);
		ASSERT_ON_ERROR(lRetVal);
		uint8_t ucConfigFileData[CONTENTSIZE_FILE_USERWIFI];
		uint8_t* pucConfigFileData = &ucConfigFileData[0];

		strcpy((char*)pucConfigFileData, (const char*)g_cWlanSSID);
		pucConfigFileData += strlen((const char*)pucConfigFileData)+1;

		strcpy((char*)pucConfigFileData, (const char*)g_cWlanSecurityType);
		pucConfigFileData += strlen((const char*)pucConfigFileData)+1;

		strcpy((char*)pucConfigFileData, (const char*)g_cWlanSecurityKey);

		int32_t lFileHandle;
		lRetVal = WriteFile_ToFlash((uint8_t*)&ucConfigFileData[0],
									(uint8_t*)FILENAME_USERWIFI,
									CONTENTSIZE_FILE_USERWIFI,
									0, 1, &lFileHandle);
		ASSERT_ON_ERROR(lRetVal);

		// For verification - DBG
		lRetVal = ReadFile_FromFlash((ucConfigFileData+3),
										(uint8_t*)FILENAME_USERWIFI,
										CONTENTSIZE_FILE_USERWIFI-3,
										0);
		ASSERT_ON_ERROR(lRetVal);
	}
	else
	{
		UART_PRINT("WiFi connect test failed\n\r***TRY AGAIN*** \n\r");
	}

	//
	// Configure to AP Mode to display message
	//
	if(ConfigureMode(ROLE_AP) !=ROLE_AP)
	{
		UART_PRINT("Unable to set AP mode...\n\r");
		lRetVal = sl_Stop(SL_STOP_TIMEOUT);
		CLR_STATUS_BIT_ALL(g_ulStatus);
		LOOP_FOREVER();
	}
	while(!IS_IP_ACQUIRED(g_ulStatus));	//looping till ip is acquired
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

#define ANGLE_90		0
#define ANGLE_40		1

int32_t CollectAngle(uint8_t ucAngle)
{
	int32_t lRetVal = -1;
	float_t fAngleTemp;
	int32_t lFileHandle;
	float_t *Mag_Calb_Value = (float_t *) g_image_buffer;


	//Collect the readings
	angleCheck_Initializations();

	uint8_t tmpCnt=0;
	for(tmpCnt =0;tmpCnt<75;tmpCnt++)
	{
		fAngleTemp = get_angle();
		//UART_PRINT("Measured Angle: %f\n\r",fAngleTemp);
	}

	ReadFile_FromFlash((uint8_t*)g_image_buffer, (uint8_t*)FILENAME_ANGLE_VALS, MAX_FILESIZE_ANGLE_VALS, 0);

//	float fAnglet[2];
//	ReadFile_FromFlash((uint8_t*)fAnglet, (uint8_t*)FILENAME_ANGLE_VALS, (sizeof(float)*2), 0);

	//Save it in flash in the right place
	//create_AngleValuesFile();
	if(ucAngle == ANGLE_90)
	{
		UART_PRINT("90deg: %f\n\r",fAngleTemp);
		Mag_Calb_Value[0] = fAngleTemp;
		lRetVal = WriteFile_ToFlash((uint8_t*)g_image_buffer,
								(uint8_t*)FILENAME_ANGLE_VALS,
								MAX_FILESIZE_ANGLE_VALS, 0,
								SINGLE_WRITE, &lFileHandle);
		ASSERT_ON_ERROR(lRetVal);
	}
	else if (ucAngle == ANGLE_40)
	{
		UART_PRINT("40deg: %f\n\r",fAngleTemp);
		Mag_Calb_Value[1] = fAngleTemp;
		lRetVal = WriteFile_ToFlash((uint8_t*)g_image_buffer,
								(uint8_t*)FILENAME_ANGLE_VALS,
								MAX_FILESIZE_ANGLE_VALS, 0,
								SINGLE_WRITE, &lFileHandle);
		ASSERT_ON_ERROR(lRetVal);
	}

	return lRetVal;
}

int32_t CalibrateMagSensor()
{
	int32_t lRetVal = -1;
	int32_t lFileHandle;
	uint8_t tmpCnt=0;
	float_t *Mag_Calb_Value = (float_t *) g_image_buffer;

	//Collect the readings
	fxosDefault_Initializations();

	ReadFile_FromFlash((uint8_t*)Mag_Calb_Value, (uint8_t*)FILENAME_ANGLE_VALS, MAX_FILESIZE_ANGLE_VALS, 0);

	g_ucMagCalb = 0;
	while(g_ucMagCalb < MAG_SENSOR_CALIBCOUNT)
	{
		fxos_Calibration();
	}

	tmpCnt = OFFSET_MAG_CALB;
	Mag_Calb_Value[tmpCnt++] = thisMagCal.finvW[0][0];
	Mag_Calb_Value[tmpCnt++] = thisMagCal.finvW[0][1];
	Mag_Calb_Value[tmpCnt++] = thisMagCal.finvW[0][2] ;
	Mag_Calb_Value[tmpCnt++] = thisMagCal.finvW[1][0] ;
	Mag_Calb_Value[tmpCnt++] = thisMagCal.finvW[1][1] ;
	Mag_Calb_Value[tmpCnt++] = thisMagCal.finvW[1][2] ;
	Mag_Calb_Value[tmpCnt++] = thisMagCal.finvW[2][0] ;
	Mag_Calb_Value[tmpCnt++] = thisMagCal.finvW[2][1] ;
	Mag_Calb_Value[tmpCnt++] = thisMagCal.finvW[2][2] ;
	Mag_Calb_Value[tmpCnt++] = thisMagCal.fV[0];
	Mag_Calb_Value[tmpCnt++] = thisMagCal.fV[1];
	Mag_Calb_Value[tmpCnt++] = thisMagCal.fV[2];

//	uint8_t i=0;
//	for(i=2; i<14;i++)
//		UART_PRINT("%f\n",Mag_Calb_Value[i]);

	lRetVal = WriteFile_ToFlash((uint8_t*)Mag_Calb_Value,
							(uint8_t*)FILENAME_ANGLE_VALS,
							MAX_FILESIZE_ANGLE_VALS, 0,
							SINGLE_WRITE, &lFileHandle);
	ASSERT_ON_ERROR(lRetVal);

	return 0;

}

int32_t AccessPtMode_HTTPServer_Start()
{
	int32_t lRetVal = -1;
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
	lRetVal = sl_Start(0, 0, 0);
	if (lRetVal < 0)
	{
	   UART_PRINT("Failed to start the device \n\r");
	   LOOP_FOREVER();
	}

	//
	// Configure to AP Mode
	//
	if(ConfigureMode(ROLE_AP) !=ROLE_AP)
	{
		UART_PRINT("Unable to set AP mode...\n\r");
		lRetVal = sl_Stop(SL_STOP_TIMEOUT);
		CLR_STATUS_BIT_ALL(g_ulStatus);
		LOOP_FOREVER();
	}
	while(!IS_IP_ACQUIRED(g_ulStatus));	//looping till ip is acquired
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

int32_t User_Configure()
{
	long lRetVal = -1;
	bool run_flag=true;

//	CreateFile_Flash((uint8_t*)FILENAME_ANGLE_VALS, MAX_FILESIZE_ANGLE_VALS);

	AccessPtMode_HTTPServer_Start();

//
//	ReadFile_FromFlash((uint8_t*)&fAngle, (uint8_t*)FILENAME_ANGLE_VALS, sizeof(float), 0);
//
//	float fAnglet[2];
//	ReadFile_FromFlash((uint8_t*)fAnglet, (uint8_t*)FILENAME_ANGLE_VALS, (sizeof(float)*2), 0);


	while(run_flag == true)
    {

		if(g_ucCalibration == BUTTON_PRESSED)
		{
			UART_PRINT("*** Calibration\n\r");
			UART_PRINT("***ROTATE DEVICE NOW***\n\r");
			CalibrateMagSensor();
			UART_PRINT("***WAITING FOR BUTTON PRESS***");
			g_ucCalibration = BUTTON_NOT_PRESSED;
    	}
		if(g_ucAngle40 == BUTTON_PRESSED)
		{
			UART_PRINT("*** Angle40\n\r");
			CollectAngle(ANGLE_40);
			UART_PRINT("***WAITING FOR BUTTON PRESS***");
			g_ucAngle40 = BUTTON_NOT_PRESSED;
		}

		if(g_ucAngle90 == BUTTON_PRESSED)
		{
			UART_PRINT("*** Angle90\n\r");
			CollectAngle(ANGLE_90);
			UART_PRINT("***WAITING FOR BUTTON PRESS***");
			g_ucAngle90 = BUTTON_NOT_PRESSED;
		}

		if(g_ucConfig == BUTTON_PRESSED)
		{
			WiFiProvisioning();
			UART_PRINT("***WAITING FOR BUTTON PRESS***");
			g_ucConfig = BUTTON_NOT_PRESSED;
    	}

		if(g_ucExitButton == BUTTON_PRESSED)
		{
//			fAngle = 0;
//			ReadFile_FromFlash((uint8_t*)&fAngle, (uint8_t*)FILENAME_ANGLE_VALS, sizeof(float), 0);
//			UART_PRINT("Angle90 = %3.2f\n\r",fAngle);
//
//			fAngle = 0;
//			ReadFile_FromFlash((uint8_t*)&fAngle, (uint8_t*)FILENAME_ANGLE_VALS, sizeof(float), sizeof(float));
//			UART_PRINT("\n\nAngle40 = %3.2f\n\r",fAngle);
			run_flag = false;
			break;
		}
		if(!GPIOPinRead(GPIOA1_BASE, GPIO_PIN_0))	//Take this to GPIO Interrupt
		{
			UART_PRINT("Button Pressed for second time\n\r");
			g_ucPushButtonPressedTwice = BUTTON_PRESSED;
		}
		if(g_ucPushButtonPressedTwice == BUTTON_PRESSED)
		{
			//sl_Stop(0xFF);
			UART_PRINT("Entering OTA update\n\r");
			OTA_Update();
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

	lRetVal = sl_Stop(SL_STOP_TIMEOUT);
	CLR_STATUS_BIT_ALL(g_ulStatus);

    return SUCCESS;
}

int32_t create_AngleValuesFile()
{
	long lFileHandle;
	unsigned long ulToken = NULL;
	long lRetVal;

	lRetVal = sl_Start(0,0,0);
	ASSERT_ON_ERROR(lRetVal);

	//
	// NVMEM File Open to write to SFLASH
	//
	lRetVal = sl_FsOpen((unsigned char *)FILENAME_ANGLE_VALS,//0x00212001,
						FS_MODE_OPEN_CREATE(MAX_FILESIZE_ANGLE_VALS,_FS_FILE_OPEN_FLAG_COMMIT|_FS_FILE_PUBLIC_WRITE),
						&ulToken,
						&lFileHandle);
	if(lRetVal < 0)
	{
		UART_PRINT("File Open Error: %i", lRetVal);
		lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
		ASSERT_ON_ERROR(CAMERA_CAPTURE_FAILED);
	}

	// Close the created file
	lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
	ASSERT_ON_ERROR(lRetVal);

	lRetVal = sl_Stop(0xFFFF);
	ASSERT_ON_ERROR(lRetVal);

	return lRetVal;
}
//******************************************************************************
//
//! \brief Connecting to a WLAN Accesspoint
//!
//!  This function connects to the required AP (SSID_NAME) with Security
//!  parameters specified in te form of macros at the top of this file
//!
//! \param  None
//!
//! \return  0 - Success
//!            -1 - Failure

//!
//! \warning    If the WLAN connection fails or we don't aquire an IP
//!            address, It will be stuck in this function forever.
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
		((!IS_CONNECTED(g_ulStatus)) || (!IS_IP_ACQUIRED(g_ulStatus))))
    {
        osi_Sleep(1); //Sleep 1 millisecond
        uiConnectTimeoutCnt++;
    }

    if (!(IS_CONNECTED(g_ulStatus) || IS_IP_ACQUIRED(g_ulStatus)))
    {
    	return USER_WIFI_PROFILE_FAILED_TO_CONNECT;
    }
    else
    {
    	return SUCCESS;
    }
}

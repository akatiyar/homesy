/*
 * wifi_provisioning.c
 *
 *  Created on: 18-Sep-2015
 *      Author: Chrysolin
 */


#include "app_common.h"
#include "network_related_fns.h"
#include "flash_files.h"

static long WlanConnect();
extern float_t* g_pfUserConfigData;

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
int32_t WiFiProvisioning()
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
		RETURN_ON_ERROR(lRetVal);

		// For verification - DBG
		lRetVal = ReadFile_FromFlash((ucConfigFileData+3),
										(uint8_t*)USER_CONFIGS_FILENAME,
										WIFI_DATA_SIZE-3,
										WIFI_DATA_OFFSET);
		RETURN_ON_ERROR(lRetVal);*/
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
	RETURN_ON_ERROR(lRetVal);

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
